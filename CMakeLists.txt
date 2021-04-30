cmake_minimum_required(VERSION 3.10)
project(fdtd CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)")

aux_source_directory(src SOURCES)
add_executable(fdtd ${SOURCES})

if(NOGEM5)
  set(CMAKE_CXX_FLAGS "-DNOGEM5=true ${CMAKE_CXX_FLAGS}")
  target_link_libraries(fdtd libvl.a)
else()
  target_include_directories(fdtd PRIVATE "/benchmarks/VirtualLink/near-data-sim/include/")
  target_link_libraries(fdtd /usr/local/include/libm5.a) #/benchmarks/VirtualLink/near-data-sim/util/m5/libm5.a)
  target_include_directories(fdtd PRIVATE "/benchmarks/VirtualLink/libvl")
  target_link_libraries(fdtd /usr/local/lib/libvl.a) #/benchmarks/VirtualLink/libvl/libvl/libvl.a)
endif()

target_link_libraries(fdtd ${CMAKE_THREAD_LIBS_INIT})

