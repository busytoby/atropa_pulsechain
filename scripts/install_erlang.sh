#!/usr/bin/env bash
set -euo pipefail

# Define local installation paths
PREFIX_DIR="/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/erlang"
BUILD_DIR="/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/build_erlang"

echo "=== Preparing Directories ==="
mkdir -p "$PREFIX_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "=== Downloading Erlang/OTP 26.2.5.2 Source ==="
if [ ! -f "otp_src_26.2.5.2.tar.gz" ]; then
    curl -L -o "otp_src_26.2.5.2.tar.gz" "https://github.com/erlang/otp/releases/download/OTP-26.2.5.2/otp_src_26.2.5.2.tar.gz"
fi

echo "=== Extracting Erlang/OTP Source ==="
if [ ! -d "otp_src_26.2.5.2" ]; then
    tar -xzf "otp_src_26.2.5.2.tar.gz"
fi

cd "otp_src_26.2.5.2"

echo "=== Configuring Erlang Build (Prefix: $PREFIX_DIR) ==="
./configure --prefix="$PREFIX_DIR" \
            --without-javac \
            --without-odbc \
            --enable-sharing-preserving \
            --enable-dirty-schedulers

echo "=== Compiling Erlang ==="
make -j$(nproc)

echo "=== Installing Erlang ==="
make install

export PATH="$PREFIX_DIR/bin:$PATH"

echo "=== Erlang Installation Verified: ==="
"$PREFIX_DIR/bin/erl" -version

cd "$BUILD_DIR"

echo "=== Cloning rebar3 ==="
if [ ! -d "rebar3" ]; then
    git clone https://github.com/erlang/rebar3.git
fi

cd rebar3
echo "=== Building rebar3 from Source ==="
./bootstrap

echo "=== Copying rebar3 to Erlang bin directory ==="
cp rebar3 "$PREFIX_DIR/bin/"

echo "=== rebar3 Installation Verified: ==="
"$PREFIX_DIR/bin/rebar3" --version

echo "==============================================="
echo "Installation complete!"
echo "Erlang and rebar3 are installed at: $PREFIX_DIR/bin"
echo "To use them, add to your PATH:"
echo "export PATH=\"$PREFIX_DIR/bin:\$PATH\""
echo "==============================================="
