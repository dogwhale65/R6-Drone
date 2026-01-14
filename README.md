# R6 Surveillance Drone

An open-source recreation of the surveillance drone from Rainbow Six Siege.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub stars](https://img.shields.io/github/stars/dogwhale65/r6-drone)](https://github.com/dogwhale65/r6-drone/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/dogwhale65/r6-drone)](https://github.com/dogwhale65/r6-drone/issues)
![R6 Drone Banner](images/banner.png)

## About

I'm building a functional, 3D-printable version of the drone from R6 Siege. It features motorized wheels, LEDs, and a camera for FPV control.

Still early days — I've got the motors picked out and a rough CAD design, but there's plenty left to figure out.

The drone was designed to fit on a BambuLab P1S, so it may not be perfectly accurate to the in-game model. If anyone has the correct dimensions and wants to share them, I'm all ears!

I'm also *not* an engineer, so apologies if things don't work out perfectly. This is my first real project, and I'm learning as I go.

## Progress

- [x] Motor selection
- [x] Initial CAD design
- [x] Finalize parts list
- [x] Electronics/wiring diagram
- [X] 3D model finalization
- [X] Assembly guide
- [X] Software
- [ ] Testing

## Future Plans

- Dedicated controller
- Camera integration
- Battery level indicator

## Parts List

### Electronics

| Component | Model | Link | Qty |
| --------- | ----- | ---- | --- |
| Battery | 11.1V 3S LiPo 2200mAh | [Amazon](https://www.amazon.com/dp/B07MQT6YJN) | 1 |
| Motors | 12V 550RPM | [Amazon](https://www.amazon.com/dp/B072R5G5GR) | 2 |
| Motor Driver | L298N H-Bridge | [Amazon](https://www.amazon.com/WWZMDiB-L298N-H-Bridge-Controller-Raspberry/dp/B0CR6BX5QL) | 1 |
| MCU | ESP32-S3 Dev Board | [Amazon](https://www.amazon.com/dp/B0C9GLDCRC?th=1) | 1 |
| Buck Converter | LM2596 DC-DC | [Amazon](https://www.amazon.com/dp/B08NV3JCBC) | 1 |
| XT60 Connectors | Male/Female pairs | [Amazon](https://www.amazon.com/dp/B0DB1LBGSH) | 1 pack |
| Power Switch | Rocker switch 10A, 20mm | [Amazon](https://www.amazon.com/dp/B07QQ22DTB) | 1 |
| Blue LEDs | 3mm diffused, 12V pre-wired | [Amazon](https://www.amazon.com/dp/B07T8FN8CS) | 1 pack |
| Wire | 18AWG silicone, multi-color | [Amazon](https://www.amazon.com/dp/B08BR454BX?th=1) | 1 pack |
| Heat Shrink | Assorted tubing kit | [Amazon](https://www.amazon.com/dp/B084GDLSCK) | 1 pack |
| LiPo Charger | 3S Charger | [Amazon](https://www.amazon.com/dp/B0B5GRRY59) | 1 |
| Camera | Generic Camera Module | [Amazon](https://www.amazon.com/dp/B012UXNDOY) | 1 |
| Wire Connectors | Lever nuts | [Amazon](https://www.amazon.com/dp/B0CJ5QF3VX) | 1 pack |

> **Heads up:** The motors I picked have a design flaw. Some mount holes go straight into the gearbox, so screws can hit the internals and prevent the motor from spinning. 

### 3D Printed Parts

Individual 3MF files are in `/3mf`. There's also a ready-to-print 3MF in the root folder with all parts pre-positioned and print settings configured.

> **Note:** These parts haven't been fully tested yet. They work for the most part, updates will be made as I test/improve them.

| Part | Qty | Notes |
| ---- | --- | ----- |
| Main Body | 1 | PLA or PETG, 20% infill, 3 walls, supports recommended |
| Battery Sled | 1 | PLA or PETG, 50% infill, 10 walls, support recommended |
| Motor Plates | 2 | PLA or PETG 20% infill, 5 walls |
| Wheels | 2 | TPU or PETG, 20% infill, 2 walls |

> It takes about 9 hours to print the main body and uses almost 400g of filament. I'm trying to incorporate as many features as possible into a single print to minimize this.

>The wheels are still being tested. TPU and PETG slip a lot. I'm currently working on a new design for the wheels, but cant print them at the current moment. I found out that if you just put some rubber bands around the wheels, it works just fine, but they fall off if you take a turn too hard. New wheel design will be posted soon, which has grooves for the rubber bands to stay in place better.

### Wheel Design Iterations

1. Just a simple cutout for the shaft. It stripped the wheels and obviously fell off.
2. Same principle but I added a screw hole to screw in and clamp onto the D-shaft. Worked good at preventing stripping, but still fell off.
3. A clamp system that used 2 screws to clamp around the shaft. Worked just about the same as the original wheel. The screws stripped the plastic before any real torque could be applied to it to actually clamp.
4. Current design, a separate hub that screws both onto/into the D-shaft and the wheel respectively. Haven't printed it yet, but I think it will work.

Print at 0.2mm layer height with supports where needed.

## Wiring

![Wiring Diagram](images/R6-Drone-Wiring-Colors-Fixed.png)

**Notes:**

- This diagram doesn't include switches or extras.
- Logic wires aren't shown yet — I'll add them in a future revision.
- In my first build, I messed up my ESP32 and broke the 5V connection on my ESP32. I worked around it by adding a second buck converter and powering via the 3.3V pin instead.

Logic wires:

- ENA → GPIO 5
- IN1 → GPIO 6
- IN2 → GPIO 7
- ENB → GPIO 8
- IN3 → GPIO 9
- IN4 → GPIO 10

## Assembly

> **Note:** I used M3 x 20mm screws because I had them lying around. Shorter screws work for the motor plates, but I'd recommend at least 20mm for the main body.

1. Attach the motor plates to the motors, ensuring the motors can still spin freely.
2. Mount the motor plates to the bottom half of the main body.
3. Install the electronics inside the main body.
4. Attach the top half of the main body to the motor plates.
5. Secure the two body halves together.

## Code

The code is in `/code`. It runs a simple web server for controlling the drone. Connect to the drone's WiFi, open the web interface, and drive using on-screen joystick controls or WASD keys on a keyboard. Connect to it by connecting to the drone's WiFi network and visiting `http://192.168.4.1` in your browser. The default password is `password123` on the network `Robot-ESP32`

I am working on making an actual controller for it, rather than using my phone or a computer. Currently, there's a Bluetooth version for mobile app control.



## License

MIT — see [LICENSE](LICENSE).

---

[github.com/dogwhale65/r6-drone](https://github.com/dogwhale65/r6-drone)
