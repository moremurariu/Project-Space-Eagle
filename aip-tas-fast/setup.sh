#!/bin/bash
# Builds the exact fast TAS tools on upstream DDNet (server-only build) and checks them.
# Needs the KoG map as aip-tas-fast/kog.map (sha256 353b27cf72168cd0bb917c56eb46a83ef8d4266f310ec79b68db90f61637deaa).
# Run from anywhere: bash aip-tas-fast/setup.sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
if [ ! -d ddnet ]; then
	git clone https://github.com/ddnet/ddnet ddnet
	git -C ddnet checkout "$(cat DDNET_BASE_COMMIT.txt)"
	git -C ddnet apply ../ddnet.patch
fi
# apt deps (Debian/Ubuntu): build-essential cmake ninja-build python3 libcurl4-openssl-dev libsqlite3-dev zlib1g-dev libssl-dev libpng-dev, plus Rust (cargo)
mkdir -p ddnet/build
cd ddnet/build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCLIENT=OFF -DDOWNLOAD_GTEST=ON -DMYSQL=OFF -DWEBSOCKETS=OFF \
	-DDISCORD=OFF -DSTEAM=OFF -DUPNP=OFF -DAUTOUPDATE=OFF -DVIDEORECORDER=OFF
ninja -j"${JOBS:-$(nproc)}" replay fastcheck eacct ddsearch testrunner
cd "$ROOT"
if [ ! -f kog.map ]; then
	echo "put the KoG map at $ROOT/kog.map to run the checks"
	exit 0
fi
echo "--- fast stepper vs DDNet prediction code (expect 0 mismatches)"
ddnet/build/fastcheck kog.map runs/kog_pregren_978_previous.txt 5000 | tail -3
echo "--- best run on the prediction code"
ddnet/build/replay kog.map runs/best.txt 100000 | grep -E 'GRENADE|frz 1'
echo "--- best run on the real server code"
TAS_MAP=$ROOT/kog.map TAS_INPUTS=$ROOT/runs/best.txt ddnet/build/testrunner --gtest_filter='TasServer.*' | grep -E 'RESULT|PASSED|FAILED'
