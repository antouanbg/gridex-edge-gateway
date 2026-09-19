#!/bin/sh
# Run as the service user, inside the service sandbox. Never repair permissions
# with elevated privileges here, and never append test data to the real journal.
set -eu
case "${GRIDEX_TELEMETRY_JOURNAL_ENABLED:-1}" in
  0|false|FALSE|no|NO|off|OFF) exit 0 ;;
esac
journal=${GRIDEX_TELEMETRY_JOURNAL_PATH:-/var/lib/gridex/telemetry-journal.ndjson}
fail() { echo "GrideX journal preflight failed: $1" >&2; exit 1; }
case "$journal" in /*) ;; *) fail 'journal path must be absolute' ;; esac
parent=$(dirname -- "$journal")
[ ! -L "$journal" ] || fail 'journal must not be a symlink'
[ -d "$parent" ] && [ -w "$parent" ] && [ -x "$parent" ] || fail 'parent directory is not writable by service user'
for file in "$journal" "$journal.1"; do
  [ ! -L "$file" ] || fail 'journal or rotation target is a symlink'
  if [ -e "$file" ]; then
    [ -f "$file" ] && [ -w "$file" ] || fail 'journal or rotation file is not writable by service user'
  fi
done
probe=$(mktemp -d "$parent/.journal-preflight.XXXXXX") || fail 'cannot create rotation probe'
trap 'rm -f -- "$probe/new" "$probe/rotated"; rmdir -- "$probe"' EXIT HUP INT TERM
printf 'probe\n' > "$probe/new" || fail 'cannot write probe'
mv -- "$probe/new" "$probe/rotated" || fail 'cannot rotate probe'
