SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti")
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
message(STATUS "LLVM ${LLVM_INCLUDE_DIRS}")
add_subdirectory(Mutation)
