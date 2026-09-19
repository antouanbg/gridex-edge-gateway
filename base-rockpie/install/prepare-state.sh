#!/bin/sh
# Run after cmake --install on the target, before enabling the service.
# Uses the installed declarative account/state definitions; no secret settings.
set -eu
[ "$(id -u)" = 0 ] || { echo 'Run this preparation as root.' >&2; exit 1; }
command -v systemd-sysusers >/dev/null
command -v systemd-tmpfiles >/dev/null
systemd-sysusers /usr/local/lib/sysusers.d/gridex.conf
systemd-tmpfiles --create /usr/local/lib/tmpfiles.d/gridex.conf
echo 'GrideX service account and journal permissions prepared; service not enabled or restarted.'
