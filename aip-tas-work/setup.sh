#!/bin/bash
# One-command build of every AiP-Gores TAS tool on upstream DDNet (no aip-tas.zip needed).
#  - clones DDNet at DDNET_BASE_COMMIT.txt into ddnet-up/ and applies ddnet-upstream.patch (TAS build targets, the
#    TasReplay / TasServer server-code replay tests, the merged exact collision fast paths, the prediction-world
#    double-explosion fix)
#  - links ddnet-up/src/tas to ddnet/src/tas (this folder's sources) and ddnet/build-sim to the build dir
#  - fetches the KoG map (KoG-teeworlds/maps, sha256 353b27cf...) to tas-work/AiP-Gores.map
#  - runs the checks: simbench exactness fuzz (every stepper vs plain DDNet code) and server replays of the best runs
# Needs: build-essential cmake ninja-build python3 libcurl4-openssl-dev libsqlite3-dev zlib1g-dev libssl-dev libpng-dev
# and a Rust toolchain (rustup). Optional: ccache (used automatically). Run from aip-tas-work/: bash setup.sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
BASE=$(cat DDNET_BASE_COMMIT.txt)
if [ ! -d ddnet-up/.git ]; then
	git clone --filter=blob:none https://github.com/ddnet/ddnet ddnet-up
	git -C ddnet-up checkout -q "$BASE"
	git -C ddnet-up apply "$ROOT/ddnet-upstream.patch"
	rm -rf ddnet-up/src/tas
	ln -s "$ROOT/ddnet/src/tas" ddnet-up/src/tas
fi
command -v cargo > /dev/null || { curl -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal; . "$HOME/.cargo/env"; }
LAUNCH=""
command -v ccache > /dev/null && LAUNCH="-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
mkdir -p ddnet-up/build
cd ddnet-up/build
[ -f build.ninja ] || cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCLIENT=OFF -DDOWNLOAD_GTEST=ON -DMYSQL=OFF \
	-DWEBSOCKETS=OFF -DDISCORD=OFF -DSTEAM=OFF -DUPNP=OFF -DAUTOUPDATE=OFF -DVIDEORECORDER=OFF $LAUNCH
# the tools the recommended workflows use (README "Which tool for what"); add more names as needed
ninja -j"${JOBS:-$(nproc)}" segf seg x_ds x_tig x_trace x_lob x_dbl x_win x_pert x_graft rdv ddsearch replay fastcheck viacheck \
	eacct eaudit fgcheck lobscan rejoin perturb lab mapdump simbench testrunner
cd "$ROOT"
ln -sfn "$ROOT/ddnet-up/build" ddnet/build-sim
# fast/ (the CFast / ddsearch drivers) expects ddnet/build and kog.map next to its tools/
mkdir -p fast/ddnet && ln -sfn "$ROOT/ddnet-up/build" fast/ddnet/build && ln -sfn "$ROOT/tas-work/AiP-Gores.map" fast/kog.map
cd tas-work
if [ ! -f AiP-Gores.map ]; then
	TMP=$(mktemp -d)
	GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 --filter=blob:none --no-checkout https://github.com/KoG-teeworlds/maps "$TMP/maps"
	git -C "$TMP/maps" checkout HEAD -- maps/easy/Aip-Gores.map
	cp "$TMP/maps/maps/easy/Aip-Gores.map" AiP-Gores.map
	rm -rf "$TMP"
fi
sha256sum AiP-Gores.map | grep -q ^353b27cf || { echo "wrong map"; exit 1; }
[ -f map.txt ] || ../ddnet/build-sim/mapdump AiP-Gores.map > map.txt
echo "--- exactness: every stepper vs plain DDNet prediction code from random inputs (expect 0 mismatches)"
TRIALS=500 ../ddnet/build-sim/simbench AiP-Gores.map kog_full_best.txt fuzz | grep RESULT
echo "--- the best full run and the best pre-grenade run on the real server code"
./srvfin.sh kog_full_best.txt
./srvfin.sh ../pre_grenade_kog/kog_pregren_best.txt
