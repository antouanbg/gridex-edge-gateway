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

## Mandatory Pull Request workflow / Задължителен Pull Request процес

- Every completed change set must be committed on a named branch, pushed to
  `origin` and given a Pull Request before it is reported as ready for review.
- Target `main` unless an explicitly documented dependency requires another
  base branch. State scope, tests and commissioning limits in the PR.
- Never merge automatically. Report the URL and wait for the project owner's
  review/merge decision.
- If PR creation is blocked, record the branch, commit SHA and exact blocker
  in `CODEX_STATE.md` and `HANDOFF.md` where applicable.

- Всяка завършена промяна се commit-ва в именуван branch, push-ва се към
  `origin` и получава Pull Request, преди да бъде докладвана като готова за
  review.
- Целта е `main`, освен ако документирана зависимост не изисква друга base
  branch. В PR-а се описват обхватът, тестовете и commissioning ограниченията.
- Не merge-вай автоматично. Докладвай URL и изчакай review/merge решение на
  собственика на проекта.
- При блокиран PR запиши branch-а, commit SHA и точното препятствие в
  `CODEX_STATE.md` и при нужда в `HANDOFF.md`.
