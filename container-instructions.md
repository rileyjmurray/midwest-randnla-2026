# Docker Image: randlapack-python-workshop

## What's included

- Ubuntu 24.04
- Build tools: GCC, CMake
- System libraries: OpenBLAS, LAPACK, OpenMP
- Miniconda (`conda` available in `bash` on launch)
- Random123 (header-only, at `/opt/random123/include`)
- BLAS++ and LAPACK++ (compiled and installed to `/opt/lapp`)

## Getting Docker

Download and install **Docker Desktop** from https://www.docker.com/products/docker-desktop/. It provides the Docker engine, CLI, and a GUI for managing images and containers. After installing, launch Docker Desktop and confirm it is running (the whale icon appears in your menu/system tray) before using any `docker` commands.

Verify the installation:

```bash
docker --version
```

At any time you can use the following command to see all available images:
```bash
docker images
```

Or to see all containers (running and stopped):
```bash
docker ps -a
```

## Building a new `randlapack-python-workshop` image

From this directory (`docker-workflow/`):

```bash
docker build -t randlapack-python-workshop .
```

The build takes several minutes — BLAS++ and LAPACK++ are compiled from source.

The Miniconda installer is selected automatically based on the host architecture (`uname -m`), so the build works on both x86-64 and Apple Silicon (ARM64) hosts.

## Running a container

The following command launches the `randlapack-python-workshop` Docker image
in interactive mode (`-it`), allowing the container to use up to 8 GB of RAM (`--memory=8g`). It bind-mounts the current working directory into the container at `/workshop`.

```bash
docker run -it --memory=8g -v `pwd`:/workshop randlapack-python-workshop
```

*On Docker Desktop (Mac/Windows):* the total RAM available to *all* containers combined is controlled by the Docker Desktop settings (Settings → Resources → Memory). The `--memory` flag then sub-limits within that pool. If you do not set `--memory`, the container can use the entire pool.

## Environment variables set in the image

| Variable | Value |
|---|---|
| `OMP_NUM_THREADS` | `4` |
| `BLASPP_DIR` | `/opt/lapp/lib/cmake/blaspp` |
| `LAPACKPP_DIR` | `/opt/lapp/lib/cmake/lapackpp` |
| `R123_DIR` | `/opt/random123/include` |

## Using conda

Conda is initialized for bash on launch. You can create and activate environments normally:

```bash
conda create -n myenv python=3.11
conda activate myenv
```
