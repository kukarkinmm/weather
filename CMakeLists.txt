cmake_minimum_required(VERSION 3.17)
project(weather)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS REQUIRED coroutine program_options)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    link_libraries(${Boost_LIBRARIES})
endif ()
message(${Boost_LIBRARIES})
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)

include(cmake/targets.cmake)

add_subdirectory(src)

add_executable(weather main.cpp)
target_link_libraries(weather client json local_storage)