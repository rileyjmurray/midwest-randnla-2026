#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

void register_bqrrp(py::module_ &m);

PYBIND11_MODULE(_suparla, m) {
    m.doc() = "suparla — Python bindings for selected RandLAPACK drivers";

    m.def(
        "hello",
        []() -> std::string { return "Hello from suparla!"; },
        "Return a greeting confirming that the suparla extension module loaded."
    );

    register_bqrrp(m);
}
