# Use a modern Ubuntu base with good C++20/23 support (GCC 13)
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# 1. Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libgtest-dev \
    libopenblas-dev \
    liblapack-dev \
    libomp-dev \
    && rm -rf /var/lib64/apt/lists/*

# 2. Install Miniconda (architecture-aware)
RUN ARCH=$(uname -m) \
    && wget -q "https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-${ARCH}.sh" -O /tmp/miniconda.sh \
    && bash /tmp/miniconda.sh -b -p /opt/conda \
    && rm /tmp/miniconda.sh

ENV PATH="/opt/conda/bin:$PATH"

RUN conda init bash && conda clean -afy

WORKDIR /build

# 3. Install Random123 (Header-only dependency)
RUN git clone https://github.com/DEShawResearch/random123.git /opt/random123

# 4. Build and Install BLAS++ (Required compiled dependency)
# Note: We force 64-bit integers for BLAS to avoid issues with large matrices
RUN git clone https://github.com/icl-utk-edu/blaspp.git \
    && mkdir build-lapp && cd build-lapp \
    && cmake ../blaspp \
        -DCMAKE_BUILD_TYPE=Release \
        -Dbuild_tests=OFF \
        -Dblas_int=int64 \
        -DCMAKE_INSTALL_PREFIX=/opt/lapp \
    && make -j$(nproc) install \
    && rm -rf * \
    && cd ..

# 5. Build and Install LAPACK++ (Required compiled dependency)
RUN git clone https://github.com/icl-utk-edu/lapackpp.git \
    && cd build-lapp \
    && cmake ../lapackpp \
        -DCMAKE_BUILD_TYPE=Release \
        -Dbuild_tests=OFF \
        -Dblaspp_DIR=/opt/lapp/lib/cmake/blaspp \
        -DCMAKE_INSTALL_PREFIX=/opt/lapp \
    && make -j$(nproc) install \
    && cd / \
    && rm -rf /build

# /workshop is reserved as the user's mount point
WORKDIR /workshop

# Set environment variables
ENV OMP_NUM_THREADS=4
ENV BLASPP_DIR=/opt/lapp/lib/cmake/blaspp
ENV LAPACKPP_DIR=/opt/lapp/lib/cmake/lapackpp
ENV R123_DIR=/opt/random123/include


CMD ["/bin/bash"]
