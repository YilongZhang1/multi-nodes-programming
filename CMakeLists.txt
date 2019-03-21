cmake_minimum_required(VERSION 3.6) 

set(PROJ_NAME multi_nodes)
set(CMAKE_CXX_FLAGS ${CMAKE_COMPILE_FLAGS} ${MPI_C_COMPILE_FLAGS} "-std=gnu++11 -Wall -Werror")

project(${PROJ_NAME} CXX) 

# need the MPI package
find_package(MPI REQUIRED) 

# common stuff
include_directories (${PROJECT_SOURCE_DIR}/include)
file(GLOB SOURCES src/*.cpp)
add_executable(${PROJ_NAME} ${SOURCES}) 

# mpi related stuff
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH}) 
link_directories(${MPI_C_LINK_FLAGS}) 
 
target_link_libraries(${PROJ_NAME} ${MPI_CXX_LIBRARIES} boost_filesystem-mt boost_system-mt)
