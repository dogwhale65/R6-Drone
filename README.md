# R6 Surveillance Drone

An open-source recreation of the surveillance drone from Rainbow Six Siege.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![R6 Drone Banner](images/banner.png)

## What is this?

I'm building a functional, 3D-printable version of the drone from R6 Siege. It'll have motorized wheels, LEDs, and maybe a camera if I can fit one in there.

Still early days - I've got the motors picked out and a rough CAD design, but there's plenty left to figure out.

## Current Progress

- [x] Motor selection
- [x] Initial CAD design
- [ ] Finalize parts list
- [ ] Electronics/wiring diagram
- [ ] 3D model finalization
- [ ] Assembly guide
- [ ] Software
- [ ] Testing

## Parts List

### Electronics

| Component | Model | Link | Qty |
| ----------- | ------- | ------ | ----- |
| **Battery** | 11.1V 3S LiPo 2200mAh | [Amazon](https://www.amazon.com/dp/B07MQT6YJN) | 1 pack |
| **Motor** | 12V 550RPM | [Amazon](https://www.amazon.com/dp/B072R5G5GR) | 2 Motors |
| **Motor Driver** | L298N H-Bridge | [Amazon](https://www.amazon.com/WWZMDiB-L298N-H-Bridge-Controller-Raspberry/dp/B0CR6BX5QL) | 1 pack|
| **Microcontroller** | ESP32-S3 Dev  Board | [Amazon](https://www.amazon.com/dp/B0C9GLDCRC?th=1) | 1 pack |
| **Buck Converter** | LM2596 DC-DC | [Amazon](https://www.amazon.com/dp/B08NV3JCBC) | 1 pack |
| **XT60 Connectors** | Male/Female pairs | [Amazon](https://www.amazon.com/dp/B0DB1LBGSH) | 1 pack |
| **Power Switch** | Rocker switch 10A, 20mm | [Amazon](https://www.amazon.com/dp/B07QQ22DTB) | 1 pack |
| **Blue LEDs** | 3mm diffused, 12V pre-wired | [Amazon](https://www.amazon.com/dp/B07T8FN8CS) | 1 pack |
| **Resistor Kit** | Assortment | [Amazon](https://www.amazon.com/dp/B08FD1XVL6) | 1 pack |
| **Wire** | 18AWG silicone, multi-color | [Amazon](https://www.amazon.com/dp/B08BR454BX?th=1) | 1 pack |
| **Heat Shrink** | Assorted tubing kit | [Amazon](https://www.amazon.com/dp/B084GDLSCK) | 1 pack |
| **JST Connectors** | JST-XH 2.54mm kit | [Amazon](https://www.amazon.com/dp/B0731MZCGF) | 1 pack |
| **LiPo Charger** | Generic 3S LiPo Battery Charger | [Amazon](https://www.amazon.com/dp/B0B5GRRY59) | 1 pack |
| **Camera** | ESP32-CAM module | [Amazon](https://www.amazon.com/dp/B0948ZFTQZ) | 1 pack |

**Notes:**

- LM2596 steps 11.1V → 3.3V for ESP32


### 3D Printed Parts

STL files will go in `/stl` once they're ready. Or  I'll do 3mf files,  I dont know yet.

| Part | Qty | Notes |
|------|-----|-------|
| Main Body | 1 | PLA or PETG, 20% infill |
| Motor Plates | 2 | 50% infill |

Print at 0.2mm layer height, 50mm/s, with supports where needed.

## Wiring

Basic idea:

```
Battery → Switch → Motor Driver → Motors
                 ↓
         5V Buck Converter → MCU + LEDs
```

Tentative pin assignments (Arduino/ESP32):

- D2, D3: Motor 1 (PWM)
- D4, D5: Motor 2 (PWM)
- D6: LEDs
- A0: Battery voltage (via divider)

## Assembly

Rough order of operations:

1. Print everything.
2. Test fit before committing
3. Install motors to plates.
4. Wire up the electronics.
5. Test motors work.
6. Screw Plates to Body.
7. Close body up
8. Test.

Detailed guide coming once I actually build the thing.

## Software

Will add code once I get there. Planning for:

- Motor control
- LED effects
- Battery monitoring?
- WiFi/app control

## Disclaimer

Fan project, not affiliated with Ubisoft. Just for fun.

## License

MIT - see [LICENSE](LICENSE).

---

Project: [github.com/dogwhale65/r6-drone](https://github.com/dogwhale65/r6-drone)
