/**
 * @file critical_section_nesting.cpp
 * @brief A nested atomic_section must keep exceptions disabled until the outermost section ends.
 */
#include "atomic_section.hpp"
#include "test_support.hpp"

static uint32_t primask() {
  uint32_t value;
  __asm volatile("mrs %0, primask" : "=r"(value));
  return value;
}

int main() {
  itm_initialize();
  yesrtos_test::check(primask() == 0, "exceptions disabled before any section");
  {
    atomic_section outer;
    yesrtos_test::check(primask() == 1, "outer section did not disable exceptions");
    {
      atomic_section inner;
      yesrtos_test::check(primask() == 1, "inner section did not keep exceptions disabled");
    }
    yesrtos_test::check(primask() == 1, "inner section re-enabled exceptions inside the outer one");
  }
  yesrtos_test::check(primask() == 0, "outer section did not restore exceptions");
  yesrtos_test::pass();
}
