# ROCK Pi isolated OT DHCP service

## English

The ROCK Pi has two Ethernet roles. Management/WAN reaches the Site Router and
is the only interface with a default route. OT reaches ESP32 nodes and BESS
equipment; it has a static address, no default route, and runs DHCP only on
that physical interface. The network installer creates a narrowly matched
native `systemd-networkd` override for the OT interface, so it takes precedence
over a vendor wildcard DHCP rule without modifying management configuration.

gridex-ot-dhcp.service renders dnsmasq configuration at start from the
root-owned deployment file /etc/gridex/gridex-rockpie.env. Required keys are
GRIDEX_MANAGEMENT_INTERFACE, GRIDEX_OT_INTERFACE, GRIDEX_OT_ADDRESS,
GRIDEX_OT_PREFIX, GRIDEX_OT_DHCP_RANGE_START, GRIDEX_OT_DHCP_RANGE_END and
GRIDEX_OT_DHCP_LEASE_SECONDS. Optional GRIDEX_OT_DHCP_RESERVATIONS uses
MAC,IP,HOSTNAME entries separated by semicolons.

The service advertises the ROCK Pi OT address as gateway but enables neither
forwarding nor NAT. It therefore allows node-to-ROCK Pi traffic without an
OT-to-Internet path. Real site addresses, MAC addresses and leases stay only
in the protected deployment file.

Run the network installer first and then the DHCP installer as administrator
on the physical ROCK Pi. Verify that management retains the sole default route,
the ROCK Pi OT interface has no default route, and DHCP listens only on OT.
Restart the read-only Edge service and confirm the node slot becomes online.

## Български

ROCK Pi има две Ethernet роли. Management/WAN достига Site Router и е
единственият интерфейс с default route. OT достига ESP32 нодове и BESS
оборудване; има статичен адрес, няма default route и раздава DHCP само на този
физически интерфейс. Network installer-ът създава тясно ограничен native
`systemd-networkd` override за OT интерфейса, който има приоритет пред vendor
wildcard DHCP правило, без да променя management конфигурацията.

gridex-ot-dhcp.service генерира dnsmasq конфигурация при старт от root-owned
deployment файла /etc/gridex/gridex-rockpie.env. Задължителните ключове са
GRIDEX_MANAGEMENT_INTERFACE, GRIDEX_OT_INTERFACE, GRIDEX_OT_ADDRESS,
GRIDEX_OT_PREFIX, GRIDEX_OT_DHCP_RANGE_START, GRIDEX_OT_DHCP_RANGE_END и
GRIDEX_OT_DHCP_LEASE_SECONDS. Незадължителният GRIDEX_OT_DHCP_RESERVATIONS
ползва MAC,IP,HOSTNAME записи, разделени с точка и запетая.

Услугата обявява OT адреса на ROCK Pi за gateway, но не включва forwarding или
NAT. Така разрешава трафик нод-към-ROCK Pi, без да създава OT-към-Интернет
път. Реалните site адреси, MAC адреси и lease-ове остават само в защитения
deployment файл.

Изпълни първо network installer-а, а след това DHCP installer-а като
администратор на физическия ROCK Pi. Провери дали management запазва
единствения default route, OT интерфейсът на ROCK Pi няма default route и DHCP
слуша само на OT. Рестартирай read-only Edge услугата и потвърди, че node slot
става online.
