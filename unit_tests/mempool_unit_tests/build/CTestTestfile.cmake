# CMake generated Testfile for 
# Source directory: /Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests
# Build directory: /Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(mempool_unit_test0 "./mempool_unit_test" "alloc size < 32")
set_tests_properties(mempool_unit_test0 PROPERTIES  _BACKTRACE_TRIPLES "/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;29;add_test;/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;0;")
add_test(mempool_unit_test1 "./mempool_unit_test" "alloc and free size < 32")
set_tests_properties(mempool_unit_test1 PROPERTIES  _BACKTRACE_TRIPLES "/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;30;add_test;/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;0;")
add_test(mempool_unit_test2 "./mempool_unit_test" "alloc size 128")
set_tests_properties(mempool_unit_test2 PROPERTIES  _BACKTRACE_TRIPLES "/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;31;add_test;/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;0;")
add_test(mempool_unit_test3 "./mempool_unit_test" "invalid size")
set_tests_properties(mempool_unit_test3 PROPERTIES  _BACKTRACE_TRIPLES "/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;32;add_test;/Users/lhan/Projects/YesRTOS/unit_tests/mempool_unit_tests/CMakeLists.txt;0;")
