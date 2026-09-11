# GrideX Edge gateway — Working Rules

## Architecture and safety

ROCK Pi and ESP/OLIMEX devices run behind a dedicated Site Router. The Site
Router is the WireGuard peer; do not put WireGuard on ROCK Pi or ESP firmware.
No public MQTT listener, direct cloud route to the OT/BESS network, or
site-to-site routing is allowed.

Vendor-specific Modbus, RS-485, RS-232 and CAN mappings belong in individual
drivers. Never guess addresses, sign, scale, byte/word order or allowed writes.
Manufacturer documentation and bench validation are required before enabling
control. Keep live BMS limits, heartbeat, software fuse and safe-state logic
local; a cloud strategy must never bypass them.

Never commit keys, passwords, certificates, tokens, real IP/VPN ranges or
customer hardware inventory. Use placeholders and deployment configuration.

## Required task workflow

1. Read this file, `CODEX_STATE.md` when present, and `HANDOFF.md`.
2. Inspect `git status`, branch and relevant protocol docs before editing.
3. Preserve unrelated work; run the applicable unit/build tests.
4. Document protocol or architecture changes in English and Bulgarian where
   practical, and inspect diffs for accidental secrets before committing.

## Mandatory handoff policy

`HANDOFF.md` is the durable repository backlog for planned work that is not
implemented, bench-tested, commissioned or deployment-verified.

- Read it at the start of every repository task.
- Update it before the final response and before final commit/push whenever
  work remains deferred, blocked or awaiting hardware/infrastructure.
- Each entry needs dependencies, acceptance evidence and one exact next action.
  Remove it only after required source, test and commissioning evidence exists.
- Never include secrets, real network details or customer data, and never call
  a related task complete while its handoff entry is stale.
