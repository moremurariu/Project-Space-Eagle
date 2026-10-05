#!/bin/bash
# build a physlab4 tool (single thread, nice'd) against the existing build-sim objects (nothing existing is modified)
set -e
HERE=$(cd $(dirname $0); pwd)
B=$HERE/../../ddnet/build-sim
S=$HERE/../../ddnet/src
NAME=$1
cd $B
OBJS=$(grep "^build match:" build.ninja | sed 's/^build match: CXX_EXECUTABLE_LINKER__match_Release //; s/ |.*//' | tr ' ' '\n' | grep -v 'match.dir/src/tas/match.cpp.o' | tr '\n' ' ')
nice -n 15 /usr/bin/c++ -DCONF_INFORM_UPDATE -DCONF_OPENSSL -DGAME_RELEASE_VERSION_INTERNAL=20.2 -DGLEW_STATIC -D_FILE_OFFSET_BITS=64 \
  -I$S/tas/shim -I$B/src -I$S -I$S/rust-bridge -O2 -DNDEBUG -std=c++20 -fPIE -fno-exceptions -fsigned-char -w \
  -c $HERE/$NAME.cpp -o $HERE/$NAME.o
nice -n 15 /usr/bin/c++ -O2 -fPIE -pie $HERE/$NAME.o $OBJS -o $HERE/$NAME /usr/lib/x86_64-linux-gnu/libcrypto.so /usr/lib/x86_64-linux-gnu/libcurl.so /usr/lib/x86_64-linux-gnu/libsqlite3.so /usr/lib/x86_64-linux-gnu/libz.so release/libddnet_engine_shared.a -lrt -ldl -fuse-ld=lld
echo built $HERE/$NAME
