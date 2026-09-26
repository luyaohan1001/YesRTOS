#include "atomic_section.hpp"

#if defined(ARMV7M)
/*
 * Why the destructor restores the saved PRIMASK instead of writing 0 (enable) or 1 (disable):
 *
 * Critical sections nest, e.g. a caller already inside an atomic_section calls Mutex::unlock(), which opens its own:
 *
 *   {
 *     atomic_section outer;      // PRIMASK 0 -> 1
 *     ...
 *     {
 *       atomic_section inner;    // PRIMASK 1 -> 1
 *     }                          // must leave PRIMASK at 1: outer is still running
 *     ...                        // still protected
 *   }                            // PRIMASK 1 -> 0
 *
 * - Always writing 0 on exit would re-enable exceptions when the inner section ends, so the rest of the outer section
 *   runs unprotected and SysTick/PendSV can switch threads in the middle of it.
 * - Always writing 1 on exit would leave exceptions disabled forever after the outermost section ends.
 *
 * Only the value PRIMASK had on entry is correct for every nesting level, so the constructor saves it and the destructor
 * puts it back. Exceptions made pending inside the section (e.g. PendSV from request_context_switch()) are taken as soon
 * as the outermost section restores PRIMASK to 0.
 */
atomic_section::atomic_section() {
  this->saved_primask = save_and_disable_exception();
}

atomic_section::~atomic_section() {
  restore_exception(this->saved_primask);
}
#endif