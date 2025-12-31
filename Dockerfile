# AumCoin Build Environment
# Ubuntu 20.04 with OpenSSL 1.1.x for compatibility with v0.6.3c codebase

FROM ubuntu:20.04

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    libboost-all-dev \
    libdb++-dev \
    libdb-dev \
    libminiupnpc-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

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
