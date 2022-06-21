cmake_minimum_required (VERSION 3.22)

project ("cpp-projects")

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS "-fsanitize=undefined -fsanitize=address -Wall -Wextra -Wuninitialized")
set (CMAKE_VERBOSE_MAKEFILE ON)

# Include sub-projects.
add_subdirectory ("static_ranges")
