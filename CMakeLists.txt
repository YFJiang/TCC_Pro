cmake_minimum_required(VERSION 3.16)
project(TCC_Pro CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

file(GLOB_RECURSE TCC_SOURCES "src/*.cpp")
add_executable(TCC_Pro ${TCC_SOURCES})
target_include_directories(TCC_Pro PRIVATE src)
target_compile_features(TCC_Pro PRIVATE cxx_std_17)

# ---- test harness ----
enable_testing()
add_test(NAME equivalence
  COMMAND bash "${CMAKE_SOURCE_DIR}/tests/run_equivalence.sh"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
