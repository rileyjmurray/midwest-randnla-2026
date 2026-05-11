cmake_minimum_required(VERSION 3.18)
project(suparla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Workshop docker layout: RandLAPACK is installed at /workshop/install-randlapack.
# Generalizing this to a user-overridable hint is left for the polished version.
if(NOT DEFINED RandLAPACK_DIR AND EXISTS "/workshop/install-randlapack/lib/cmake/RandLAPACK/RandLAPACKConfig.cmake")
    set(RandLAPACK_DIR "/workshop/install-randlapack/lib/cmake/RandLAPACK")
endif()

find_package(RandLAPACK CONFIG REQUIRED)

pybind11_add_module(_suparla
    src/suparla_module.cc
    src/bqrrp_binding.cc
)
target_link_libraries(_suparla PRIVATE RandLAPACK::RandLAPACK)

# /opt/lapp/lib is not on the system's default library search path, so embed it
# as an RPATH on the compiled extension. BUILD_WITH_INSTALL_RPATH ensures it
# applies to the editable build output, not just to an installed copy.
set_target_properties(_suparla PROPERTIES
    INSTALL_RPATH "/opt/lapp/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)

install(TARGETS _suparla LIBRARY DESTINATION suparla)
