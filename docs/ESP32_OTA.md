# ROCK Pi mediated ESP32 OTA

## English

### Purpose and boundary

This procedure updates an OLIMEX ESP32-EVB node only through the local Edge
path. It does not create a VPN endpoint, public listener, MQTT command route,
or inbound Internet route on the ESP32.

```text
Approved backend operator
  -> Site Router WireGuard policy
  -> ROCK Pi operator session
  -> local OT/bench Ethernet
  -> ESP32-EVB /gridex/ota
```

The Site Router remains the WireGuard endpoint. The ESP32 is never directly
reachable from WireGuard or the public Internet. A router policy may allow an
approved backend peer to reach the ROCK Pi management endpoint; it must never
forward that access to the ESP32 OTA port.

### ESP32 safeguards

The ESP32 OTA endpoint is disabled by default. A local USB serial provisioning
step must set both the ROCK Pi IPv4 address and an OTA secret:

```text
rockpi <ROCK_PI_OT_OR_BENCH_ADDRESS>
ota-key <32-TO-128-CHARACTER-RANDOM-SECRET>
```

The serial console is 115200 bps, 8N1. Wait for the boot message after opening
the USB serial port before issuing a command. The node stores only the SHA-256
verifier of the secret in NVS under `gridex-control`; it never stores or logs
the reusable secret.

The endpoint listens on TCP 8080 only after this provisioning and requires all
of the following:

1. the TCP source address is exactly the provisioned ROCK Pi address;
2. the `X-GrideX-Token` header hashes to the stored verifier;
3. the `X-GrideX-SHA256` header is a valid digest and matches the received
   firmware image;
4. the firmware is no larger than 3 MiB.

The endpoint accepts only a `POST /gridex/ota` multipart upload. A successful
image is written by the ESP32 update library and the node restarts. Invalid
sources, secrets, hashes and incomplete images are rejected. The node remains
an unconfigured, read-only field driver after the pilot update; OTA does not
approve any downstream device command.

### ROCK Pi operator utility

`gridex_ota_apply` is a local, operator-invoked client installed alongside the
ROCK Pi service. It has no listener. The firmware and token must be staged on
the ROCK Pi with the token readable only by its owner (`0600`):

```bash
gridex_ota_apply \
  --host <ESP32_OT_OR_BENCH_ADDRESS> \
  --port 8080 \
  --firmware /secure-staging/firmware.bin \
  --token-file /secure-staging/esp32-ota-token
```

The utility calculates the image SHA-256 locally and sends it with the upload.
It refuses a token file that grants group or other read access. Do not put the
token, device address, or staged firmware into Git. Rotate the secret with a
local serial `ota-key` command after a commissioning or personnel change.

### Pilot evidence and remaining deployment work

The physical pilot completed a verified end-to-end update initiated on ROCK Pi:
the node accepted the image, rebooted, and ROCK Pi's active service continued
in its locked read-only state. Modbus TCP transport reached the ESP32 before
and after the reboot. This evidence verifies the local Ethernet path; it does
not commission a production OTA route.

Before production use, approve the Site Router ACL, define the release-signing
and secret-rotation owner, deploy the utility through the normal ROCK Pi
release process, and complete an isolated OT soak test. No BESS/PCS writes are
part of this procedure.

## Български

### Предназначение и граница

Тази процедура обновява OLIMEX ESP32-EVB нод само през локалния Edge път. Тя
не създава VPN endpoint, публичен listener, MQTT command маршрут или входящ
Интернет маршрут към ESP32.

```text
Одобрен backend оператор
  -> WireGuard policy на Site Router
  -> операторска сесия към ROCK Pi
  -> локален OT/bench Ethernet
  -> ESP32-EVB /gridex/ota
```

Site Router остава WireGuard endpoint. ESP32 никога не е достъпен директно от
WireGuard или публичния Интернет. Router policy може да разреши на одобрен
backend peer да достигне management endpoint-а на ROCK Pi, но никога не трябва
да препраща този достъп към OTA порта на ESP32.

### Защити на ESP32

OTA endpoint-ът на ESP32 е изключен по подразбиране. Локално provisioning през
USB serial трябва да зададе едновременно IPv4 адреса на ROCK Pi и OTA secret:

```text
rockpi <ROCK_PI_OT_OR_BENCH_ADDRESS>
ota-key <32-TO-128-CHARACTER-RANDOM-SECRET>
```

Serial конзолата е 115200 bps, 8N1. След отваряне на USB serial порта изчакай
boot съобщението, преди да подадеш команда. Нодът записва в NVS под
`gridex-control` само SHA-256 verifier на тайната; не записва и не логва
повторно използваемата тайна.

Endpoint-ът слуша на TCP 8080 само след това provisioning и изисква едновременно:

1. TCP source адресът да съвпада точно с provision-натия адрес на ROCK Pi;
2. `X-GrideX-Token` header-ът да се хешира до записания verifier;
3. `X-GrideX-SHA256` header-ът да е валиден digest и да съвпада с получения
   firmware image;
4. firmware-ът да е не по-голям от 3 MiB.

Endpoint-ът приема само `POST /gridex/ota` multipart upload. При успешен образ
ESP32 update библиотеката го записва и нодът се рестартира. Невалидни source,
secret, hash и непълни образи се отказват. След pilot обновяването нодът
остава unconfigured, read-only field driver; OTA не одобрява downstream
команди към устройство.

### Операторска програма на ROCK Pi

`gridex_ota_apply` е локален, операторски client, инсталиран до ROCK Pi
услугата. Той няма listener. Firmware и token се подготвят на ROCK Pi, като
token-ът се чете само от собственика (`0600`):

```bash
gridex_ota_apply \
  --host <ESP32_OT_OR_BENCH_ADDRESS> \
  --port 8080 \
  --firmware /secure-staging/firmware.bin \
  --token-file /secure-staging/esp32-ota-token
```

Програмата пресмята SHA-256 на образа локално и го изпраща с upload-а. Тя
отказва token файл с права за четене от group или other. Не записвай token,
адрес на устройство или staging firmware в Git. Ротирай тайната с локалната
serial команда `ota-key` след commissioning или промяна на персонала.

### Pilot доказателство и оставаща deployment работа

Физическият пилот изпълни потвърдено end-to-end обновяване, стартирано от ROCK
Pi: нодът прие образа, рестартира се, а активната услуга на ROCK Pi продължи в
заключено read-only състояние. Modbus TCP transport достигна ESP32 преди и след
рестарта. Това доказва локалния Ethernet път, но не е commissioning на
production OTA маршрут.

Преди production употреба одобри Site Router ACL, определи собственик на
release signing и secret rotation, внедри програмата чрез нормалния ROCK Pi
release процес и изпълни изолиран OT soak тест. В тази процедура няма
BESS/PCS writes.
