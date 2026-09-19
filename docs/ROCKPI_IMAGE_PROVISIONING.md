# ROCK Pi image preparation and enrolment

## English

Scope: reusable preparation for a new ROCK Pi; no image has been built/flashed
by this change. Keep the pinned pilot OS baseline and commissioning locks.
The image includes binaries, service account definitions, default state-directory
permissions and safe templates, **not** a commissioned device clone.

### State preparation

With the standard `/usr/local` install prefix, `cmake --install` now installs
`lib/sysusers.d/gridex.conf`, `lib/tmpfiles.d/gridex.conf` and the journal preflight.
After installation in the target Linux image/host, run as root:

```sh
/usr/local/lib/gridex/prepare-state.sh
```

This creates the non-login gridex account and prepares only the named state/log
directories and current/rotated journal. Existing journal content is not truncated;
no recursive ownership change is requested. Run preparation after restoring state
or installing an upgrade, before starting the service. It does not enable or
restart services, change networks or grant control approval. Image builders using
offline roots must run the corresponding systemd-sysusers/systemd-tmpfiles tools
against that explicit image root, not the build host. Custom install prefixes need
matching service/helper paths; the packaged runtime expects `/usr/local`.

The service checks journal access as gridex inside its sandbox before launch.
It checks existing files and probes write/rename in a temporary subdirectory,
without writing synthetic telemetry. Failure prevents startup with a diagnostic;
fix permissions/storage, do not unlock control or run the service as root.
The canonical directory is 0750 gridex:gridex; journal files are 0640 gridex:gridex.
Custom journal paths stay in the protected device env and require explicitly
provisioned ownership and service sandbox access; they are not silently repaired.

### New device workflow

1. Verify image hash and hardware target; obtain explicit approval before flashing.
   Include no customer env, journal history, MQTT/OTA private keys, claim secrets,
   SSH host keys or copied machine identity in a distributable image. Generate
   machine/SSH identities on first boot; never clone an enrolled board wholesale.
2. Boot with all write gates locked; management DHCP, isolated OT settings only
   through approved provisioning. Keep Site Router as the VPN endpoint.
3. Assign a unique node/gateway identity and per-device credentials through a
   protected enrolment process. A future short-lived single-use claim links the
   device to an authorised Site admin; that UI/backend claim flow remains backlog,
   not implemented by this image preparation. No shared factory password.
4. Admin selects the Site and device roles (maximum two), then approves a versioned
   configuration. ROCK initiates its retrieval; ESP configuration/updates pass
   through ROCK only. Imported config and receipt are not proof of application.
5. Accept activation only after matching revision/readback and health evidence.
   Show ROCK last backend receipt separately from ESP last successful poll and
   advancing heartbeat. Unknown/stale data never becomes online by registration.

### Image release acceptance

- Fresh boot: gridex account, exact modes/ownership, sandboxed preflight pass;
  no credentials or customer history baked into image; unique identities on two clones.
- Upgrade/restore: seed root-owned current and `.1` journals, run preparation,
  verify bytes preserved and writable as gridex; unrelated backup ownership unchanged.
- Rotation/reboot: use a separate temporary test journal and bounded test settings;
  check rollover, permissions after reboot, full-disk failure and recovery. Do not
  rotate/delete a live journal for testing. Retention is not proof of backend ACK.
- Device proof: at least three fresh snapshots with online=true, no poll failures
  and advancing ESP heartbeat; independently verify backend receipt/UI freshness.
- No BESS/PCS writes, relay pulses, automatic VPN activation or root service.

2026-09-19 owner-provided pilot evidence: after correcting root-owned state/journal,
snapshots 9–11 at 19:11:41–19:11:52 UTC showed online=true and heartbeat 674→685→695.
This proves the reported local interval, not current status or backend delivery.
The separate PCS heartbeat remains unconfirmed while commissioning is locked.

## Български

Обхват: подготовка за нов ROCK Pi; тази промяна не изгражда/записва имидж.
Запазват се фиксираната пилотна OS и commissioning заключването. Имиджът съдържа
програми, service account, права и безопасни шаблони, не клонирано прието устройство.

При стандартен prefix `/usr/local`, `cmake --install` инсталира sysusers/tmpfiles
правилата и journal preflight. След инсталацията изпълни като root в целевата
Linux среда `/usr/local/lib/gridex/prepare-state.sh`. Създава gridex без login и
подготвя само именуваните папки и текущия/ротирания журнал. Не изтрива съдържание
и не задава рекурсивна смяна на собственост. Изпълнява се и след restore/upgrade,
преди старт. Не включва/рестартира услуги, мрежи или control права. Offline image
builder използва съответните systemd инструменти с изричен image root, не host-а.
Нестандартен prefix изисква съгласувани пътища; пакетът очаква `/usr/local`.

Преди старт услугата проверява журнала като gridex в sandbox-а: достъп до файловете
и пробен запис/rename в временна подпапка, без фалшива телеметрия. При отказ не
стартира; поправят се права/диск, не се отключва управление и не се пуска като root.
Папките са 0750 gridex:gridex, журналите 0640 gridex:gridex. Друг journal път остава
в защитения env и изисква изрично подготвени права/sandbox, без автоматична поправка.

Новото устройство преминава през:
1. Проверен image hash и изрично одобрение на носителя преди flash. Без клиентски
   env, история, MQTT/OTA ключове, claim тайни, SSH host ключове или копирана machine
   идентичност. Уникални machine/SSH идентичности при първи старт; без клониране на
   вече заведена платка.
2. Първи старт със заключени записи, management DHCP и отделна OT конфигурация само
   през одобрено провизиране. VPN остава на Site Router.
3. Уникален gateway/node и отделни credentials през защитен enrolment. Бъдещ
   еднократен краткоживеещ claim свързва устройството с администратора на Обекта;
   този UI/backend процес остава задача, не е реализиран тук. Без обща factory парола.
4. Администраторът избира Обект, до две роли и одобрява конфигурационна версия.
   ROCK започва изтеглянето, ESP се конфигурира/обновява само през него. Импортът
   или получената задача не доказват успешно прилагане.
5. Активиране само след съвпадащи revision/readback и health доказателства. Отделни
   часове за последно backend съобщение от ROCK и успешна ESP проба/heartbeat.
   Регистрацията не превръща неизвестни/стари данни в online.

Приемане на имидж: fresh boot права/account/sandbox; без тайни/история и с различни
идентичности на два клонинга; upgrade/restore на root-owned журнал и `.1` без
загуба на байтове или променени backup права; ротация/рестарт/пълен диск върху
отделен временен журнал, не живия. Поне три свежи online проби с растящ ESP
heartbeat и без polling грешки; отделна проверка на backend/UI. Съхранението не
доказва ACK. Без BESS/PCS записи, релета, автоматичен VPN или root услуга.

Доказателство от собственика, 2026-09-19: след поправка на root-owned папка/журнал,
проби 9–11 от 19:11:41–19:11:52 UTC са online=true с heartbeat 674→685→695.
Това доказва локалния интервал, не текущото състояние или backend доставката.
Отделният PCS heartbeat остава непотвърден при commissioning заключване.
