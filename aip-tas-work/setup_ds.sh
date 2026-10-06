#!/bin/bash
# Builds the TAS tools (incl. x_ds / x_trace / segf / testrunner) on upstream DDNet without aip-tas.zip:
# clones DDNet at DDNET_BASE_COMMIT.txt into ddnet-up/, applies ddnet-upstream-ds.patch (TAS targets, TasReplay server
# test, exact fast paths, prediction-world double-explosion fix), links ddnet-up/src/tas to ddnet/src/tas (this repo's
# sources) and ddnet/build-sim to the build dir. Fetches the KoG map (KoG-teeworlds/maps, sha256 353b27cf...).
# Needs: build-essential cmake ninja-build python3 libcurl4-openssl-dev libsqlite3-dev zlib1g-dev libssl-dev libpng-dev
# and a Rust toolchain (rustup). Run from aip-tas-work/: bash setup_ds.sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
BASE=$(cat DDNET_BASE_COMMIT.txt)
if [ ! -d ddnet-up/.git ]; then
	git clone --filter=blob:none https://github.com/ddnet/ddnet ddnet-up
	git -C ddnet-up checkout -q "$BASE"
	git -C ddnet-up apply "$ROOT/ddnet-upstream-ds.patch"
	rm -rf ddnet-up/src/tas
	ln -s "$ROOT/ddnet/src/tas" ddnet-up/src/tas
fi
mkdir -p ddnet-up/build
cd ddnet-up/build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCLIENT=OFF -DDOWNLOAD_GTEST=ON -DMYSQL=OFF -DWEBSOCKETS=OFF \
	-DDISCORD=OFF -DSTEAM=OFF -DUPNP=OFF -DAUTOUPDATE=OFF -DVIDEORECORDER=OFF
ninja -j"${JOBS:-$(nproc)}" lab seg segf rdv mapdump x_ds x_trace x_kopp testrunner
cd "$ROOT"
ln -sfn "$ROOT/ddnet-up/build" ddnet/build-sim
cd tas-work
if [ ! -f AiP-Gores.map ]; then
	TMP=$(mktemp -d)
	GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 --filter=blob:none --no-checkout https://github.com/KoG-teeworlds/maps "$TMP/maps"
	git -C "$TMP/maps" checkout HEAD -- maps/easy/Aip-Gores.map
	cp "$TMP/maps/maps/easy/Aip-Gores.map" AiP-Gores.map
	rm -rf "$TMP"
fi
sha256sum AiP-Gores.map | grep -q ^353b27cf || { echo "wrong map"; exit 1; }
../ddnet/build-sim/mapdump AiP-Gores.map > map.txt
echo "--- check: the best run on the server (TasReplay)"
./srvfin.sh "$(ls kog_full_2*.txt | sort | head -1)"
