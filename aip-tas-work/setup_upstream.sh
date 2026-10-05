#!/bin/bash
# Builds the TAS tools on upstream DDNet (no aip-tas.zip needed): clones ddnet at DDNET_BASE_COMMIT.txt into ./ddnet
# (next to the saved src/tas sources), applies ddnet-upstream.patch (TAS targets in CMakeLists.txt, the trimmed server
# replay test TasReplay.Run, opt-in exact fast paths in CCollision / CCharacterCore), builds the tools and testrunner.
# The map: put the KoG AiP-Gores.map (sha256 353b27cf...) at tas-work/AiP-Gores.map first.
# Run from aip-tas-work/: bash setup_upstream.sh
set -e
cd "$(dirname "$0")"
ROOT=$(pwd)
BASE=$(cat DDNET_BASE_COMMIT.txt)

if [ ! -d ddnet/.git ]; then
	TMP=$(mktemp -d)
	git clone --filter=blob:none https://github.com/ddnet/ddnet "$TMP/ddnet"
	git -C "$TMP/ddnet" checkout -q "$BASE"
	cp -a "$TMP/ddnet/." ddnet/
	rm -rf "$TMP"
	cd ddnet
	git checkout -q "$BASE" -- .
	git apply ../ddnet-upstream.patch
	# the physics-experiments test file needs code that is not upstream; keep it for reference only
	mkdir -p src/tas-ref
	if [ -f src/test/zz_physics_sim_test.cpp ]; then git mv -f src/test/zz_physics_sim_test.cpp src/tas-ref/ 2> /dev/null || mv src/test/zz_physics_sim_test.cpp src/tas-ref/; fi
	cd "$ROOT"
fi

command -v cargo > /dev/null || { curl -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal; . "$HOME/.cargo/env"; }
mkdir -p ddnet/build-sim
cd ddnet/build-sim
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCLIENT=OFF -DDOWNLOAD_GTEST=ON -DMYSQL=OFF -DWEBSOCKETS=OFF \
	-DDISCORD=OFF -DSTEAM=OFF -DUPNP=OFF -DAUTOUPDATE=OFF -DVIDEORECORDER=OFF
ninja -j"${JOBS:-$(nproc)}" lab seg pre polish polish2 pf match mapdump bench testrunner

cd "$ROOT/tas-work"
../ddnet/build-sim/mapdump AiP-Gores.map > map.txt
echo "--- check: the 977 run (expect 'grenade pickup at input 1045 ... race tick 977', 0 mismatches)"
./simcmp.py ../pre_grenade_kog/kog_pregren_977.txt
