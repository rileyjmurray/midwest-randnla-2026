"""Tests for su.bqrrp — Block QR with Randomized Pivoting."""
import numpy as np
import pytest
import suparla as su


def make_matrix(m, n, rank=None, seed=0):
    rng = np.random.default_rng(seed)
    if rank is None or rank >= min(m, n):
        return rng.standard_normal((m, n))
    # Exactly rank-k: SVD of a random matrix with the trailing singular values
    # set to exactly zero so BQRRP's epsilon-based threshold can detect them.
    A_full = rng.standard_normal((m, n))
    U, s, Vt = np.linalg.svd(A_full, full_matrices=False)
    s[rank:] = 0.0
    return (U * s) @ Vt


# ---------------------------------------------------------------------------
# Shape and type checks
# ---------------------------------------------------------------------------

def test_output_shapes():
    m, n = 80, 40
    A = make_matrix(m, n)
    A_fact, tau, J, rank = su.bqrrp(A, d_factor=2.0, block_size=16, seed=0)
    assert A_fact.shape == (m, n)
    assert tau.shape == (n,)
    assert J.shape == (n,)
    assert isinstance(rank, int)


def test_output_dtypes():
    A = make_matrix(60, 30)
    A_fact, tau, J, rank = su.bqrrp(A)
    assert A_fact.dtype == np.float64
    assert tau.dtype == np.float64
    assert J.dtype == np.int64


def test_fortran_order_output():
    A = make_matrix(60, 30)
    A_fact, _, _, _ = su.bqrrp(A)
    assert A_fact.flags["F_CONTIGUOUS"]


def test_original_not_modified():
    A = make_matrix(50, 25)
    A_copy = A.copy()
    su.bqrrp(A)
    np.testing.assert_array_equal(A, A_copy)


# ---------------------------------------------------------------------------
# Pivot sanity
# ---------------------------------------------------------------------------

def test_J_is_valid_permutation():
    m, n = 60, 30
    A = make_matrix(m, n)
    _, _, J, _ = su.bqrrp(A, block_size=8)
    assert set(J.tolist()) == set(range(n)), "J must be a permutation of 0..n-1"


def test_rank_in_range():
    m, n = 60, 30
    A = make_matrix(m, n)
    _, _, _, rank = su.bqrrp(A)
    assert 0 < rank <= min(m, n)


# ---------------------------------------------------------------------------
# Factorization correctness: A[:, J] ~= Q @ R
# ---------------------------------------------------------------------------

def _reconstruct_q(A_fact, tau, rank):
    """Form the first `rank` columns of Q from a GEQP3-format factored matrix."""
    from scipy.linalg import get_lapack_funcs
    (dorgqr,) = get_lapack_funcs(("orgqr",), (A_fact,))
    # Workspace query: scipy returns (q, work, info); optimal lwork is in work[0].
    _, work, _ = dorgqr(A_fact[:, :rank], tau[:rank], lwork=-1)
    lwork = int(work[0].real)
    Q, _, info = dorgqr(A_fact[:, :rank], tau[:rank], lwork=lwork)
    assert info == 0
    return Q  # shape (m, rank)


def test_factorization_full_rank():
    """For a generic full-rank matrix, ||A[:,J] - Q@R|| / ||A|| should be ~machine epsilon."""
    pytest.importorskip("scipy")
    m, n = 80, 40
    A = make_matrix(m, n, seed=1)
    A_fact, tau, J, rank = su.bqrrp(A, d_factor=2.0, block_size=16, seed=0)

    Q = _reconstruct_q(A_fact, tau, rank)
    R = np.triu(A_fact[:rank, :])

    residual = np.linalg.norm(A[:, J] - Q @ R) / np.linalg.norm(A)
    assert residual < 1e-10, f"Residual {residual:.2e} too large"


def test_factorization_low_rank():
    """For a rank-k matrix the factorization residual should vanish."""
    pytest.importorskip("scipy")
    m, n, k = 80, 40, 10
    A = make_matrix(m, n, rank=k, seed=2)
    A_fact, tau, J, rank = su.bqrrp(A, d_factor=2.0, block_size=8, seed=0)

    Q = _reconstruct_q(A_fact, tau, rank)
    R = np.triu(A_fact[:rank, :])

    residual = np.linalg.norm(A[:, J] - Q @ R) / (np.linalg.norm(A) + 1e-30)
    assert residual < 1e-10, f"Residual {residual:.2e} too large"


# ---------------------------------------------------------------------------
# Input flexibility
# ---------------------------------------------------------------------------

def test_c_contiguous_input():
    """C-contiguous input should be accepted and give the same result as F-contiguous."""
    pytest.importorskip("scipy")
    A_c = np.ascontiguousarray(make_matrix(60, 30, seed=3))
    A_f = np.asfortranarray(A_c)
    r_c = su.bqrrp(A_c, block_size=8, seed=7)
    r_f = su.bqrrp(A_f, block_size=8, seed=7)
    np.testing.assert_array_equal(r_c[2], r_f[2])  # same pivots
    assert r_c[3] == r_f[3]                          # same rank


def test_seed_reproducibility():
    A = make_matrix(60, 30, seed=4)
    r1 = su.bqrrp(A, seed=42)
    r2 = su.bqrrp(A, seed=42)
    np.testing.assert_array_equal(r1[2], r2[2])
    assert r1[3] == r2[3]
