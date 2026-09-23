#!/bin/sh
# Incremental ROCK Pi activation. Preserves the existing MQTT identity,
# certificates, commissioning locks, Ethernet and control configuration.
set -eu

[ "$(id -u)" -eq 0 ] || { echo 'Run with sudo on ROCK Pi.' >&2; exit 1; }
repo="${GRIDEX_EDGE_REPO:-/home/antouan/gridex-edge-gateway}"
env_file=/etc/gridex/gridex-rockpie.env
service_file=/etc/systemd/system/gridex-rockpie.service
binary=/usr/local/bin/gridex_rockpie_service

[ -d "$repo/.git" ] || { echo "Existing source checkout not found: $repo" >&2; exit 1; }
[ -f "$env_file" ] || { echo "Existing config not found: $env_file" >&2; exit 1; }
[ -f "$service_file" ] || { echo "Existing service unit not found: $service_file" >&2; exit 1; }
[ -x "$binary" ] || { echo "Existing service binary not found: $binary" >&2; exit 1; }

for gate in GRIDEX_APPROVE_ADDRESSING GRIDEX_APPROVE_POWER_SIGN GRIDEX_APPROVE_SCALING GRIDEX_APPROVE_INT32_WORD_ORDER; do
    value=$(sed -n "s/^${gate}=//p" "$env_file" | tail -n 1)
    case "$value" in 0|"0"|'0') ;; *) echo "$gate must remain explicitly zero; refusing activation." >&2; exit 1;; esac
done

command -v git >/dev/null || { echo 'git is required on the existing ROCK image.' >&2; exit 1; }
command -v cmake >/dev/null || { echo 'cmake is required on the existing ROCK image.' >&2; exit 1; }
command -v pkg-config >/dev/null || { echo 'pkg-config is required on the existing ROCK image.' >&2; exit 1; }
pkg-config --exists libmosquitto || { echo 'libmosquitto-dev/runtime is required; no MQTT build attempted.' >&2; exit 1; }

git -C "$repo" fetch origin feat/rock-temperature
git -C "$repo" checkout feat/rock-temperature
git -C "$repo" pull --ff-only origin feat/rock-temperature

build_dir=$(mktemp -d /tmp/gridex-rock-telemetry.XXXXXX)
backup_dir=/var/backups/gridex-rock-telemetry-$(date +%Y%m%d%H%M%S)
mkdir -p "$backup_dir"
cmake -S "$repo/base-rockpie" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
    -DGRIDEX_ENABLE_PRIVATE_MQTT=ON -DGRIDEX_REQUIRE_MQTT=ON
cmake --build "$build_dir" --target gridex_rockpie_service --parallel 2
ldd "$build_dir/gridex_rockpie_service" | grep 'libmosquitto' >/dev/null

cp -p "$binary" "$backup_dir/gridex_rockpie_service"
cp -p "$env_file" "$backup_dir/gridex-rockpie.env"
install -m 0755 "$build_dir/gridex_rockpie_service" "$binary"

sed -i \
    -e '/^GRIDEX_SYSTEM_TELEMETRY_ENABLED=/d' \
    -e '/^GRIDEX_SYSTEM_TELEMETRY_PUBLISH_SECONDS=/d' \
    -e '/^GRIDEX_SYSTEM_DATA_DIRECTORY=/d' \
    -e '/^GRIDEX_BOOT_ID=/d' "$env_file"
{
    echo 'GRIDEX_SYSTEM_TELEMETRY_ENABLED=1'
    echo 'GRIDEX_SYSTEM_TELEMETRY_PUBLISH_SECONDS=30'
    echo 'GRIDEX_SYSTEM_DATA_DIRECTORY=/var/lib/gridex'
    echo "GRIDEX_BOOT_ID=$(cat /proc/sys/kernel/random/boot_id)"
} >> "$env_file"
chmod 0640 "$env_file"

systemctl daemon-reload
systemctl restart gridex-rockpie
systemctl is-active --quiet gridex-rockpie
echo "ROCK_SYSTEM_TELEMETRY_ACTIVE backup=$backup_dir"
journalctl -u gridex-rockpie --since '10 seconds ago' --no-pager | tail -n 20
