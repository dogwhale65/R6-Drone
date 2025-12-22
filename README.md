# R6 Surveillance Drone
>
> An open-source recreation of the iconic surveillance drone from Rainbow Six Siege

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Status](https://img.shields.io/badge/Status-In%20Development-orange)](https://github.com/yourusername/r6-drone)

![R6 Drone Banner](images/banner.png)
*Work in progress - Image placeholder*

## 📋 Table of Contents

- [Overview](#overview)
- [Project Status](#project-status)
- [Features](#features)
- [Bill of Materials](#bill-of-materials)
- [Electronics & Wiring](#electronics--wiring)
- [3D Printing](#3d-printing)
- [Assembly Instructions](#assembly-instructions)
- [Software](#software)
- [Contributing](#contributing)
- [License](#license)

## 🎯 Overview

This project aims to create a functional, 3D-printable replica of the surveillance drone from Rainbow Six Siege. The drone features motorized wheels, LED lighting, and optional camera capabilities to bring the in-game gadget to life.

**Inspiration:** Rainbow Six Siege tactical surveillance drone

## 🚧 Project Status

**Current Version:** v0.1-alpha

- [x] Motor selection
- [x] Initial CAD design/outline
- [ ] Complete parts list
- [ ] Electronics design & wiring diagram
- [ ] 3D model finalization
- [ ] Assembly instructions
- [ ] Software/controller integration
- [ ] Testing & iteration

## ✨ Features

### Planned Features

- ✅ Motorized wheel system
- 🔲 LED lighting (authentic R6 styling)
- 🔲 Optional camera mount
- 🔲 RC control via smartphone/controller
- 🔲 Rechargeable battery system
- 🔲 Sound effects (optional)
- 🔲 Compact, display-worthy design

## 🛠️ Bill of Materials

### Motors & Actuators

| Component | Specification | Quantity | Link | Notes |
|-----------|--------------|----------|------|-------|
| DC Geared Motor | 12V, 550 RPM, 37mm gearbox | 2 | [Amazon](https://www.amazon.com/Greartisan-Electric-Reduction-Centric-Diameter/dp/B072R5G5GR) | Main drive motors |

**Motor Specifications:**

- Voltage: 12V DC
- Speed: 550 RPM
- Torque: 0.7 kg·cm
- Current: 0.5A rated
- Gearbox: 37mm diameter
- Output Shaft: 6×14mm D-shaped
- Reduction Ratio: 1:9

### Electronics (In Progress)

#### Power System

| Component | Specification | Quantity | Link | Status |
|-----------|--------------|----------|------|--------|
| Battery | 11.1V 3S LiPo (recommended) | 1 | TBD | 🔲 Not selected |
| Battery Connector | XT60 or similar | 1 | TBD | 🔲 Not selected |
| Power Switch | SPST, 3A+ rating | 1 | TBD | 🔲 Not selected |

#### Motor Control

| Component | Specification | Quantity | Link | Status |
|-----------|--------------|----------|------|--------|
| Motor Driver | Dual H-Bridge (L298N or similar) | 1 | TBD | 🔲 Not selected |
| Microcontroller | Arduino Nano/ESP32 | 1 | TBD | 🔲 Not selected |

#### Optional Components

| Component | Specification | Quantity | Link | Status |
|-----------|--------------|----------|------|--------|
| LEDs | Blue 5mm (for lighting) | 4-6 | TBD | 🔲 Not selected |
| LED Resistors | 220Ω | 4-6 | TBD | 🔲 Not selected |
| Camera Module | ESP32-CAM or Raspberry Pi Camera | 1 | TBD | 🔲 Optional |
| Speaker | Small 8Ω speaker | 1 | TBD | 🔲 Optional |

#### Miscellaneous

| Component | Specification | Quantity | Link | Status |
|-----------|--------------|----------|------|--------|
| Wheels/Treads | TBD (custom or commercial) | 2 | TBD | 🔲 Not selected |
| Wiring | 22-24 AWG stranded wire | 1 set | TBD | 🔲 Not selected |
| Heat Shrink Tubing | Assorted sizes | 1 set | TBD | 🔲 Not selected |
| M3 Screws | Various lengths | ~20 | TBD | 🔲 Not selected |

### Electronics Recommendations

Based on your motor selection (12V, 0.5A each = 1A total), here are my recommendations:

**Power System:**

- **Battery:** 11.1V 3S LiPo (1000-1500mAh) - provides good runtime while keeping size compact
- **Alternative:** 3S 18650 battery pack (12.6V fully charged)
- **Voltage Regulator:** 5V buck converter for microcontroller/LEDs

**Motor Driver:**

- **Option 1:** L298N Dual H-Bridge (cheap, 2A per channel, overkill for your needs but robust)
- **Option 2:** DRV8833 Dual H-Bridge (smaller footprint, 1.5A per channel, more efficient)
- **Option 3:** TB6612FNG (compact, efficient, 1.2A per channel)

**Microcontroller:**

- **Basic Version:** Arduino Nano (simple, well-documented)
- **WiFi Version:** ESP32 (enables smartphone control, camera streaming)
- **Advanced:** Raspberry Pi Zero W (full Linux, better camera support)

**Additional Circuit Protection:**

- Fuse or PTC resettable fuse (2A rating)
- Capacitors (100µF across motor terminals to reduce electrical noise)

### 3D Printed Parts

*Coming soon - STL files will be available in the `/stl` folder*

| Part Name | Quantity | Material | Infill | Notes |
|-----------|----------|----------|--------|-------|
| Main Body | 1 | PLA/PETG | 20% | TBD |
| Bottom Chassis | 1 | PLA/PETG | 30% | TBD |
| Wheel Mount | 2 | PLA/PETG | 30% | TBD |
| Camera Housing | 1 | PLA/PETG | 15% | TBD (optional) |
| LED Diffusers | 4 | Clear PLA/PETG | 15% | TBD |

**Recommended Print Settings:**

- Layer Height: 0.2mm
- Print Speed: 50mm/s
- Supports: Yes (where needed)
- Bed Adhesion: Brim recommended

## 🔌 Electronics & Wiring

### Basic Wiring Diagram

```
[Battery] ─┬─ [Switch] ─┬─ [Motor Driver] ─┬─ [Motor 1]
           │             │                   └─ [Motor 2]
           │             │
           └─ [5V Buck Converter] ─┬─ [Microcontroller]
                                    └─ [LEDs + Resistors]
```

*Detailed wiring diagrams coming soon*

### Pin Assignments (Tentative)

```
Arduino/ESP32 Pins:
- D2, D3: Motor 1 control (PWM)
- D4, D5: Motor 2 control (PWM)
- D6: LED control (PWM for dimming)
- A0: Battery voltage monitoring (voltage divider)
```

## 🖨️ 3D Printing

### STL Files

*Files will be uploaded to `/stl` folder once finalized*

**Printing Tips:**

1. Print chassis parts in PETG for better durability
2. Use tree supports for complex geometries
3. Test fit all parts before final assembly
4. Consider printing wheels in TPU for better traction

### Assembly Order (Draft)

1. Print all parts
2. Install motor mounts in chassis
3. Mount motors with M3 screws
4. Install motor driver and microcontroller
5. Wire electronics according to diagram
6. Test motor movement
7. Install wheels
8. Add LED lighting
9. Close chassis and final assembly

*Detailed assembly guide with photos coming soon*

## 💻 Software

*Code repository coming soon*

### Planned Features

- Basic motor control
- Speed adjustment
- LED effects
- Battery monitoring
- Optional: WiFi control via smartphone app
- Optional: Camera streaming

### Development Environment

- Arduino IDE or PlatformIO
- Libraries: TBD

## 🤝 Contributing

This project is in early development and contributions are welcome!

**How to contribute:**

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

**Areas where help is needed:**

- Electronics design optimization
- Code development
- Testing and iteration
- Documentation
- 3D model refinement

## ⚠️ Disclaimer

This is a fan project and is not affiliated with or endorsed by Ubisoft or Rainbow Six Siege. This project is for educational and entertainment purposes only.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Rainbow Six Siege by Ubisoft for the inspiration
- The maker community for tools and knowledge
- Contributors and testers (you!)

## 📞 Contact

Project Link: [https://github.com/dogwhale65/r6-drone](https://github.com/dogwhale65/r6-drone)

---

**Star ⭐ this repository if you find it interesting!**

*Last updated: December 2024*
