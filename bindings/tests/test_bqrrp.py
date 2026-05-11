"""Correctness and rough-edge tests for the rough bqrrp binding."""
import numpy as np

import suparla as su


def _check_qr(A, Q, R, J):
    k = min(*A.shape)
    assert Q.shape == (A.shape[0], k)
    assert R.shape == (k, A.shape[1])
    assert J.shape == (A.shape[1],)
    assert np.allclose(Q.T @ Q, np.eye(k), atol=1e-8)
    inv_J = np.argsort(J)
    assert np.allclose(A, (Q @ R)[:, inv_J], atol=1e-6 * np.linalg.norm(A))


def test_bqrrp_square():
    rng = np.random.default_rng(0)
    A = np.asfortranarray(rng.standard_normal((80, 80)))
    Q, R, J = su.bqrrp(A, seed=42)
    _check_qr(A, Q, R, J)


def test_bqrrp_tall():
    rng = np.random.default_rng(1)
    A = np.asfortranarray(rng.standard_normal((200, 80)))
    Q, R, J = su.bqrrp(A, seed=42)
    _check_qr(A, Q, R, J)


def test_bqrrp_does_not_mutate_input():
    rng = np.random.default_rng(2)
    A = np.asfortranarray(rng.standard_normal((40, 20)))
    A_orig = A.copy()
    su.bqrrp(A, seed=7)
    assert np.array_equal(A, A_orig)


def test_bqrrp_rejects_c_order():
    rng = np.random.default_rng(3)
    A = rng.standard_normal((50, 30))
    assert not A.flags.f_contiguous
    try:
        su.bqrrp(A)
    except RuntimeError as e:
        assert "Fortran" in str(e)
        return
    raise AssertionError("Expected RuntimeError for C-order input")


def test_bqrrp_rejects_float32():
    rng = np.random.default_rng(4)
    A = np.asfortranarray(rng.standard_normal((50, 30)).astype(np.float32))
    try:
        su.bqrrp(A)
    except RuntimeError as e:
        assert "float64" in str(e)
        return
    raise AssertionError("Expected RuntimeError for float32 input")


if __name__ == "__main__":
    test_bqrrp_square()
    test_bqrrp_tall()
    test_bqrrp_does_not_mutate_input()
    test_bqrrp_rejects_c_order()
    test_bqrrp_rejects_float32()
    print("ok")
