## Column Switcher

#### Original sources: 
- https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/
- https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino

---
#### Important notices: 
- Relays I use are controlled by LOW state. If yours are by HIGH, change the defined `CONTROL_STATE` value in line 26 in `main.cpp`!

- **DO NOT** use Solid-State Relays (SSRs) since they are not the wisest choice
to be applied in audio signal passthroughs. Period.

---

#### TODOs:
- Try-catches for file openings and readings.
- Performance improvements in generating the SSIDs JSON in `generateSsidDropdownJsonArray()`.

- General Clean Code refactoring (restructure).

---

#### **Feel free to contribute!**

---
 Rafal Jurek (jurek.rafal@outlook.com) @ 29/11/2024
