# CABLEBOTTT — Sketch A: `cablebottt_laser`

**Autonomous light-following robot.** The cablebot reads two light sensors (LDRs) and drives its motor toward the brighter side. This is the **sensor → decision → actuator** loop in its purest form.

Board: **Seeed Studio XIAO ESP32-C3** · File: **`cablebottt_laser/cablebottt.ino`** · Repo: `github.com/fablabbcn/cablebottt`

---

## 📚 Libraries needed: NONE

This sketch uses **no external libraries**. It has zero `#include` lines and relies only on the standard Arduino core for ESP32 (`Serial`, `analogRead`, `analogWrite`, `digitalWrite`, `pinMode`, `millis`, `delay`).

That means: **as soon as the ESP32 board package is installed (Part 1 below), you can upload straight away** — nothing to add in Library Manager. Save the library installs for Sketch B (the webapp one).

---

## Part 1 — One-time setup (once per computer)

### 1.1 · Install the Arduino IDE
Download the stable **Arduino IDE 2.x** from `arduino.cc/en/software` and install it.

### 1.2 · Add ESP32 board support
1. Go to **File › Preferences** (on Mac: **Arduino IDE › Settings**).
2. In **"Additional boards manager URLs"**, paste:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json
   ```
3. **OK**.

### 1.3 · Install the boards package
1. **Tools › Board › Boards Manager…** (or the boards icon on the left).
2. Search **`esp32`**.
3. Install **"esp32 by Espressif Systems"** (latest version). Takes a couple of minutes.

### 1.4 · Select board and port
1. Connect the XIAO with a **USB-C data cable** (many cables are charge-only; if the board doesn't show up, swap the cable first).
2. **Tools › Board › esp32 › `XIAO_ESP32C3`**.
3. **Tools › Port** → pick the port that appears when you plug the board in (COMx on Windows, `/dev/cu.usbmodem…` on Mac, `/dev/ttyACM0` on Linux).

### 1.5 · ⚙️ CRITICAL setting for the Serial Monitor
Under **Tools**, set:
- **USB CDC On Boot → `Enabled`**

> Without this, the ESP32-C3's Serial Monitor stays **blank** over USB-C. It's the #1 gotcha. Turn it on before you start. Leave the other **Tools** options at their defaults.

---

## Part 2 — Upload Sketch A

### 2.1 · Open the code
Open **`cablebottt_laser/cablebottt.ino`** (double-click the `.ino`, or **File › Open…**).

### 2.2 · Check board and port
Make sure **`XIAO_ESP32C3`** and the correct port are still selected (Part 1.4).

### 2.3 · Upload
Press the **Upload** button (→ arrow). You'll see `Compiling…` then `Writing…`.
If it ends with **`Hard resetting via RTS pin…`** → uploaded successfully ✅.

### 2.4 · Verify it works
1. Open **Tools › Serial Monitor** and set the speed to **`115200 baud`** (bottom-right).
2. On boot, the robot reads a **baseline** light level for ~1 second. You'll see the two LDR readings and a `baseline` line.
3. Shine your phone's flashlight on one LDR: the motor should pull toward the **brighter** side.

---

## How it works (to explain to students)

| Step | What happens |
|---|---|
| **Baseline** | At startup it averages 20 readings from each LDR to learn the "neutral" light level of the room. |
| **Read** | In the loop it keeps a rolling average of the last 20 readings per side (smoothing out flicker). |
| **Decide** | It compares left vs. right against the baseline. Note: the LDR value goes *down* as light goes *up*, so the code inverts the sign. |
| **Act** | It drives the motor (`D9`/`D10`) toward the brighter side, with strength proportional to how big the difference is. |

The takeaway for STEAM: this is a complete **closed feedback loop** — the robot senses the world, makes a decision, and acts on it, with no human in the loop. That's the seed of computational thinking.

---

## Pinout (Sketch A)

| Function | Pin |
|---|---|
| Motor driver 1 | `D9` |
| Motor driver 2 | `D10` |
| LDR left | `A1` |
| LDR right | `A0` |

---

## Quick troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Board **doesn't appear** under Port | Charge-only cable, or board stuck | Swap the USB-C cable. If it persists: **bootloader mode** → hold **BOOT**, press and release **RESET**, release **BOOT**. The port reappears; upload again. |
| **Serial Monitor blank** | CDC missing | **Tools › USB CDC On Boot → Enabled** and upload again. |
| `A fatal error occurred: Failed to connect` | Port busy or in run mode | Close the Serial Monitor, enter bootloader mode (BOOT+RESET) and retry. |
| IDE shows **DFRobot Beetle** on connect | Known auto-detection bug | Ignore it: manually select `XIAO_ESP32C3` under **Tools › Board** and upload anyway. |
| **Motor spins by itself** for an instant on boot | Pin `D9` is a *strapping* pin (boots high) | Normal. The code silences it as soon as it starts. |
| Motor doesn't react to light | Room too bright/even, or LDRs swapped | Cover one LDR fully to create contrast; check `A0`/`A1` wiring. |
