# GrideX Edge gateway — Working rules

## Architecture and safety

The Site Router is the WireGuard peer. ROCK Pi and ESP/OLIMEX devices are
behind it and do not run WireGuard. ESP32 nodes expose local OT Modbus TCP to
ROCK Pi; ROCK Pi is the sole site MQTT bridge through the Site Router VPN.
There is no public MQTT listener, direct cloud route to OT/BESS, or site-to-site
routing. Vendor mappings remain in device drivers and physical writes require
manufacturer confirmation, local limits and fail-safe handling.

## Bilingual documentation — mandatory

For every user-facing, architecture, safety, protocol or operational text:

1. English is the canonical section and Bulgarian is its corresponding section.
2. Update both languages in the same commit whenever meaning changes.
3. Keep figures, data paths, register semantics, roles, units, defaults and
   safety conditions identical; translate wording, never alter meaning.
4. Use the approved project terminology: `Site = Обект`, `Edge gateway = Edge
   шлюз`, `self-consumption = собствено потребление`, and `EFC = еквивалентни
   пълни цикли`.
5. Do not translate code identifiers, protocol names or product brands.

Before a documentation commit, inspect both EN and BG sections for semantic
parity. Repository state is authoritative; do not preserve obsolete translations
when the canonical architecture changes.

## Task recovery

Read this file, `CODEX_STATE.md` and `HANDOFF.md` when present, then inspect
`git status` before a change. Every `HANDOFF.md` must identify its repository
directly under the title as `Repository / GitHub: <owner>/<repository>` and be
updated for all incomplete, untested or uncommissioned work.
