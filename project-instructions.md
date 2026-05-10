# RandLAPACK/RandBLAS build instructions with Docker

## Prerequisites

1. *Install Docker Desktop* from https://www.docker.com/products/docker-desktop/. Launch it and confirm it's running (whale icon in the menu/system tray) before continuing.

2. *Give Docker enough RAM.* Docker Desktop → Settings → Resources → Memory. Set to at least 8 GB. The default on some systems (~2 GB) is too small to compile RandLAPACK.

3. *Pre-pull the image* (~2.4 GB) so the download doesn't eat workshop time:
   ```bash
   docker pull ghcr.io/rileyjmurray/randlapack-python-workshop:latest
   ```
   The image is multi-arch — Docker will fetch the right variant for Apple Silicon or Intel/AMD automatically.

4. *`cd` into a working directory* that you want to appear as `/workshop` inside the container. Anything in it will be visible to the container; anything the container writes there will persist on your host. An empty directory is fine — you'll clone RandLAPACK into it in a later step.

For background on Docker concepts (images vs containers, what's actually inside this image, how to build it from scratch), see [container-instructions.md](container-instructions.md).

## Launch docker

```
docker run -it -v `pwd`:/workshop ghcr.io/rileyjmurray/randlapack-python-workshop:latest 
```

## clone, build, and install RandLAPACK

```
git clone --recursive https://github.com/BallisticLA/RandLAPACK.git repo-randlapack
cd build-randlapack
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
cmake \
    -DCMAKE_CXX_FLAGS="-Wno-psabi" \
    -DCMAKE_INSTALL_PREFIX=`pwd`/../install-randlapack \
    -DCMAKE_BUILD_TYPE=Release \
    -Dlapackpp_DIR=$LAPACKPP_DIR \
    -Dblaspp_DIR=$BLASPP_DIR \
    -DRandom123_DIR=$R123_DIR \
    ../repo-randlapack 
make -j4 install
```
