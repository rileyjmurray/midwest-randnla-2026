cmake_minimum_required(VERSION 3.18)
project(suparla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Locate the workshop RandLAPACK install (header-only; brings in RandBLAS,
# BLAS++, LAPACK++, Random123, and OpenMP transitively).
if(NOT RandLAPACK_DIR)
    set(RandLAPACK_DIR "/workshop/install-randlapack/lib/cmake/RandLAPACK")
endif()

# FindRandom123.cmake (pulled in transitively via RandBLAS) contains
#   install(FILES CMake/FindRandom123.cmake ...)
# which uses a path relative to our source tree that doesn't exist.  Pre-load
# the Random123 exports file so the "if (NOT TARGET Random123)" guard in
# FindRandom123.cmake fires and skips those install() calls entirely.
get_filename_component(_rl_cmake_dir "${RandLAPACK_DIR}/.." ABSOLUTE)
if(NOT TARGET Random123 AND EXISTS "${_rl_cmake_dir}/Random123.cmake")
    include("${_rl_cmake_dir}/Random123.cmake")
endif()
unset(_rl_cmake_dir)

find_package(RandLAPACK REQUIRED)

pybind11_add_module(_suparla src/suparla_module.cc)
target_link_libraries(_suparla PRIVATE RandLAPACK::RandLAPACK)

# Embed the BLAS++/LAPACK++ runtime library path so Python can dlopen the
# extension without requiring LD_LIBRARY_PATH to be set.
set_target_properties(_suparla PROPERTIES
    INSTALL_RPATH           "/opt/lapp/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)

install(TARGETS _suparla LIBRARY DESTINATION suparla)
