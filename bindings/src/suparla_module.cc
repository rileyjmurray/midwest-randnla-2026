#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <string>
#include <stdexcept>

#include <RandBLAS.hh>
#include <rl_bqrrp.hh>

namespace py = pybind11;

// Run BQRRP on a float64 NumPy matrix and return the factored form.
static py::tuple py_bqrrp(
    py::array_t<double, py::array::f_style | py::array::forcecast> A_in,
    double   d_factor,
    int64_t  block_size,
    uint64_t seed
) {
    auto req = A_in.request();
    if (req.ndim != 2)
        throw std::invalid_argument("A must be a 2-D array");

    int64_t m = req.shape[0];
    int64_t n = req.shape[1];

    // Working copy in column-major order — BQRRP overwrites A in place.
    auto A_work = py::array_t<double, py::array::f_style>({(py::ssize_t)m, (py::ssize_t)n});
    std::copy_n(static_cast<const double*>(req.ptr), m * n, A_work.mutable_data());

    auto tau_arr = py::array_t<double>(n);
    auto J_arr   = py::array_t<int64_t>(n);
    std::fill_n(tau_arr.mutable_data(), n, 0.0);
    std::fill_n(J_arr.mutable_data(),   n, int64_t(0));

    RandBLAS::RNGState<RandBLAS::DefaultRNG> state(seed);
    RandLAPACK::BQRRP<double, RandBLAS::DefaultRNG> alg(/*time_subroutines=*/false, block_size);

    int ret = alg.call(
        m, n,
        A_work.mutable_data(), /*lda=*/m,
        d_factor,
        tau_arr.mutable_data(),
        J_arr.mutable_data(),
        state
    );
    if (ret != 0)
        throw std::runtime_error("BQRRP returned error code " + std::to_string(ret));

    // LAPACK stores pivots 1-indexed; convert to 0-indexed for Python.
    int64_t* J = J_arr.mutable_data();
    for (int64_t i = 0; i < n; ++i)
        J[i] -= 1;

    return py::make_tuple(A_work, tau_arr, J_arr, static_cast<int64_t>(alg.rank));
}

PYBIND11_MODULE(_suparla, m) {
    m.doc() = "suparla — Python bindings for selected RandLAPACK drivers";

    m.def(
        "hello",
        []() -> std::string { return "Hello from suparla!"; },
        "Return a greeting confirming that the suparla extension module loaded."
    );

    m.def(
        "bqrrp",
        &py_bqrrp,
        py::arg("A"),
        py::arg("d_factor")   = 2.0,
        py::arg("block_size") = 64,
        py::arg("seed")       = uint64_t(0),
        R"doc(
Block QR with randomized pivoting (BQRRP).

Computes a rank-revealing QR factorization  A[:, J] = Q @ R  where Q is
unitary and R is upper triangular.  Output follows LAPACK's GEQP3 convention:
the upper triangle of A_fact holds R, and the Householder vectors defining Q
are stored below the diagonal alongside the scalar vector tau.

Parameters
----------
A : array_like, shape (m, n), dtype float64
    Input matrix.  A copy is made; the original is not modified.
d_factor : float, optional
    Sketch-dimension factor.  The sketch has ``d_factor * block_size`` rows.
    Must satisfy ``d_factor >= 1``.  Default 2.0.
block_size : int, optional
    Column-block size for the blocked algorithm.  Default 64.
seed : int, optional
    Non-negative integer seed for the random number generator.  Default 0.

Returns
-------
A_fact : ndarray, shape (m, n), dtype float64, Fortran order
    Factored form: upper triangle is R; below-diagonal entries are the
    Householder vectors implicitly defining Q.
tau : ndarray, shape (n,), dtype float64
    Householder scalars.  Entries beyond index ``rank`` are zero.
J : ndarray, shape (n,), dtype int64
    0-indexed column permutation such that ``A[:, J] ~= Q @ R``.
rank : int
    Estimated numerical rank of A.
)doc"
    );
}
