cmake_minimum_required(VERSION 3.3)
project(CSCE_3600_Minor_Assignment_7_server)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

# set(SOURCE_FILES svrMinor7.c)

include_directories(.)

#add_executable(server svrMinor7.c)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable(server svrMinor7.c)

# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# helloDemo
# target_link_libraries (helloDemo LINK_PUBLIC Hello)