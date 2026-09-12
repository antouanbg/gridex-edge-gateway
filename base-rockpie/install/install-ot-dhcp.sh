#!/bin/sh
# Installs the local, interface-bound OT DHCP service. Run as root on ROCK Pi.
set -eu

# Resolve paths from this script, not from the caller's current directory.
script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repository_root=$(CDPATH= cd -- "$script_directory/../.." && pwd)

renderer_source="$repository_root/base-rockpie/ot-dhcp/gridex-render-ot-dhcp"
unit_source="$repository_root/base-rockpie/systemd/gridex-ot-dhcp.service"
# A CMake-installed utility lives beside its renderer; the unit resides in
# systemd's local unit directory. Prefer the repository layout for staged
# commissioning, but keep the installed utility usable as well.
if [ ! -f "$renderer_source" ] || [ ! -f "$unit_source" ]; then
    renderer_source="$script_directory/gridex-render-ot-dhcp"
    unit_source=/usr/local/lib/systemd/system/gridex-ot-dhcp.service
fi
[ -f "$renderer_source" ] || {
    echo "cannot find gridex-render-ot-dhcp" >&2
    exit 1
}
[ -f "$unit_source" ] || {
    echo "cannot find gridex-ot-dhcp.service" >&2
    exit 1
}

command -v apt-get >/dev/null 2>&1 || {
    echo 'apt-get is required on the approved Armbian/Debian image' >&2
    exit 1
}
if ! command -v dnsmasq >/dev/null 2>&1; then
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y dnsmasq
fi

install -d -m 0750 /etc/gridex /usr/local/lib/gridex /var/lib/gridex
install -m 0750 "$renderer_source" \
    /usr/local/lib/gridex/gridex-render-ot-dhcp
install -m 0644 "$unit_source" \
    /etc/systemd/system/gridex-ot-dhcp.service

systemctl daemon-reload
systemctl enable --now gridex-ot-dhcp.service
# `enable --now` does not reload an already active process after a unit update.
systemctl restart gridex-ot-dhcp.service
