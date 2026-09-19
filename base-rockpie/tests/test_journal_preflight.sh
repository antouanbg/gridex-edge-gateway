#!/bin/sh
set -eu
checker=$1
testdir=$(mktemp -d)
trap 'chmod u+w "$testdir" "$testdir/journal" 2>/dev/null || :; rm -f "$testdir/journal" "$testdir/journal.1"; rmdir "$testdir"' EXIT
export GRIDEX_TELEMETRY_JOURNAL_PATH="$testdir/journal"
sh "$checker"
[ ! -e "$testdir/journal" ]
printf 'existing record\n' > "$testdir/journal"
sh "$checker"
[ "$(cat "$testdir/journal")" = 'existing record' ]
ln -s "$testdir/journal" "$testdir/journal.1"
if sh "$checker" 2>/dev/null; then exit 1; fi
rm "$testdir/journal.1"
if [ "$(id -u)" != 0 ]; then
  chmod 0400 "$testdir/journal"
  if sh "$checker" 2>/dev/null; then exit 1; fi
  chmod 0600 "$testdir/journal"
  chmod 0500 "$testdir"
  if sh "$checker" 2>/dev/null; then exit 1; fi
  chmod 0700 "$testdir"
fi
GRIDEX_TELEMETRY_JOURNAL_ENABLED=0 GRIDEX_TELEMETRY_JOURNAL_PATH=/missing/journal sh "$checker"
if GRIDEX_TELEMETRY_JOURNAL_PATH=relative sh "$checker" 2>/dev/null; then exit 1; fi
echo 'journal preflight tests passed'
