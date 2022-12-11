cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(circuit-sat-minisat)

#--------------------------------------------------------------------------------------------------
# Configurable options:

option(STATIC_BINARIES "Link binaries statically." ON)
option(USE_SORELEASE   "Use SORELEASE in shared library filename." ON)

#--------------------------------------------------------------------------------------------------
# Library version:

set(SOLVER_SOMAJOR   2)
set(SOLVER_SOMINOR   1)
set(SOLVER_SORELEASE 0)

# Compute VERSION and SOVERSION:
if (USE_SORELEASE)
  set(SOLVER_VERSION ${SOLVER_SOMAJOR}.${SOLVER_SOMINOR}.${SOLVER_SORELEASE})
else()
  set(SOLVER_VERSION ${SOLVER_SOMAJOR}.${SOLVER_SOMINOR})
endif()
set(SOLVER_SOVERSION ${SOLVER_SOMAJOR})

#--------------------------------------------------------------------------------------------------
# Dependencies:

set(circuitsatDirectory "../circuitsat")

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${circuit-sat-minisat_SOURCE_DIR})
include_directories(${circuitsatDirectory})

#--------------------------------------------------------------------------------------------------
# Compile flags:

SET(CMAKE_CXX_FLAGS "-O3")
add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

#--------------------------------------------------------------------------------------------------
# Build Targets:

set(SOLVER_LIB_SOURCES
    solver/utils/Options.cc
    solver/utils/System.cc
    solver/core/Solver.cc
    ${circuitsatDirectory}/core/source/structures/parser.hpp
    ${circuitsatDirectory}/core/source/bench_to_cnf/bench_to_cnf.hpp)

add_library(solver-lib-static STATIC ${SOLVER_LIB_SOURCES})
add_library(solver-lib-shared SHARED ${SOLVER_LIB_SOURCES})

target_link_libraries(solver-lib-shared ${ZLIB_LIBRARY})
target_link_libraries(solver-lib-static ${ZLIB_LIBRARY})

add_executable(csat-solver solver/core/Main.cc)

if(STATIC_BINARIES)
  target_link_libraries(csat-solver solver-lib-static)
else()
  target_link_libraries(csat-solver solver-lib-shared)
endif()

# set_target_properties(solver-lib-static PROPERTIES OUTPUT_NAME "solver")
# set_target_properties(solver-lib-shared
#   PROPERTIES
#     OUTPUT_NAME "solver" 
#     VERSION ${MINISAT_VERSION}
#     SOVERSION ${MINISAT_SOVERSION})

# set_target_properties(minisat_simp       PROPERTIES OUTPUT_NAME "solver")

#--------------------------------------------------------------------------------------------------
# Installation targets:

install(TARGETS solver-lib-static solver-lib-shared csat-solver
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY solver/mtl solver/utils solver/core
        DESTINATION CMAKE_CURRENT_SOURCE_DIR
        FILES_MATCHING PATTERN "*.h")
