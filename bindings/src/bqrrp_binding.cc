#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <RandBLAS.hh>
#include <RandLAPACK.hh>
#include <lapack.hh>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace py = pybind11;

namespace {

// Rough BQRRP binding. Limitations a polished version would fix:
//   - float64 only; float32 inputs raise.
//   - Requires Fortran-order input; C-order raises (call np.asfortranarray first).
//   - Copies the input array internally so the caller's A is preserved.
std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<int64_t>>
bqrrp_rough(
    py::array A,
    int64_t block_size,
    double d_factor,
    uint64_t seed
) {
    auto buf = A.request();

    if (buf.format != py::format_descriptor<double>::format())
        throw std::runtime_error("A must be float64.");

    if (!(A.flags() & py::array::f_style))
        throw std::runtime_error("A must be Fortran-order (column-major). Use np.asfortranarray(A).");

    if (buf.ndim != 2)
        throw std::runtime_error("A must be 2D.");
    const int64_t m = buf.shape[0];
    const int64_t n = buf.shape[1];
    if (m == 0 || n == 0)
        throw std::runtime_error("A must be non-empty.");
    const int64_t k = std::min(m, n);

    const size_t mn = static_cast<size_t>(m) * static_cast<size_t>(n);
    std::vector<double> Awork(mn);
    std::memcpy(Awork.data(), buf.ptr, sizeof(double) * mn);

    std::vector<double> tau(static_cast<size_t>(k));
    std::vector<int64_t> J(static_cast<size_t>(n));

    RandBLAS::RNGState<r123::Philox4x32> state(seed);
    RandLAPACK::BQRRP<double, r123::Philox4x32> bqrrp(/*time_subroutines=*/false, block_size);
    bqrrp.call(m, n, Awork.data(), m, d_factor, tau.data(), J.data(), state);

    const int64_t rank = bqrrp.rank;

    py::array_t<double> R({rank, n});
    auto R_acc = R.mutable_unchecked<2>();
    for (int64_t j = 0; j < n; ++j) {
        for (int64_t i = 0; i < rank; ++i) {
            R_acc(i, j) = (i <= j) ? Awork[i + j * m] : 0.0;
        }
    }

    if (k > 0)
        lapack::orgqr(m, k, k, Awork.data(), m, tau.data());

    py::array_t<double> Q({m, rank});
    auto Q_acc = Q.mutable_unchecked<2>();
    for (int64_t j = 0; j < rank; ++j) {
        for (int64_t i = 0; i < m; ++i) {
            Q_acc(i, j) = Awork[i + j * m];
        }
    }

    py::array_t<int64_t> Jpy(static_cast<py::ssize_t>(n));
    auto J_acc = Jpy.mutable_unchecked<1>();
    for (int64_t i = 0; i < n; ++i) {
        J_acc(i) = J[i] - 1;
    }

    return std::make_tuple(Q, R, Jpy);
}

}  // namespace

void register_bqrrp(py::module_ &m) {
    m.def("bqrrp", &bqrrp_rough,
        py::arg("A"),
        py::arg("block_size") = static_cast<int64_t>(64),
        py::arg("d_factor") = 1.25,
        py::arg("seed") = static_cast<uint64_t>(0),
        R"doc(Randomized blocked QR with column pivoting (rough binding).

Returns (Q, R, J) such that A[:, J] is approximately Q @ R, where Q is
m-by-k orthonormal, R is k-by-n upper triangular, and J is a 0-indexed
length-n permutation. k = min(m, n).

Rough edges this version exposes (the polished step-3 version fixes them):
  - float64 only.
  - Requires Fortran-order input; call np.asfortranarray(A) first if needed.
)doc"
    );
}
