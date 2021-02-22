cmake_minimum_required (VERSION 2.6)
project (Elastic_Rosetta)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall -march=native -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -march=native -std=c++11")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(test)
# add_subdirectory(bench)

