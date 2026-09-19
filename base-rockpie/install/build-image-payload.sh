#!/bin/sh
# Native Debian/Armbian builder. Stage an image payload, never replace live files.
set -eu
skip_dependencies=0
if [ "$#" -eq 1 ] && [ "$1" = --skip-dependencies ]; then
    skip_dependencies=1
elif [ "$#" -ne 0 ]; then
    echo 'Usage: sh base-rockpie/install/build-image-payload.sh [--skip-dependencies]' >&2
    exit 2
fi
[ "$(uname -s)" = Linux ] || { echo 'Run inside the Debian/Armbian Linux builder.' >&2; exit 1; }
[ "$(id -u)" -ne 0 ] || { echo 'Run as the build user; sudo is used for dependencies only.' >&2; exit 1; }
source_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
[ -f "$source_dir/src/MqttHealthPublisher.cpp" ] || { echo 'Incorrect ROCK source root.' >&2; exit 1; }
if [ "$skip_dependencies" -eq 0 ]; then
    command -v apt-get >/dev/null
    command -v sudo >/dev/null
    sudo apt-get update
    sudo apt-get install -y --no-install-recommends git ca-certificates openssl cmake g++ make pkg-config libmosquitto-dev
fi
pkg-config --exists libmosquitto
stage=$(mktemp -d "${TMPDIR:-/tmp}/gridex-image-payload.XXXXXX")
cmake -S "$source_dir" -B "$stage/build" -DCMAKE_BUILD_TYPE=Debug \
    -DGRIDEX_ENABLE_PRIVATE_MQTT=ON -DGRIDEX_REQUIRE_MQTT=ON
cmake --build "$stage/build" --parallel 2
[ -x "$stage/build/gridex_rockpie_service" ] || { echo 'ROCK service missing; refusing payload.' >&2; exit 1; }
ctest --test-dir "$stage/build" --output-on-failure
ldd "$stage/build/gridex_rockpie_service" | grep 'libmosquitto' >/dev/null
if ldd "$stage/build/gridex_rockpie_service" | grep 'not found'; then
    echo 'Missing runtime dependency; payload refused.' >&2
    exit 1
fi
DESTDIR="$stage/rootfs" cmake --install "$stage/build" --prefix /usr/local
dpkg-query -W git ca-certificates openssl cmake g++ make pkg-config libmosquitto-dev libmosquitto1 > "$stage/packages.tsv"
sha256sum "$stage/rootfs/usr/local/bin/gridex_rockpie_service" > "$stage/service.sha256"
printf '\nIMAGE_PAYLOAD_READY=%s\n' "$stage"
echo 'Staged only: no live service, device configuration, certificate or Ethernet changes.'
