# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Library/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/cmake/data/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Library/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/cmake/data/CMake.app/Contents/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/cfontas/Desktop/VulkanDemo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/cfontas/Desktop/VulkanDemo/build

# Include any dependencies generated for this target.
include libs/VulkanWindowing/display/CMakeFiles/display.dir/depend.make

# Include the progress variables for this target.
include libs/VulkanWindowing/display/CMakeFiles/display.dir/progress.make

# Include the compile flags for this target's objects.
include libs/VulkanWindowing/display/CMakeFiles/display.dir/flags.make

libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.o: libs/VulkanWindowing/display/CMakeFiles/display.dir/flags.make
libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.o: ../libs/VulkanWindowing/display/window.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/cfontas/Desktop/VulkanDemo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.o"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/display.dir/window.cpp.o -c /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/window.cpp

libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/display.dir/window.cpp.i"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/window.cpp > CMakeFiles/display.dir/window.cpp.i

libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/display.dir/window.cpp.s"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/window.cpp -o CMakeFiles/display.dir/window.cpp.s

libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.o: libs/VulkanWindowing/display/CMakeFiles/display.dir/flags.make
libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.o: ../libs/VulkanWindowing/display/glfw_window.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/cfontas/Desktop/VulkanDemo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.o"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/display.dir/glfw_window.cpp.o -c /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/glfw_window.cpp

libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/display.dir/glfw_window.cpp.i"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/glfw_window.cpp > CMakeFiles/display.dir/glfw_window.cpp.i

libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/display.dir/glfw_window.cpp.s"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/glfw_window.cpp -o CMakeFiles/display.dir/glfw_window.cpp.s

libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.o: libs/VulkanWindowing/display/CMakeFiles/display.dir/flags.make
libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.o: ../libs/VulkanWindowing/display/input_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/cfontas/Desktop/VulkanDemo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.o"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/display.dir/input_manager.cpp.o -c /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/input_manager.cpp

libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/display.dir/input_manager.cpp.i"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/input_manager.cpp > CMakeFiles/display.dir/input_manager.cpp.i

libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/display.dir/input_manager.cpp.s"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display/input_manager.cpp -o CMakeFiles/display.dir/input_manager.cpp.s

# Object files for target display
display_OBJECTS = \
"CMakeFiles/display.dir/window.cpp.o" \
"CMakeFiles/display.dir/glfw_window.cpp.o" \
"CMakeFiles/display.dir/input_manager.cpp.o"

# External object files for target display
display_EXTERNAL_OBJECTS =

libs/VulkanWindowing/display/libdisplay.a: libs/VulkanWindowing/display/CMakeFiles/display.dir/window.cpp.o
libs/VulkanWindowing/display/libdisplay.a: libs/VulkanWindowing/display/CMakeFiles/display.dir/glfw_window.cpp.o
libs/VulkanWindowing/display/libdisplay.a: libs/VulkanWindowing/display/CMakeFiles/display.dir/input_manager.cpp.o
libs/VulkanWindowing/display/libdisplay.a: libs/VulkanWindowing/display/CMakeFiles/display.dir/build.make
libs/VulkanWindowing/display/libdisplay.a: libs/VulkanWindowing/display/CMakeFiles/display.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/cfontas/Desktop/VulkanDemo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library libdisplay.a"
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && $(CMAKE_COMMAND) -P CMakeFiles/display.dir/cmake_clean_target.cmake
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/display.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
libs/VulkanWindowing/display/CMakeFiles/display.dir/build: libs/VulkanWindowing/display/libdisplay.a

.PHONY : libs/VulkanWindowing/display/CMakeFiles/display.dir/build

libs/VulkanWindowing/display/CMakeFiles/display.dir/clean:
	cd /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display && $(CMAKE_COMMAND) -P CMakeFiles/display.dir/cmake_clean.cmake
.PHONY : libs/VulkanWindowing/display/CMakeFiles/display.dir/clean

libs/VulkanWindowing/display/CMakeFiles/display.dir/depend:
	cd /Users/cfontas/Desktop/VulkanDemo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/cfontas/Desktop/VulkanDemo /Users/cfontas/Desktop/VulkanDemo/libs/VulkanWindowing/display /Users/cfontas/Desktop/VulkanDemo/build /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display /Users/cfontas/Desktop/VulkanDemo/build/libs/VulkanWindowing/display/CMakeFiles/display.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : libs/VulkanWindowing/display/CMakeFiles/display.dir/depend

