cmake_minimum_required(VERSION 3.3)
project(CSCE_3600_Minor_Assignment_7_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

#set(SOURCE_FILES cliMinor7.c)

include_directories(.)

# add_executable(client ${SOURCE_FILES})

# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_executable(client cliMinor7.c)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
# target_include_directories (Hello PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})