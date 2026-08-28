# GrideX Edge hardware baseline - Rev A

## Статус

Базовата платформа за Rev A е **Radxa ROCK Pi E с RK3328**. Разширителните нодове са **LilyGo T-CAN485 (ESP32)**. Конкретният BOM на захранването, изолацията и DIN carrier платката остава за схемния проект.

## Базов модул - ROCK Pi E

ROCK Pi E предоставя RK3328 quad-core Cortex-A53, 1/2 GB DDR4, два физически Ethernet интерфейса и 40-pin GPIO с UART. Linux услугата се компилира отделно от ESP32 firmware-а.

Минимална базова конфигурация:

| Компонент | Базово изискване |
|---|---|
| CPU | Rockchip RK3328, 4x Cortex-A53 ARM64 до 1.3 GHz |
| RAM | 2 GB DDR4 предпочитано |
| Storage | 16/32 GB eMMC предпочитано; industrial microSD само за прототип |
| RS485 | 1 галванично изолиран гръбнак, 115200 8N1 |
| Ethernet | 1x GbE за OT/STE-261L и 1x 100 MbE за WAN/EMS/IBEX |
| Power | 24 VDC industrial input, reverse-polarity и surge защита |
| Watchdog | Независим hardware watchdog |
| Time | RTC и синхронизация чрез NTP |
| Mounting | 6M DIN, PETG/ASA за прототип; индустриален корпус за серия |
| Environment | Проектна цел -20°C до +60°C |
| Security | Secure boot/TPM когато избраната платформа го поддържа |
| Optional | LTE/4G, GNSS time, isolated digital I/O |

## Мрежова и портова топология

- GbE/OT: директна изолирана мрежа или OT switch към STE-261L/BCQ, Modbus TCP порт 3200, unit ID 1. Няма default route.
- 100 MbE/WAN: site router, OpenRemote, IBEX API, VPN, NTP и обновявания. Default route е само тук.
- UART към изолиран RS485 трансивър: MBUS нодове, 115200 8N1, 4-жилен екраниран кабел A/B/+12V/GND.
- Терминиране 120 ohm само в двата физически края. Bias/fail-safe се реализира в базовия модул.

При STE-261L PCS, BMS, I/O и охлаждането се forward-ват от BCQ на един Modbus TCP endpoint. RS485 гръбнакът е само за T-CAN485 нодовете - електромер, зарядна станция и бъдещ втори кабинет.

Нод, който управлява RS485 инвертор/електромер/EVSE, има **втори отделен изолиран RS485 канал** към устройството. Onboard MAX13487 остава само за upstream MBUS. Допълнителният downstream канал е част от конкретната node carrier платка и се конфигурира според точния driver profile.

## UART към RS485

ROCK Pi E не се свързва директно към шината. Между UART и A/B задължително има индустриален галванично изолиран RS485 трансивър с surge/ESD защита. За хардуер V1.2 UART1 е наличен на physical pin 3 TX / pin 5 RX; UART2 е на pin 8 TX / pin 10 RX. Конкретният `/dev/ttyS*` се потвърждава върху избрания Linux image и се задава с конфигурация, а не се кодира твърдо.

## Какво липсва за окончателен BOM

1. Точна хардуерна ревизия на ROCK Pi E и WiFi вариант.
2. Изолиран RS485 трансивър, DC/DC изолация и ниво на surge/ESD защита.
3. 24 V input stage и отделен защитен 12 V изход за MBUS нодовете.
4. Изискван IP клас и монтаж вътре/вън.
5. Сертификационни изисквания и температурен диапазон.
6. Кратковременна UPS/supercapacitor автономност.
7. Точен конектор и максимален ток/дължина на 12 V гръбнака.
8. Потвърден I/O списък от първия обект.

## Разделяне на кода

- `base-rockpie/` - Linux/ARM64 service, Modbus TCP към STE-261L, local heartbeat/safe-state и MBUS master.
- `node-tcan485/` - ESP32 firmware, Modbus RTU server, auto-addressing и driver layer.
