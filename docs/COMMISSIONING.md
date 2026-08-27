# Commissioning checklist - SunStorage Pro 261

Командният път остава заключен, докато всяка точка не бъде подписана от отговорния инженер.

- [ ] BCQ endpoint е достъпен на TCP port 3200, unit ID 1.
- [ ] Потвърдено е дали документните адреси се подават като 0-based PDU или с +1 offset.
- [ ] Прочетените SOC, SOH, напрежение и ток съвпадат с локалния HMI.
- [ ] Регистри 127/128 дават динамични BMS лимити в kW с divisor 10.
- [ ] Потвърден е sign convention с команда не по-голяма от одобрената commissioning мощност.
- [ ] Setpoint 5005 е потвърден като INT16, kW x10.
- [ ] Heartbeat 5302 е включен и 5301 се обновява локално през 35 s.
- [ ] При спиране на heartbeat PCS влиза в standby според протокола.
- [ ] При загуба на OpenRemote/EMS Edge прилага 0 kW без да променя други настройки.
- [ ] Software fuse е проверен с реалния PCC meter.
- [ ] Charge/discharge prohibit bits в BMS alarm register 126 се спазват.
- [ ] Всички write команди и причини за clamp се записват в audit log.

