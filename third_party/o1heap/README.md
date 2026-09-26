# o1heap (vendored)

Constant-time deterministic memory allocator for hard real-time systems, by Pavel Kirienko. MIT License, see
[LICENSE](LICENSE).

- Upstream: https://github.com/pavel-kirienko/o1heap
- Version: 3.0, commit `388a73fd9007300e5130c5fe352d9ce3288b6dde`
- Files: `o1heap.c`, `o1heap.h` and `LICENSE`, copied unmodified.

YesRTOS uses it through `YesRTOS::Heap` (`kernel/include/heap.hpp`), which adds the locking o1heap leaves to the
caller. Build options are set in `kernel/src/o1heap_config.h`. To update, copy the same three files from a newer
upstream release and update the version and commit above.
