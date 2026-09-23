#!/bin/sh
# Opt in the existing ROCK Pi CPU sensor only. Never touches Ethernet, MQTT
# identity, device control or commissioning approval settings.
set -eu

[ "$(id -u)" -eq 0 ] || { echo 'Run with sudo on ROCK Pi.' >&2; exit 1; }
env_file=/etc/gridex/gridex-rockpie.env
service=gridex-rockpie
[ -f "$env_file" ] || { echo 'ROCK configuration missing.' >&2; exit 1; }
systemctl is-active --quiet "$service" || { echo 'ROCK service is not active; no changes made.' >&2; exit 1; }
grep -q '^GRIDEX_SYSTEM_TELEMETRY_ENABLED=1$' "$env_file" || {
    echo 'System telemetry is not active; no changes made.' >&2; exit 1;
}
for gate in GRIDEX_APPROVE_ADDRESSING GRIDEX_APPROVE_POWER_SIGN GRIDEX_APPROVE_SCALING GRIDEX_APPROVE_INT32_WORD_ORDER; do
    [ "$(sed -n "s/^${gate}=//p" "$env_file" | tail -n 1)" = 0 ] || {
        echo "Commissioning lock $gate differs from zero; no changes made." >&2; exit 1;
    }
done

path=$(sed -n 's/^GRIDEX_CPU_TEMPERATURE_FILE=//p' "$env_file" | tail -n 1)
[ -n "$path" ] || path=/sys/class/thermal/thermal_zone0/temp
case "$path" in /sys/class/thermal/thermal_zone*/temp) ;; *) echo 'Unexpected CPU sensor path; no changes made.' >&2; exit 1;; esac
[ -r "$path" ] && su -s /bin/sh gridex -c "test -r '$path'" || {
    echo "CPU sensor unreadable by service user: $path" >&2; exit 1;
}
raw=$(sed -n '1p' "$path")
printf '%s\n' "$raw" | grep -Eq '^-?[0-9]+$' || {
    echo 'CPU sensor is not an integer millidegree value.' >&2; exit 1;
}
awk -v n="$raw" 'BEGIN { exit !(n >= -40000 && n <= 150000) }' || {
    echo 'CPU sensor value is outside safe range.' >&2; exit 1;
}
if grep -q '^GRIDEX_CPU_TEMPERATURE_ENABLED=1$' "$env_file"; then
    echo "ROCK_CPU_TEMPERATURE_ALREADY_ENABLED sensor=$path"; exit 0
fi

backup=$(mktemp -d /var/backups/gridex-cpu-temperature.XXXXXX)
chmod 0700 "$backup"
cp -p "$env_file" "$backup/gridex-rockpie.env"
rollback() {
    echo 'CPU temperature did not pass the stability/publish check; restoring configuration.' >&2
    cp -p "$backup/gridex-rockpie.env" "$env_file"
    systemctl restart "$service"
    echo "ROCK_CPU_TEMPERATURE_FAILED backup=$backup" >&2
    exit 1
}
trap rollback HUP INT TERM
sed -i -e '/^GRIDEX_CPU_TEMPERATURE_ENABLED=/d' -e '/^GRIDEX_CPU_TEMPERATURE_FILE=/d' "$env_file"
{
    echo 'GRIDEX_CPU_TEMPERATURE_ENABLED=1'
    echo "GRIDEX_CPU_TEMPERATURE_FILE=$path"
} >> "$env_file"
chmod 0640 "$env_file"
started=$(date --iso-8601=seconds)
systemctl restart "$service" || rollback
first_pid=$(systemctl show "$service" -p MainPID --value)
sleep 45
last_pid=$(systemctl show "$service" -p MainPID --value)
if ! systemctl is-active --quiet "$service" || [ "$first_pid" = 0 ] ||
   [ "$first_pid" != "$last_pid" ] ||
   ! journalctl -u "$service" --since "$started" --no-pager -o cat |
     grep -Eq '"system_telemetry_publish":true.*"samples":6'; then
    rollback
fi
trap - HUP INT TERM
echo "ROCK_CPU_TEMPERATURE_ACTIVE sensor=$path backup=$backup"
