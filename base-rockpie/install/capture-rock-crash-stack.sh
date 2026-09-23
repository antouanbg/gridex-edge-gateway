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
if [ -e "$override_file" ]; then
    echo "Existing debug override found: $override_file" >&2
    exit 1
fi

temporary_dir=$(mktemp -d /tmp/gridex-rock-crash.XXXXXX)
cleanup() {
    systemctl stop gridex-rockpie >/dev/null 2>&1 || true
    rm -f "$override_file"
    systemctl daemon-reload >/dev/null 2>&1 || true
    systemctl reset-failed gridex-rockpie >/dev/null 2>&1 || true
    rm -f "$temporary_dir/rock-crash-gdb.conf"
    rmdir "$temporary_dir" >/dev/null 2>&1 || true
}
trap cleanup EXIT HUP INT TERM

curl -fsSL 'https://raw.githubusercontent.com/antouanbg/gridex-edge-gateway/feat/rock-temperature/base-rockpie/install/rock-crash-gdb.conf' -o "$temporary_dir/rock-crash-gdb.conf"
install -d -m 0755 "$override_dir"
install -m 0644 "$temporary_dir/rock-crash-gdb.conf" "$override_file"
systemctl daemon-reload
systemctl reset-failed gridex-rockpie

echo 'ROCK_CRASH_CAPTURE_START'
start_at=$(date --iso-8601=seconds)
systemctl start gridex-rockpie || true
sleep 12
systemctl stop gridex-rockpie >/dev/null 2>&1 || true
journalctl -u gridex-rockpie --since "$start_at" --no-pager -o cat | tail -n 140
echo 'ROCK_CRASH_CAPTURE_END; service will remain stopped.'
