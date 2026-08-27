# GrideX Edge hardware baseline

## Статус

Хардуерът още не е финализиран като конкретен производител, модел и BOM. Финализирани са архитектурните изисквания и препоръчителният клас на платформата.

## Препоръчителна платформа за първи прототип

Индустриален Linux контролер на DIN шина с ARM64 или x86-64 процесор. Това позволява C++ service, Modbus TCP/RTU, локални логове, VPN, secure updates и OpenRemote diagnostics без ограниченията на малък MCU.

Минимална базова конфигурация:

| Компонент | Базово изискване |
|---|---|
| CPU | Industrial ARM64 или x86-64 |
| RAM | 4 GB |
| Storage | 32 GB industrial eMMC, с отчетен endurance |
| RS485 | 4 независими, галванично изолирани порта |
| Ethernet | 2 независими GbE интерфейса - OT LAN и EMS/VPN uplink |
| Power | 24 VDC industrial input, reverse-polarity и surge защита |
| Watchdog | Независим hardware watchdog |
| Time | RTC и синхронизация чрез NTP |
| Mounting | DIN rail, метален корпус |
| Environment | Проектна цел -20°C до +60°C |
| Security | Secure boot/TPM когато избраната платформа го поддържа |
| Optional | LTE/4G, GNSS time, isolated digital I/O |

## Портова топология за SunStorage Pro 261

- Ethernet OT LAN: SunStorage/BCQ Modbus TCP, порт 3200.
- Ethernet EMS uplink: OpenRemote/VPN и northbound Modbus TCP, порт 1502.
- RS485-1: външен PCC smart meter.
- RS485-2: PV/SmartLogger или допълнителен електромер, ако не е Ethernet.
- RS485-3: EVSE/управляем товар.
- RS485-4: резервен сегмент или независим локален BMS/IO.

При all-in-one STE-261L PCS, BMS, I/O и охлаждането вече се forward-ват през BCQ на един Modbus TCP endpoint; RS485 портовете остават важни за периферията на обекта и за други BESS конфигурации.

## Какво липсва за окончателен BOM

1. Максимална допустима цена за 1 брой и целеви обем.
2. Нужни ли са LTE/4G, Wi-Fi и GNSS.
3. Изискван IP клас на кутията и монтаж вътре/вън.
4. Сертификационни изисквания за крайния продукт и пазари.
5. Конекторен стандарт, termination/bias политика и целево ниво на RS485 изолация.
6. Нужна ли е кратковременна UPS/supercapacitor автономност.
7. Избор: готов индустриален контролер за пилота или собствена carrier платка.
8. Потвърден I/O списък от еднолинейната схема на първия обект.

## Решение, което препоръчвам

За пилота: готов индустриален Linux контролер с 4 изолирани RS485 и 2 Ethernet. За серия: собствена carrier платка едва след полеви тестове на първите 3-5 обекта. Това намалява риска, без firmware архитектурата да се променя.

