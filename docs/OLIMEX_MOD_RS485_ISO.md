# OLIMEX MOD-RS485-ISO integration / Интеграция с OLIMEX MOD-RS485-ISO

## English

The selected isolated RS-485/RS-422 module for applicable ESP32-EVB device
nodes is **OLIMEX MOD-RS485-ISO**. It provides galvanic isolation through the
TI ISO35T and supports half-duplex RS-485 or full-duplex RS-422/RS-485 through
board jumpers. It is suitable for industrial meters, inverters and BMS devices
only after the downstream vendor protocol has been confirmed.

This is **not** a passive UART-to-RS-485 carrier. A PIC16 with Olimex firmware
is part of the module; the ESP32 host controls it over the UEXT RS-232 or I²C
host interface. Therefore the existing direct-transceiver `Serial1` plus DE/RE
GPIO path must not be selected for this module.

The planned ESP32 adapter must:

1. use the Olimex firmware version installed on the physical module;
2. use the corresponding Olimex Arduino library/protocol under its licence;
3. configure UEXT host pins through one board profile, never hard-coded inside
   a vendor driver;
4. expose the same `IDownstreamModbusClient` interface used by each driver;
5. configure baud, parity, stop bits, half/full duplex and timeout per device;
6. prove read-only communication on a bench before any write is unlocked.

The module manual warns that firmware revisions use different I²C addresses and
different protocols. Commissioning must record the actual PIC revision and
firmware before compiling/deploying the adapter. The module's published
hardware capability includes 3 kV galvanic isolation and signalling up to
1 Mbps. [Olimex product page](https://www.olimex.com/Products/Modules/Interface/MOD-RS485-ISO/) and [user manual](https://www.olimex.com/Products/Modules/Interface/MOD-RS485-ISO/resources/MOD-RS485-ISO-UM.pdf).

## Български

Избраният изолиран RS-485/RS-422 модул за приложимите ESP32-EVB нодове е
**OLIMEX MOD-RS485-ISO**. Той осигурява галванична изолация чрез TI ISO35T и
поддържа half-duplex RS-485 или full-duplex RS-422/RS-485 чрез jumper-и. Може
да се използва за индустриални електромери, инвертори и BMS устройства само
след потвърждение на downstream протокола от производителя.

Това **не е** пасивен UART↔RS-485 carrier. Модулът има PIC16 с Olimex firmware;
ESP32 го управлява през UEXT host интерфейса по RS-232 или I²C. Поради това
сегашният generic път `Serial1` с DE/RE GPIO не трябва да се избира за този
модул.

Планираният ESP32 adapter трябва да използва точната firmware версия и
съответната Olimex библиотека/протокол, да конфигурира UEXT през един board
profile, да предоставя `IDownstreamModbusClient`, и да задава baud/parity/stop
bits/duplex/timeout според конкретното устройство. Първо се валидира read-only
комуникация на стенд; writes се отключват едва след това.

Olimex изрично посочва, че различните firmware ревизии използват различен I²C
address и различен протокол. При commissioning задължително се записват PIC
ревизията и firmware версията, преди adapter-ът да се компилира и инсталира.
