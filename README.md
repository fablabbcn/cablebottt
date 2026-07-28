# CABLEBOTTT

**A hands-on STEAM workshop by FLU @ Fab Lab Barcelona.**

Design, fabricate and program a **cable-driven robot** (cablebot). Participants build a small robot that travels along a cable using a rotating motor, and bring it to life by wiring up inputs (light, distance) and outputs (motor, LED, buzzer, servo). The activity combines **mechanics + digital fabrication + computational thinking** in an accessible, creative format.

> Based on a yellow gearmotor driven from a **Seeed Studio XIAO ESP32-C3**, on a custom PCB.

---

## What you'll build

The cablebot comes in **two behaviors**, using the same hardware:

| | **Sketch A — autonomous** | **Sketch B — teleoperated** |
|---|---|---|
| **Idea** | The robot decides for itself | You drive it from your phone |
| **How** | Follows light using 2 LDR sensors | Serves a webapp over its own WiFi |
| **Inputs** | 2 light sensors (LDR) | Distance sensor (optional) |
| **Outputs** | Motor | Motor, color LED, buzzer, servo |
| **Libraries** | None | Adafruit NeoPixel + VL53L1X |
| **Best for** | Seeing a closed feedback loop | Programming your own behaviors |

Both are fully documented — see **[Programming](#3-programming)** below.

---

## Folder structure

```
cablebottt/
├── README.md                        ← you are here (project index)
│
├── fabrication/                     ← everything you cut / print / mill
│   ├── ⟨laser_pieces.svg⟩           ← laser-cut structure (the 3 assembly pieces)
│   ├── ⟨mount_3d.stl⟩               ← 3D-printed parts (if any)
│   ├── pcb/                         ← custom PCB (KiCad project + Gerbers)
│   │   ├── fab26_cablebot.kicad_pro
│   │   ├── fab26_cablebot.kicad_sch
│   │   ├── fab26_cablebot.kicad_pcb
│   │   ├── gerber/                  ← ready-to-mill / ready-to-order
│   │   └── svg/fab26_cablebot-preview.svg
│   └── silkscreen/back_silkscreen.svg
│
├── code/
│   ├── cablebottt_laser/cablebottt.ino        ← Sketch A (autonomous)
│   └── cablebottt_web/cablebottt_web.ino      ← Sketch B (teleoperated)
│
├── docs/
│   ├── cablebottt_sketch_A_tutorial.md        ← how to program Sketch A
│   └── cablebottt_sketch_B_tutorial.md        ← how to program Sketch B
│
└── images/                          ← photos & reference
    ├── ⟨assembled_bot.jpg⟩
    ├── ⟨soldering_xiao.jpg⟩
    └── ⟨pcb_render.png⟩
```

> **⟨…⟩ = placeholder.** Replace with your real filenames. If the layout above doesn't match how you organized the folder, adjust the paths — the section links below still work as long as the two `docs/…` tutorials keep their names.

---

## How to use this project

Do it in three stages. Each stage has its own detailed doc; this index just tells you the order.

### 1. Fabricate

| Part | File | Process |
|---|---|---|
| Structure | `fabrication/⟨laser_pieces.svg⟩` | Laser-cut, then assemble the 3 pieces |
| Custom PCB | `fabrication/pcb/gerber/` | Mill or order from the Gerbers |
| Board | Seeed Studio **XIAO ESP32-C3** | Solder the header pins onto the PCB |

Populate the PCB, solder the XIAO, attach the motor and the laser pointer, and assemble the laser-cut structure. Photos in **`images/`**.

### 2. Wire it up

The XIAO pinout, shared across both sketches:

| Function | Pin | Used by |
|---|---|---|
| Motor driver 1 / 2 | `D9` / `D10` | both |
| LDR left / right | `A1` / `A0` | Sketch A |
| NeoPixel (LED) | `D7` | Sketch B |
| Buzzer | `D6` | Sketch B |
| Servo | `D8` | Sketch B |
| Distance sensor SDA / SCL | `D4` / `D5` | Sketch B |

### 3. Programming

Pick the behavior and follow its step-by-step tutorial:

- **Autonomous (follows light)** → **[`docs/cablebottt_sketch_A_tutorial.md`](docs/cablebottt_sketch_A_tutorial.md)**
  *No libraries needed. Upload and go.*
- **Teleoperated (phone webapp)** → **[`docs/cablebottt_sketch_B_tutorial.md`](docs/cablebottt_sketch_B_tutorial.md)**
  *Install 2 libraries, then connect your phone to the `CABLEBOTTT` WiFi network.*

Both tutorials cover the one-time Arduino IDE + ESP32-C3 setup, so you only need to read it once.

---

## Quick start (already fabricated & flashed?)

1. Power the board over USB-C or the 9V battery.
2. **Sketch A:** shine a light near an LDR — the robot follows it.
3. **Sketch B:** on your phone, join WiFi **`CABLEBOTTT`** (password `cablebottt`), open **`http://192.168.4.1`**, and drive it.


---


*Originally created for a **FAB26 Boston** workshop. Repo: `github.com/fablabbcn/cablebottt`.*
