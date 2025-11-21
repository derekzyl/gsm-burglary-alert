# Bill of Materials (BOM) - GSM Burglary Alarm System

## Core Components (Required)

### 1. Microcontroller
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| ESP32 Development Board | ESP32-WROOM-32 (or ESP32-DevKitC) | 1 | Main microcontroller with WiFi/Bluetooth |
| ESP32 Pin Headers | Male headers for dev board | 1 set | If not included with board |

### 2. GSM Module
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| SIM800L GSM Module | SIM800L with antenna | 1 | For SMS/call functionality |
| SIM800L Breakout Board | If module doesn't include | 1 | May be included with module |
| GSM Antenna | 2G/3G antenna (if not included) | 1 | Required for good signal |
| SIM Card | Prepaid SIM with SMS/data plan | 1 | Local operator, one-time cost |
| SIM Card Holder | Standard SIM holder | 1 | May be included with module |

### 3. Sensors
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| PIR Motion Sensor | HC-SR501 | 2-4 | Room intrusion detection |
| Magnetic Reed Switch | Door/Window sensor | 2-6 | For doors and windows |
| Shock/Vibration Sensor | Piezo knock sensor or SW-420 | 1-2 | Tamper/vibration detection |
| Tamper Switch | Magnetic or micro switch | 1 | Enclosure tamper detection |
| Reed Switch Magnet | For magnetic sensors | 2-6 | Matches number of reed switches |

### 4. Actuators & Alerts
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| 12V Siren | 110dB+ siren | 1 | Main alarm siren |
| Relay Module | 5V single-channel relay | 1-2 | To control siren (12V) |
| Passive Buzzer | Piezo buzzer | 1 | Secondary alert (optional) |
| LED Indicator | 5mm LED (red/green) | 2-4 | Status indicators |

### 5. Power Supply
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| 12V Power Adapter | 3A+ output, stable | 1 | Main power supply |
| 12V Lead-Acid Battery | 7Ah SLA battery | 1 | Battery backup |
| Battery Charger | 12V SLA charger | 1 | If not included with battery |
| 5V Buck Converter | LM2596 or similar | 1 | Step down 12V to 5V for ESP32 |
| 3.3V LDO Regulator | AMS1117-3.3 (if needed) | 1 | For 3.3V components |
| Schottky Diode | 1N5822 or similar | 2 | Battery backup diode/UPS circuit |
| Capacitors | 1000µF, 470µF, 100µF | Several | Power supply filtering |
| Fuse Holder & Fuses | 5A fuse holder with fuses | 1 set | Overcurrent protection |

**Power Supply Notes:**
- SIM800L requires peak current of ~2A during transmission
- ESP32 requires ~200-500mA
- Use proper filtering and decoupling capacitors
- Diode OR circuit for battery backup (wall power OR battery)

### 6. Passive Components
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| Resistors | 220Ω, 1kΩ, 10kΩ (1/4W) | Assortment | Pull-ups, current limiting |
| Capacitors | 10µF, 100nF ceramic | Assortment | Decoupling, filtering |
| LEDs | 5mm red, green, yellow | 4-6 | Status indicators |

### 7. Enclosure & Hardware
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| Project Enclosure | Weatherproof box | 1 | Main housing |
| DIN Rail Mount | If needed | 1 | Optional mounting |
| Cable Glands | PG7, PG9 for entry | 2-4 | Weatherproof cable entry |
| Screws & Nuts | M3/M4 assortment | Assortment | Mounting hardware |
| Spacers/Standoffs | Nylon or metal | 4-8 | PCB mounting |

### 8. Cables & Connectors
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| Jumper Wires | Male-male, male-female | Assortment | Breadboard/prototyping |
| Hook-up Wire | 22-24 AWG, various colors | 50-100ft | Permanent wiring |
| Terminal Blocks | 5.08mm pitch screw terminals | 4-8 positions | Sensor/actuator connections |
| Wire Strippers | Basic stripper | 1 | Tool |
| Crimping Tool | If using connectors | 1 | Tool (optional) |

## Optional Components (Wireless Sensors)

### 9. Wireless Communication (Optional)
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| 433MHz/868MHz Transmitter | ASK/OOK transmitter | 1 per sensor | For wireless sensors |
| 433MHz/868MHz Receiver | ASK/OOK receiver | 1 | For main unit |
| 433MHz Antennas | Quarter-wave antenna | 1 per module | Improve range |
| Battery Holder | 2xAA or 3xAA holder | Per wireless sensor | For battery-powered sensors |

**Alternative:** Use ESP-NOW or BLE mesh for wireless sensors if using multiple ESP32s

## Additional Tools & Supplies

### 10. Soldering & Prototyping
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| Breadboard | Half or full size | 1-2 | Prototyping |
| Perfboard/Stripboard | For permanent PCB | 1-2 | Custom PCB alternative |
| Solder | 0.031" rosin core | 1 roll | Soldering |
| Flux | Rosin flux | 1 | Soldering aid |
| Soldering Iron | Temperature controlled | 1 | Tool |
| Desoldering Braid | For fixing mistakes | 1 | Tool |

### 11. Testing & Development
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| USB Cable | Micro-USB or USB-C for ESP32 | 1-2 | Programming/debugging |
| Multimeter | Basic digital multimeter | 1 | Testing tool |
| Logic Analyzer | Optional, for debugging | 1 | Advanced debugging |

## Software & Services

### 12. Software Licenses & Services
| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| SIM Card Plan | Prepaid SMS/data plan | Monthly | Local operator rates |
| Firebase Account | For push notifications | Free tier | First 10k notifications free |
| Backend Hosting | VPS/Cloud (optional) | Monthly | If hosting backend yourself |
| Domain Name | Custom domain (optional) | Annual | Optional |

## Important Notes

1. **Power Supply:** The SIM800L GSM module requires a stable power supply capable of delivering peak currents of ~2A. Use proper filtering capacitors (1000µF + 470µF + 100nF) close to the module.

2. **Battery Backup:** Implement a diode OR circuit to automatically switch between wall power and battery backup.

3. **Antenna Placement:** GSM antenna placement is critical. Mount externally or in a location with good signal for reliable operation.

4. **SIM Card:** Ensure the SIM card is activated and has SMS/data capabilities. Test in a phone first.

5. **Enclosure:** Use a weatherproof enclosure if installing outdoors or in a garage. Ensure proper ventilation to prevent overheating.

6. **Wiring:** Use appropriate wire gauge for power connections. 22-24 AWG for signals, 18-20 AWG for power.

7. **Safety:** Include fuses in the power supply circuit to protect against overcurrent conditions.

8. **Testing:** Test each component individually before final assembly. Use a multimeter to verify connections.

## Recommended Suppliers

- **AliExpress/Banggood:** Longer shipping times
- **DigiKey/Mouser:** Fast shipping, reliable components (US/EU)
- **Amazon:** Fast delivery, good for tools and common components
- **Local Electronics Stores:** Immediate availability

## Order of Assembly

1. **Prototype on breadboard** - Test ESP32, GSM module, and basic sensors
2. **Design power supply** - Build and test power supply circuit
3. **Integrate sensors** - Connect all sensors and test individually
4. **Add actuators** - Connect siren and relay, test control
5. **Final assembly** - Mount in enclosure, final wiring
6. **Software upload** - Program ESP32 with firmware
7. **Testing** - Comprehensive system testing
8. **Deployment** - Install at location and configure

## Future Enhancements (Optional)

- **Camera Module:** ESP32-CAM for visual monitoring
- **Solar Panel:** For battery charging in outdoor installations
- **LCD Display:** Local status display
- **Keypad:** For local arming/disarming
- **Multiple Zones:** Separate sensor groups with individual control
- **Home Automation Integration:** MQTT bridge for smart home systems

---

**Last Updated:** 2024
**Version:** 1.0

