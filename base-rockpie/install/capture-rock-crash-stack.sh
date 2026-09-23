#!/bin/sh
# One controlled ROCK Pi crash reproduction. Restores the original systemd
# unit and leaves the service stopped. Never prints the protected env file.
set -eu

[ "$(id -u)" -eq 0 ] || { echo 'Run with sudo on ROCK Pi.' >&2; exit 1; }
if systemctl is-active --quiet gridex-rockpie; then
    echo 'Service is active; refusing to interrupt a running device.' >&2
    exit 1
fi

if ! command -v gdb >/dev/null 2>&1; then
    export DEBIAN_FRONTEND=noninteractive
    apt-get update -qq
    apt-get install -y -qq --no-install-recommends --no-upgrade gdb >/dev/null
fi

override_dir=/run/systemd/system/gridex-rockpie.service.d
override_file=$override_dir/90-gridex-crash-debug.conf
debug_dir=/run/gridex-rock-debug
if [ -e "$override_file" ]; then
    echo "Existing debug override found: $override_file" >&2
    exit 1
fi
if [ -e "$debug_dir" ]; then
    echo "Existing debug path found: $debug_dir" >&2
    exit 1
fi

temporary_dir=$(mktemp -d /tmp/gridex-rock-crash.XXXXXX)
cleanup() {
    systemctl stop gridex-rockpie >/dev/null 2>&1 || true
    rm -f "$override_file"
    systemctl daemon-reload >/dev/null 2>&1 || true
    systemctl reset-failed gridex-rockpie >/dev/null 2>&1 || true
    rm -f "$debug_dir/gridex_rockpie_service"
    rmdir "$debug_dir" >/dev/null 2>&1 || true
    rm -rf "$temporary_dir"
}
trap cleanup EXIT HUP INT TERM

git clone --depth 1 --branch feat/rock-temperature \
    https://github.com/antouanbg/gridex-edge-gateway.git "$temporary_dir/source"
cmake -S "$temporary_dir/source/base-rockpie" -B "$temporary_dir/build" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DGRIDEX_ENABLE_PRIVATE_MQTT=ON -DGRIDEX_REQUIRE_MQTT=ON
cmake --build "$temporary_dir/build" --target gridex_rockpie_service --parallel 2
ldd "$temporary_dir/build/gridex_rockpie_service" | grep libmosquitto
install -d -m 0755 "$debug_dir"
install -m 0755 "$temporary_dir/build/gridex_rockpie_service" \
    "$debug_dir/gridex_rockpie_service"
install -d -m 0755 "$override_dir"
install -m 0644 "$temporary_dir/source/base-rockpie/install/rock-crash-gdb.conf" "$override_file"
systemctl daemon-reload
systemctl reset-failed gridex-rockpie

echo 'ROCK_CRASH_CAPTURE_START'
git -C "$temporary_dir/source" rev-parse --short HEAD
start_at=$(date --iso-8601=seconds)
systemctl start gridex-rockpie || true
# Cover the installer's 45-second stability window plus one publish interval.
sleep 55
systemctl stop gridex-rockpie >/dev/null 2>&1 || true
journalctl -u gridex-rockpie --since "$start_at" --no-pager -o cat | tail -n 140
echo 'ROCK_CRASH_CAPTURE_END; service will remain stopped.'
