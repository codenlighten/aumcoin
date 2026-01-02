# AumCoin Build Environment
# Ubuntu 20.04 LTS with OpenSSL 1.1.1 (Phase 2.1 - BigNum compatible)
# NOTE: Will upgrade to 3.x in Phase 2.2 after BigNum refactoring

FROM ubuntu:20.04

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

# Verify OpenSSL version (should be 1.1.1 for BigNum compatibility)
RUN openssl version && \
    (openssl version | grep -q "OpenSSL 1.1.1" && echo "✅ OpenSSL 1.1.1 detected - BigNum compatible") || \
    (openssl version | grep -q "OpenSSL 3\." && echo "⚠️  OpenSSL 3.x detected - BigNum needs refactoring") || \
    (echo "ℹ️  OpenSSL $(openssl version) detected")

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
