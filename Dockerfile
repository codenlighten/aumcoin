# AumCoin Build Environment
# Ubuntu 24.04 LTS with OpenSSL 3.3.x for modern security (Phase 2)

FROM ubuntu:24.04

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    pkg-config \
    libboost-all-dev \
    libdb++-dev \
    libdb-dev \
    libminiupnpc-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Verify OpenSSL version (should be 3.3.x)
RUN openssl version && \
    openssl version | grep -q "OpenSSL 3\." || \
    (echo "ERROR: OpenSSL 3.x required" && exit 1)

# Set working directory
WORKDIR /aumcoin

# Copy source code (including .git for version info)
COPY . .

# Create obj directories
RUN mkdir -p /aumcoin/src/obj /aumcoin/src/obj-test

# Build aumcoind
WORKDIR /aumcoin/src
RUN make -f makefile.unix

# Expose ports
EXPOSE 9443 9442

# Create data directory
RUN mkdir -p /root/.aumcoin

# Default command
CMD ["./aumcoind", "-printtoconsole"]
