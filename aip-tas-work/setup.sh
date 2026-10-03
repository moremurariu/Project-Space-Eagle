#!/bin/bash
# Builds the TAS tools (server-only DDNet build) and puts the binaries where the tas-work scripts expect them.
# (testrunner, for checking runs on real server code, is optional: cd ddnet/build-sim && ninja testrunner)
# Run from the aip-tas folder: bash setup.sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)

if command -v apt-get > /dev/null; then
	SUDO=$(command -v sudo || true)
	$SUDO apt-get update
	$SUDO apt-get install -y build-essential cmake ninja-build python3 python3-numpy python3-pil pkg-config \
		libcurl4-openssl-dev libsqlite3-dev zlib1g-dev libssl-dev libpng-dev ffmpeg curl
fi
# DDNet needs a recent Rust toolchain; distro packages are often too old
if ! command -v cargo > /dev/null; then
	curl -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal
	. "$HOME/.cargo/env"
fi

mkdir -p ddnet/build-sim
cd ddnet/build-sim
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCLIENT=OFF -DDOWNLOAD_GTEST=ON -DMYSQL=OFF -DWEBSOCKETS=OFF \
	-DDISCORD=OFF -DSTEAM=OFF -DUPNP=OFF -DAUTOUPDATE=OFF -DVIDEORECORDER=OFF
JOBS=${JOBS:-$(nproc 2> /dev/null || echo 2)}
ninja -j"$JOBS" tas lab tas2 polish nest mapdump pre

cd "$ROOT/tas-work"
# the scripts call these names; always rm before cp (overwriting a running binary in place can break it)
for b in tas_exp tas_full tas_full2 tas_full3 tas_chain tas_pad; do
	rm -f $b
	cp ../ddnet/build-sim/tas $b
done
chmod +x *.sh

echo "--- check: replaying the best run (expect rt=2854 at the end, i.e. 57.08 s)"
../ddnet/build-sim/lab AiP-Gores.map "quiet;replay best_57.08.txt;print" | tail -1
