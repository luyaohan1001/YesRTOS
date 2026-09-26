"""YesRTOS stack usage for gdb.

Needs a gdb built with Python, e.g. Homebrew's /opt/homebrew/bin/gdb. The gdb shipped with the
Arm GNU toolchain in compiler/ is built without Python and cannot load this file.

    (gdb) source tools/gdb/stack-usage.py
    (gdb) yesrtos stacks                        # every thread the kernel knows about
    (gdb) yesrtos stacks --fill 0xDEADBEEF      # stacks pre-filled with a pattern
    (gdb) yesrtos stacks some_obj.p_blocked_list
                                                # also walk extra blocked lists (non-global mutexes)

Threads are collected from the scheduler ready lists, the blocked list of every global
YesRTOS::Mutex, any list heads given as arguments, and the active thread.

"now"  = bytes between the top of the thread stack and its stack pointer
         ($psp for the active thread, the saved stkptr otherwise).
"peak" = high-water mark: bytes above the first word (scanning up from the stack limit) that
         differs from the fill value. Only meaningful if the stacks were pre-filled with that
         value; QEMU starts with zeroed RAM, hence the default fill of 0. On hardware RAM
         starts random, so peak is only an upper bound there unless stacks are painted.
"""

import re
import struct

import gdb

SCHED = "YesRTOS::PreemptFIFOScheduler"


class _CxxLanguage:
    """Evaluate C++ names even while stopped in a C frame (e.g. trace_qemu_usart.c)."""

    def __enter__(self):
        self.saved = gdb.parameter("language")
        gdb.execute("set language c++", to_string=True)

    def __exit__(self, *exc):
        gdb.execute("set language " + self.saved, to_string=True)


def _list_threads(head):
    """Yield every Thread* of a linked list starting at head (a Thread* value)."""
    seen = set()
    node = head
    while int(node) != 0 and int(node) not in seen:
        seen.add(int(node))
        yield node
        node = node["thread_info"]["p_next"]


def _global_mutex_names():
    """Names of global variables whose type is YesRTOS::Mutex."""
    out = gdb.execute(r"info variables -q -t ^YesRTOS::Mutex$", to_string=True)
    return re.findall(r"YesRTOS::Mutex\s+([\w:]+);", out)


def _routine_name(thread):
    ptr = int(thread["thread_info"]["routine_ptr"])
    block = gdb.block_for_pc(ptr) if ptr else None
    if block is not None and block.function is not None:
        return block.function.print_name
    return "0x%08x" % ptr


def _collect_threads(extra_lists):
    """Return {address: (Thread* value, where it was found)} for all known threads."""
    threads = {}

    def add(thread, where):
        threads.setdefault(int(thread), (thread, where))

    heads = gdb.parse_and_eval(SCHED + "::ready_list_heads")
    lo, hi = heads.type.range()
    for prio in range(lo, hi + 1):
        for t in _list_threads(heads[prio]):
            add(t, "ready prio %d" % prio)

    for name in _global_mutex_names():
        for t in _list_threads(gdb.parse_and_eval(name)["p_blocked_list"]):
            add(t, "blocked on " + name)

    for expr in extra_lists:
        for t in _list_threads(gdb.parse_and_eval(expr)):
            add(t, "blocked in " + expr)

    active = gdb.parse_and_eval(SCHED + "::p_active_thread")
    if int(active) != 0:
        add(active, "active")
    return threads, int(active)


def _stack_report(thread, is_active, fill):
    stack = thread["allocated_stack"]
    words = stack.type.sizeof // 4
    lo = int(stack.address)
    hi = lo + stack.type.sizeof

    sp = int(gdb.parse_and_eval("$psp")) if is_active else int(thread["stkptr"])

    # One memory read for the whole stack, then scan up from the limit for the first used word.
    raw = bytes(gdb.selected_inferior().read_memory(lo, stack.type.sizeof))
    unused = 0
    for (word,) in struct.iter_unpack("<I", raw):
        if word != fill:
            break
        unused += 1

    return {
        "size": words * 4,
        "now": hi - sp,
        "peak": (words - unused) * 4,
        "overflow": not (lo <= sp <= hi),
        "sp": sp,
        "lo": lo,
        "hi": hi,
    }


class YesRTOSPrefix(gdb.Command):
    """YesRTOS kernel helpers."""

    def __init__(self):
        super().__init__("yesrtos", gdb.COMMAND_USER, prefix=True)


class YesRTOSStacks(gdb.Command):
    """Print current and peak stack usage of every YesRTOS thread.

Usage: yesrtos stacks [--fill VALUE] [BLOCKED_LIST_HEAD ...]

--fill VALUE        Value unused stack words hold (default 0, QEMU zeroed RAM).
BLOCKED_LIST_HEAD   Extra Thread* list heads to walk, e.g. a non-global mutex's
                    p_blocked_list. Global YesRTOS::Mutex objects are found automatically."""

    def __init__(self):
        super().__init__("yesrtos stacks", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        fill = 0
        if "--fill" in argv:
            i = argv.index("--fill")
            fill = int(argv[i + 1], 0)
            del argv[i:i + 2]

        with _CxxLanguage():
            threads, active = _collect_threads(argv)
            rows = []
            for addr, (thread, where) in threads.items():
                r = _stack_report(thread, addr == active, fill)
                rows.append((thread, where, r))

            fmt = "%-3s %-10s %-20s %8s %13s %8s  %s"
            print(fmt % ("id", "thread", "routine", "now", "peak", "size", "state"))
            for thread, where, r in sorted(rows, key=lambda row: -row[2]["peak"]):
                state = where + (", active" if int(thread) == active else "")
                if r["overflow"]:
                    state += "  !! sp 0x%08x outside [0x%08x, 0x%08x]: OVERFLOW" % (r["sp"], r["lo"], r["hi"])
                print(fmt % (
                    int(thread["thread_info"]["id"]), "0x%08x" % int(thread), _routine_name(thread),
                    "%d B" % r["now"], "%d B (%d%%)" % (r["peak"], r["peak"] * 100 // r["size"]),
                    "%d B" % r["size"], state))

            estack = int(gdb.parse_and_eval("(unsigned int)&_estack"))
            msp = int(gdb.parse_and_eval("$msp"))
            print("MSP (main / exceptions): %d B below _estack" % (estack - msp))


YesRTOSPrefix()
YesRTOSStacks()
