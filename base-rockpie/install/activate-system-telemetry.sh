#!/bin/sh
# Incremental ROCK Pi activation. Preserves the existing MQTT identity,
# certificates, commissioning locks, Ethernet and control configuration.
set -eu

[ "$(id -u)" -eq 0 ] || { echo 'Run with sudo on ROCK Pi.' >&2; exit 1; }
repo="${GRIDEX_EDGE_REPO:-/home/antouan/gridex-edge-gateway}"
source_dir="$repo"
env_file=/etc/gridex/gridex-rockpie.env
service_file=/etc/systemd/system/gridex-rockpie.service
binary=/usr/local/bin/gridex_rockpie_service

[ -f "$env_file" ] || { echo "Existing config not found: $env_file" >&2; exit 1; }
[ -f "$service_file" ] || { echo "Existing service unit not found: $service_file" >&2; exit 1; }
[ -x "$binary" ] || { echo "Existing service binary not found: $binary" >&2; exit 1; }
grep -q '^EnvironmentFile=-/etc/gridex/gridex-rockpie.env' "$service_file" || { echo 'Unexpected systemd EnvironmentFile; refusing activation.' >&2; exit 1; }

for gate in GRIDEX_APPROVE_ADDRESSING GRIDEX_APPROVE_POWER_SIGN GRIDEX_APPROVE_SCALING GRIDEX_APPROVE_INT32_WORD_ORDER; do
    value=$(sed -n "s/^${gate}=//p" "$env_file" | tail -n 1)
    case "$value" in 0|"0"|'0') ;; *) echo "$gate must remain explicitly zero; refusing activation." >&2; exit 1;; esac
done

command -v git >/dev/null || { echo 'git is required on the existing ROCK image.' >&2; exit 1; }
command -v cmake >/dev/null || { echo 'cmake is required on the existing ROCK image.' >&2; exit 1; }
command -v pkg-config >/dev/null || { echo 'pkg-config is required on the existing ROCK image.' >&2; exit 1; }
pkg-config --exists libmosquitto || { echo 'libmosquitto-dev/runtime is required; no MQTT build attempted.' >&2; exit 1; }
broker=$(sed -n 's/^GRIDEX_MQTT_BROKER_URL=//p' "$env_file" | tail -n 1)
case "$broker" in mqtts://*) ;; *) echo 'Existing MQTT broker is not mqtts://; refusing activation.' >&2; exit 1;; esac
ca=$(sed -n 's/^GRIDEX_MQTT_CA_FILE=//p' "$env_file" | tail -n 1)
crt=$(sed -n 's/^GRIDEX_MQTT_CLIENT_CERT_FILE=//p' "$env_file" | tail -n 1)
key=$(sed -n 's/^GRIDEX_MQTT_CLIENT_KEY_FILE=//p' "$env_file" | tail -n 1)
for certificate in "$ca" "$crt" "$key"; do [ -s "$certificate" ] || { echo "MQTT certificate/key missing: $certificate" >&2; exit 1; }; done
openssl verify -purpose sslclient -CAfile "$ca" "$crt" >/dev/null || { echo 'MQTT client certificate verification failed.' >&2; exit 1; }
openssl x509 -in "$crt" -pubkey -noout > /tmp/gridex-client-pub.$$
openssl pkey -in "$key" -pubout | cmp -s - /tmp/gridex-client-pub.$$ || { rm -f /tmp/gridex-client-pub.$$; echo 'MQTT certificate/key mismatch.' >&2; exit 1; }
rm -f /tmp/gridex-client-pub.$$

if [ -d "$repo/.git" ]; then
    git -C "$repo" fetch origin feat/rock-temperature
    git -C "$repo" checkout feat/rock-temperature
    git -C "$repo" pull --ff-only origin feat/rock-temperature
else
    source_dir=$(mktemp -d /tmp/gridex-edge-source.XXXXXX)
    git clone --depth 1 --branch feat/rock-temperature https://github.com/antouanbg/gridex-edge-gateway.git "$source_dir"
fi
if grep -q 'healthPublisher.pump();' "$source_dir/base-rockpie/src/main.cpp" ||
   ! grep -q 'mosquitto_loop_start' "$source_dir/base-rockpie/src/MqttHealthPublisher.cpp"; then
    [ "$source_dir" = "$repo" ] || rm -rf "$source_dir"
    echo 'Fetched an obsolete MQTT candidate; refusing activation.' >&2
    exit 1
fi

build_dir=$(mktemp -d /tmp/gridex-rock-telemetry.XXXXXX)
backup_dir=/var/backups/gridex-rock-telemetry-$(date +%Y%m%d%H%M%S)
mkdir -p "$backup_dir"
cleanup() { rm -rf "$build_dir"; [ "$source_dir" = "$repo" ] || rm -rf "$source_dir"; }
trap cleanup EXIT
cmake -S "$source_dir/base-rockpie" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release \
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
start_at=$(date --iso-8601=seconds)
systemctl restart gridex-rockpie
first_pid=$(systemctl show gridex-rockpie -p MainPID --value)
sleep 45
last_pid=$(systemctl show gridex-rockpie -p MainPID --value)
mqtt_connected=$(journalctl -u gridex-rockpie --since "$start_at" --no-pager -o cat | grep -c '"mqtt_connect_result":0' || true)
system_published=$(journalctl -u gridex-rockpie --since "$start_at" --no-pager -o cat | grep -c '"system_telemetry_publish":true' || true)
if ! systemctl is-active --quiet gridex-rockpie || [ "$first_pid" = 0 ] || [ "$first_pid" != "$last_pid" ] ||
   [ "$mqtt_connected" -lt 1 ] || [ "$system_published" -lt 1 ]; then
    echo "New service did not remain stable; restoring the previous binary and config." >&2
    systemctl stop gridex-rockpie
    cp -p "$backup_dir/gridex_rockpie_service" "$binary"
    cp -p "$backup_dir/gridex-rockpie.env" "$env_file"
    systemctl start gridex-rockpie
    previous_pid=$(systemctl show gridex-rockpie -p MainPID --value)
    sleep 10
    restored_pid=$(systemctl show gridex-rockpie -p MainPID --value)
    if ! systemctl is-active --quiet gridex-rockpie || [ "$previous_pid" = 0 ] || [ "$previous_pid" != "$restored_pid" ]; then
        systemctl stop gridex-rockpie
        echo "Rollback binary also failed; service stopped to prevent a restart loop." >&2
    fi
    echo "ROCK_SYSTEM_TELEMETRY_FAILED backup=$backup_dir" >&2
    exit 1
fi
echo "ROCK_SYSTEM_TELEMETRY_ACTIVE backup=$backup_dir mqtt_connects=$mqtt_connected local_publishes=$system_published"
journalctl -u gridex-rockpie --since '10 seconds ago' --no-pager | tail -n 20
