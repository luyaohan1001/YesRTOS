---
name: yesrtos-dev
description: Development conventions for YesRTOS. Use whenever creating a git commit (or PR) in this repository with Claude's help, and when fixing a kernel bug - formats the subject as "[<TYPE>][<SIZE>][AI] <description>" (TYPE such as FEATURE or BUG FIX; bug fixes add CATEGORY/HAZARD/EXAMPLE/FIX sections), derives the size from the staged diff, stages only the change at hand, verifies the staged state builds, and records RTOS design bugs as case studies in docs/case_studies/ indexed from a folded README section.
---

# Committing to YesRTOS with Claude

Every commit Claude creates or substantially writes in this repository follows this convention.
Commits written by hand keep the existing `YYYYMMDD <description>` style; never rewrite those.

## Subject line

```
[<TYPE>][<SIZE>][AI] <description>
```

- `<TYPE>` is the kind of change:

  | Type       | Use for                                                        |
  |------------|----------------------------------------------------------------|
  | `FEATURE`  | New capability (API, target, tool, preset)                     |
  | `BUG FIX`  | Fixes incorrect behaviour; body needs the sections below       |
  | `REFACTOR` | Restructures code or build without changing behaviour          |
  | `CLEANUP`  | Removes dead code, unused targets or files                     |
  | `DOCS`     | Documentation or comments only                                 |

  When a change is both, pick the one a reader most needs to know, e.g. `BUG FIX` over
  `FEATURE` when a new API mainly exists to fix a defect.
- `<SIZE>` is one of `XS`, `S`, `M`, `L`, `XL` (see below).
- `[AI]` marks the commit as made with Claude.
- `<description>`: English, imperative mood ("Remove ...", "Fix ..."), no trailing period,
  whole subject line within 72 characters.

Examples:

```
[DOCS][XS][AI] Fix typo in PendSV_Handler comment
[FEATURE][M][AI] Add QEMU netduinoplus2 target
[BUG FIX][M][AI] Hand mutex ownership to the woken thread on unlock
[CLEANUP][L][AI] Remove RISC-V (RV32I) support
```

## Size

Count the changed lines of the staged diff before writing the subject, never estimate:

```
git diff --cached --shortstat    # insertions + deletions
```

| Size | Changed lines |
|------|---------------|
| XS   | 1 - 10        |
| S    | 11 - 50       |
| M    | 51 - 200      |
| L    | 201 - 500     |
| XL   | > 500         |

Leave vendored or generated files (`compiler/`, `*.svd`, CubeMX output) out of the count and
mention them in the body instead.

## Body

- English only, wrapped at 72 columns: what changed and why, plus anything a reviewer
  should know (behaviour changes, things left out on purpose).
- End with the `Co-Authored-By` trailer that Claude Code requires.

## Bug fixes

A commit typed `[BUG FIX]`:

```
[BUG FIX][<SIZE>][AI] <description>
```

has a body containing these four sections, in this order:

- `[CATEGORY]` A named class of bug, e.g. race condition, data integrity, memory ordering,
  atomicity, deadlock, stack overflow, undefined behaviour, interrupt priority, build.
- `[HAZARD]` What goes wrong, under which conditions, and what the user observes.
- `[EXAMPLE]` The essential lines from the code base that reproduce the issue, indented
  by 4 spaces, with the file they come from. Trim to what shows the bug.
- `[FIX]` The fix strategy in a few sentences, followed by the essential lines of the
  fixed code, indented by 4 spaces.

Put anything else (test results, follow-ups) after `[FIX]`. A commit mixing a fix with
unrelated work is split first (see "One change per commit").

Example:

```
[BUG FIX][M][AI] Hand mutex ownership to the woken thread on unlock

[CATEGORY]
Race condition

[HAZARD]
A thread woken from the mutex blocked list returned from lock()
without taking the lock, so a third thread could acquire it as well
and two threads ran inside the critical section. Seen on QEMU as
interleaved trace lines ("thread thread 1").

[EXAMPLE]
kernel/src/mutex.cpp:
    if (atomic_compare_and_swap(&locked, 0, 1) == 0) {
      PreemptFIFOScheduler::block_running_thread(&this->p_blocked_list);
      request_context_switch();
      while(locked == true);   // wakes up, but never sets locked
    }
    this->owner = PreemptFIFOScheduler::p_active_thread;

[FIX]
Run lock() and unlock() with exceptions disabled and let unlock()
hand ownership directly to the longest waiting thread, keeping
`locked` set so nobody can take the mutex in between:
    if (this->p_blocked_list) {
      this->owner = PreemptFIFOScheduler::unblock_one_thread(&this->p_blocked_list);
      request_context_switch();
    } else {
      locked = 0;
      this->owner = nullptr;
    }

QEMU netduinoplus2, 10 kHz tick: 0 interleaved lines in 100k lines.

Co-Authored-By: ...
```

## Case studies

RTOS design bugs are kept as case studies, so the design rule behind each fix is not
forgotten. Write one when a `[BUG FIX]` has category race condition, atomicity, memory
ordering, data integrity, deadlock, stack overflow, interrupt priority, or undefined
behaviour / ABI violations in the kernel or machine layer (stack alignment, naked
handlers, empty ready set, ...). Known but unfixed
issues of those kinds get a case study too, with status **Open**.

### Where

- One file per case: `docs/case_studies/CS-<NNN>-<short-kebab-title>.md`, numbered in
  order of creation (`CS-001`, `CS-002`, ...). Never renumber or reuse an ID.
- The index lives only in the top-level `README.md`, in a `## Case Studies` section right
  after `## Design Phase of YesRTOS`. There is no separate index file.

### README index

A folded `<details>` block holding one table row per case. Keep a blank line after
`<summary>` and before `</details>`, otherwise GitHub does not render the table. Update
the count and the category list in the summary whenever a row is added.

```markdown
## Case Studies

Real bugs found while building YesRTOS, kept as reminders of RTOS design rules.

<details>
<summary><b>7 case studies</b> (race conditions, atomicity, memory ordering) - click to expand</summary>

| ID | Issue | Category | Design rule | Status |
|----|-------|----------|-------------|--------|
| [CS-001](docs/case_studies/CS-001-mutex-wakeup-without-ownership.md) | Woken thread returns from `lock()` without owning the mutex | Race condition | Hand ownership to the woken thread on unlock; never let it race to re-acquire | Fixed `a1b2c3d` |
| CS-007 | FPU context not saved in PendSV | Data integrity | Save S16-S31 when EXC_RETURN bit 4 is 0 | **Open** |

</details>
```

- **Issue**: one line, what goes wrong.
- **Design rule**: one sentence, the lesson; it must match the file's `## Design rule`.
- **Status**: `Fixed <short hash>` (the commit containing the fix), `Documented` (design
  limitation, no fix intended) or `**Open**`.

### Case study file

Same sections as a `[BUG FIX]` commit body, so the commit text can be reused. English,
about one page.

```markdown
# CS-001: Mutex wake-up without ownership

**Category:** Race condition  **Status:** Fixed in `a1b2c3d`  **Test:** `tests/qemu/mutex_interleave`

## Hazard
What goes wrong, under which conditions, what is observed.

## Example
Essential lines that reproduce the issue, with the file they come from.

## Fix
Fix strategy in a few sentences, then the essential lines of the fixed code.

## How it was found
Symptom and tool, e.g. interleaved "thread thread 1" lines on QEMU with a 10 kHz tick.

## Design rule
One or two sentences, the general lesson (same text as the README row).
```

### Workflow

- The case study needs the fix's commit hash, so commit the fix first, then add or update
  the case study and its README row in a following `[DOCS]` commit.
- In the fix commit body, reference the case ID after `[FIX]`, e.g. `Case study: CS-001`.
- Prefer a regression test that reproduces the hazard on QEMU (e.g. under the
  `qemu-stress` preset) and name it in the `**Test:**` field; the document explains the
  rule, the test keeps it from regressing. Write `none` if no automated test exists yet.

## One change per commit

The working tree often holds unrelated, uncommitted work of the author. Stage only the
hunks that belong to this change:

- Stage a whole file only if every change in it belongs to this commit.
- Otherwise build the index from a patch: prepare the change on top of `HEAD` in a
  temporary worktree, then `git diff --cached > change.patch` there and
  `git apply --cached change.patch` in the main checkout.
- Never commit `build/` output or files the author did not ask to include.
- Keep each file's line endings. Some files are CRLF (`timeslice.cpp`, the linker scripts,
  the startup file): edit them with tools that preserve `\r\n` (e.g. Python
  `open(path, newline='')`), and check `git diff --cached --stat` for a whole-file rewrite
  before committing.
- Never run `git stash` inside a helper worktree: the stash is shared with the main
  checkout and mixes with the author's own stashes.

## Verify before committing

Build the staged state, not the working tree, since the two differ whenever unrelated
work is left unstaged:

```
git worktree add --detach <tmp> HEAD
git diff --cached | git -C <tmp> apply --index
cmake --preset qemu -S <tmp> -B <tmp>/build/qemu && cmake --build <tmp>/build/qemu
git worktree remove --force <tmp>
```

Use the build commands that exist at that commit: before `CMakePresets.json` landed,
configure with `cmake -S app/multi_thread -B <dir> -DARCH_DEFINED=ARMV7M`.

For changes to the kernel or to `kernel/arch/armv7m`, also run the QEMU regression tests
in both timeslice configurations:

```
ctest --preset qemu && ctest --preset qemu-stress
```

A bug fix adds a test to `tests/qemu/` (listed in `YESRTOS_QEMU_TESTS` in its
CMakeLists.txt) that fails on the old code and passes on the fix. Check both directions:
a test that also passes on the buggy code does not guard anything. Prefer forcing the
interleaving (e.g. yielding with `request_context_switch()` inside a critical section)
over hoping SysTick lands in the right place, and make multi-thread tests independent of
the order threads first run in (a 10 kHz tick preempts anywhere). Repeat new tests, e.g.
`ctest --preset qemu-stress --repeat until-fail:15`, before trusting them.

## Do not

- Push, amend or rebase unless the author asks.
- Commit when the build of the staged state fails; report the failure instead.
