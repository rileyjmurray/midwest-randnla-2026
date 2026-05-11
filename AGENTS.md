# AGENTS.md

This repo is the workshop environment for building a Python package called `suparla` that exposes selected [RandLAPACK](https://github.com/BallisticLA/RandLAPACK) drivers as functions over NumPy arrays. A workshop participant will tell you which driver to bind and how thoroughly to design the API.

## Where things live

This workspace is meant to be opened *inside* a Docker container set up by [project-instructions.md](project-instructions.md). Under `/workshop/` (the container mount point):

- `repo-randlapack/` — RandLAPACK source. RandBLAS is included as a submodule under `repo-randlapack/RandBLAS/`.
- `install-randlapack/` — installed RandLAPACK headers and CMake config, populated by the participant's initial `make install`.
- `build-randlapack/` — out-of-source build directory for RandLAPACK itself.
- `bindings/` — where the `suparla` Python package source lives. **Put new binding code here.**

## Python environment

A conda env named `py313` is set up per [project-instructions.md](project-instructions.md) and contains Python 3.13, NumPy, SciPy, Matplotlib, and pybind11. Activate it before running Python or building bindings:

```bash
conda activate py313
```

If `py313` doesn't exist yet, follow the "configure conda" section of [project-instructions.md](project-instructions.md) to create it.

## Building and importing the bindings

From `/workshop/bindings/`:

```bash
pip install -e .
```

After this succeeds, `import suparla as su` should work in any Python session with the `py313` env active.

## Dependency paths (for CMake)

These are baked into the container image as environment variables:

| Variable | Value | What it is |
| --- | --- | --- |
| `BLASPP_DIR` | `/opt/lapp/lib/cmake/blaspp` | BLAS++ CMake config |
| `LAPACKPP_DIR` | `/opt/lapp/lib/cmake/lapackpp` | LAPACK++ CMake config |
| `R123_DIR` | `/opt/random123/include` | Random123 (header-only) |

The installed RandLAPACK lives at `/workshop/install-randlapack` after the participant has built it.

## Scope of this file

This file is intentionally limited to *infrastructural* facts — where things are, how to build, what the package is named. It deliberately does not document algorithm-specific design choices. Those are part of the participant's prompt and the agent's job to discover or to be told.
