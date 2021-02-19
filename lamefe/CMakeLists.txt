cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DNDEBUG)

file(GLOB lamefeSource "*.h" "*.cpp")

include_directories("../lame/include")

link_directories("../lame/libmp3lame/.libs")

add_executable (lamefe ${lamefeSource})

target_compile_options(lamefe PUBLIC "-O3")

target_link_libraries(lamefe "libmp3lame.a" pthread stdc++fs)
