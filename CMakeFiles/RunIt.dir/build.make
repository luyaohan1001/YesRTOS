# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.29.0/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.29.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/lhan/Projects/YesRTOS

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/lhan/Projects/YesRTOS

# Include any dependencies generated for this target.
include CMakeFiles/RunIt.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/RunIt.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/RunIt.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RunIt.dir/flags.make

CMakeFiles/RunIt.dir/src/scheduler.cpp.o: CMakeFiles/RunIt.dir/flags.make
CMakeFiles/RunIt.dir/src/scheduler.cpp.o: src/scheduler.cpp
CMakeFiles/RunIt.dir/src/scheduler.cpp.o: CMakeFiles/RunIt.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/lhan/Projects/YesRTOS/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RunIt.dir/src/scheduler.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/RunIt.dir/src/scheduler.cpp.o -MF CMakeFiles/RunIt.dir/src/scheduler.cpp.o.d -o CMakeFiles/RunIt.dir/src/scheduler.cpp.o -c /Users/lhan/Projects/YesRTOS/src/scheduler.cpp

CMakeFiles/RunIt.dir/src/scheduler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/RunIt.dir/src/scheduler.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lhan/Projects/YesRTOS/src/scheduler.cpp > CMakeFiles/RunIt.dir/src/scheduler.cpp.i

CMakeFiles/RunIt.dir/src/scheduler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/RunIt.dir/src/scheduler.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lhan/Projects/YesRTOS/src/scheduler.cpp -o CMakeFiles/RunIt.dir/src/scheduler.cpp.s

CMakeFiles/RunIt.dir/src/test.cpp.o: CMakeFiles/RunIt.dir/flags.make
CMakeFiles/RunIt.dir/src/test.cpp.o: src/test.cpp
CMakeFiles/RunIt.dir/src/test.cpp.o: CMakeFiles/RunIt.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/lhan/Projects/YesRTOS/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/RunIt.dir/src/test.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/RunIt.dir/src/test.cpp.o -MF CMakeFiles/RunIt.dir/src/test.cpp.o.d -o CMakeFiles/RunIt.dir/src/test.cpp.o -c /Users/lhan/Projects/YesRTOS/src/test.cpp

CMakeFiles/RunIt.dir/src/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/RunIt.dir/src/test.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lhan/Projects/YesRTOS/src/test.cpp > CMakeFiles/RunIt.dir/src/test.cpp.i

CMakeFiles/RunIt.dir/src/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/RunIt.dir/src/test.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lhan/Projects/YesRTOS/src/test.cpp -o CMakeFiles/RunIt.dir/src/test.cpp.s

CMakeFiles/RunIt.dir/src/thread.cpp.o: CMakeFiles/RunIt.dir/flags.make
CMakeFiles/RunIt.dir/src/thread.cpp.o: src/thread.cpp
CMakeFiles/RunIt.dir/src/thread.cpp.o: CMakeFiles/RunIt.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/lhan/Projects/YesRTOS/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/RunIt.dir/src/thread.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/RunIt.dir/src/thread.cpp.o -MF CMakeFiles/RunIt.dir/src/thread.cpp.o.d -o CMakeFiles/RunIt.dir/src/thread.cpp.o -c /Users/lhan/Projects/YesRTOS/src/thread.cpp

CMakeFiles/RunIt.dir/src/thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/RunIt.dir/src/thread.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lhan/Projects/YesRTOS/src/thread.cpp > CMakeFiles/RunIt.dir/src/thread.cpp.i

CMakeFiles/RunIt.dir/src/thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/RunIt.dir/src/thread.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lhan/Projects/YesRTOS/src/thread.cpp -o CMakeFiles/RunIt.dir/src/thread.cpp.s

# Object files for target RunIt
RunIt_OBJECTS = \
"CMakeFiles/RunIt.dir/src/scheduler.cpp.o" \
"CMakeFiles/RunIt.dir/src/test.cpp.o" \
"CMakeFiles/RunIt.dir/src/thread.cpp.o"

# External object files for target RunIt
RunIt_EXTERNAL_OBJECTS =

RunIt: CMakeFiles/RunIt.dir/src/scheduler.cpp.o
RunIt: CMakeFiles/RunIt.dir/src/test.cpp.o
RunIt: CMakeFiles/RunIt.dir/src/thread.cpp.o
RunIt: CMakeFiles/RunIt.dir/build.make
RunIt: CMakeFiles/RunIt.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/lhan/Projects/YesRTOS/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable RunIt"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RunIt.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RunIt.dir/build: RunIt
.PHONY : CMakeFiles/RunIt.dir/build

CMakeFiles/RunIt.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RunIt.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RunIt.dir/clean

CMakeFiles/RunIt.dir/depend:
	cd /Users/lhan/Projects/YesRTOS && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/lhan/Projects/YesRTOS /Users/lhan/Projects/YesRTOS /Users/lhan/Projects/YesRTOS /Users/lhan/Projects/YesRTOS /Users/lhan/Projects/YesRTOS/CMakeFiles/RunIt.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/RunIt.dir/depend
