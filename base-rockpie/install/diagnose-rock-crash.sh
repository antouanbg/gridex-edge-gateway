#!/bin/sh
# Read-only ROCK Pi crash evidence. Do not print the protected deployment env.
set -u

if [ "$(id -u)" -ne 0 ]; then
    echo 'Run with sudo on ROCK Pi.' >&2
    exit 1
fi

echo 'ROCK_SERVICE_STATUS'
systemctl show gridex-rockpie \
    -p ActiveState -p SubState -p MainPID -p NRestarts \
    -p ExecMainStatus -p Result --no-pager

echo 'ROCK_BINARY'
if [ -f /usr/local/bin/gridex_rockpie_service ]; then
    sha256sum /usr/local/bin/gridex_rockpie_service
    ldd /usr/local/bin/gridex_rockpie_service | grep -E 'mosquitto|ssl|crypto|pthread|stdc\+\+' || true
fi

echo 'ROCK_RECENT_SERVICE_LOG'
journalctl -u gridex-rockpie -n 55 --no-pager -o short-iso

echo 'ROCK_KERNEL_CRASH_LOG'
journalctl -k --since '2 hours ago' --no-pager -o short-iso \
    | grep -Ei 'gridex_rockpie|segfault|invalid pointer|futex|FD_SETSIZE' \
    | tail -n 25 || true

echo 'ROCK_COREDUMP'
if command -v coredumpctl >/dev/null 2>&1; then
    coredumpctl --no-pager info gridex_rockpie_service 2>&1 | tail -n 100 || true
else
    echo 'coredumpctl is not installed; no core backtrace available from this command.'
fi
