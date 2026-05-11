cmake_minimum_required(VERSION 3.18)
project(suparla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_suparla src/suparla_module.cc)

install(TARGETS _suparla LIBRARY DESTINATION suparla)
