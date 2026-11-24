# Electrical and operational safety

This repository is a learning/demo project. Its LED GPIO outputs are intended for low-voltage indicator LEDs with suitable current-limiting resistors or an appropriate driver.

**PZEM-004T measures mains AC.** Mains voltage can cause severe injury, fire, or death. Do not handle exposed mains conductors, improvise test leads, or work on an energized circuit. Have the meter installation, enclosure, fusing/protection, isolation, earthing, and verification completed by a qualified electrician/technician using the manufacturer's documentation and applicable local standards.

Do not connect an ESP32 GPIO directly to an AC load, motor, pump, or mains circuit. A GPIO is a low-voltage logic output and is not a power switch. Any future load-control design needs correctly rated, isolated, enclosed hardware and professional review.

The firmware and dashboard are not certified protective equipment and must not be used as an overcurrent, overvoltage, or life-safety protection system.
