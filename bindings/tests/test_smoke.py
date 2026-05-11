"""Smoke test verifying the suparla extension module loads and is callable."""
import suparla as su


def test_hello():
    assert su.hello() == "Hello from suparla!"


def test_version():
    assert isinstance(su.__version__, str)


if __name__ == "__main__":
    test_hello()
    test_version()
    print("ok")
