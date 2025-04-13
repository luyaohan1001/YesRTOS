# Difference between 'STATIC' and 'SHARED' keyword in CMAKE.
  1. 'STATIC' library, i.e. file_name.a, is linked at compile time.
  2. 'SHARED' library, i.e. file_name.so, is linked (or loaded) at run time.

  Example:
  ```
  add_library(LibBareMetal STATIC
    ${CMAKE_CURRENT_LIST_DIR}/trace.c
    ${CMAKE_CURRENT_LIST_DIR}/timeslice.cpp
  )
  ```

# Difference between ${CMAKE_SOURCE_DIR} and ${CMAKE_CURRENT_LIST_DIR}
  1. ${CMAKE_SOURCE_DIR}: Directory path of where the CMake command is run.
  2. ${CMAKE_CURRENT_LIST_DIR}: Directory path of the *current* parse CMakeLists.txt.

  * Use ${CMAKE_CURRENT_LIST_DIR} is suggested, because the relative path will work in relative to location of *current* parsed CMakeList.txt always.

# CMake is running recursively:
  1. Check whether there is header guard in .cmake module.
  2. Check whether there is recursive add_subdirectory
  3. Check root cause for:
  ```
  -- Configuring done (0.6s)
  You have changed variables that require your cache to be deleted.
  Configure will be re-run and you may have to reset some variables.
  The following variables have changed:
  CMAKE_C_COMPILER= /Library/Developer/CommandLineTools/usr/bin/cc
  CMAKE_CXX_COMPILER= /Library/Developer/CommandLineTools/usr/bin/c++
  ```
