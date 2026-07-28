# CABLEBOTTT — Sketch B: `cablebottt_web`

**Phone-controlled robot.** The cablebot creates its own WiFi network and serves a webapp from the board. From your phone you drive the motor and control a color LED, a buzzer, a servo, and an optional distance sensor.

Board: **Seeed Studio XIAO ESP32-C3** · File: **`cablebottt_web/cablebottt_web.ino`** · Repo: `github.com/fablabbcn/cablebottt`

---

## 📚 Libraries needed: 2 (+ 1 automatic)

Unlike Sketch A, this sketch **needs external libraries**. Install these from Library Manager:

| Library | Why |
|---|---|
| **Adafruit NeoPixel** | Drives the color LED (`D7`). |
| **Adafruit VL53L1X** | Drives the time-of-flight distance sensor (`D4`/`D5`). |
| **Adafruit BusIO** | Dependency of VL53L1X — installs **automatically** when you accept "Install All". |

Everything else (`WiFi`, `WebServer`, `Wire`) is already part of the ESP32 core — nothing to add for those.

---

## Part 1 — One-time setup (once per computer)

> If you already set this up for **Sketch A**, skip to **Part 2**. The board setup is identical; only the libraries below are new.

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

> Without this, the ESP32-C3's Serial Monitor stays **blank** over USB-C — and you won't see the boot log that tells you the WiFi came up. Turn it on before you start. Leave the other **Tools** options at their defaults.

---

## Part 2 — Install the libraries

Go to **Tools › Manage Libraries…** (or the books icon on the left) and install, one by one:

1. Search **`Adafruit NeoPixel`** → **Install**.
2. Search **`Adafruit VL53L1X`** → **Install**.
   - The IDE will ask whether to also install its **dependencies** (`Adafruit BusIO`). Click **"Install All"**. ✅

That's it — those two (plus BusIO, which comes along) cover everything the sketch imports.

---

## Part 3 — Upload Sketch B

### 3.1 · Open the code
Open **`cablebottt_web/cablebottt_web.ino`**.

### 3.2 · Check board, port and CDC
- Board **`XIAO_ESP32C3`**, correct port.
- **USB CDC On Boot → `Enabled`** (Part 1.5) so you can see the boot log.

### 3.3 · Upload
Press the **Upload** button (→ arrow). It compiles (slower than Sketch A because of the WiFi libraries) and writes.
If it ends with **`Hard resetting via RTS pin…`** → uploaded successfully ✅.

### 3.4 · Verify it works
1. Open **Tools › Serial Monitor** at **`115200 baud`**. You should see the boot sequence:
   ```
   [BOOT] Iniciando cablebottt...
   [BOOT] Motor listo (D9/D10)
   [BOOT] Neopixel listo (D7)
   [BOOT] Servo listo (D8)
   [BOOT] Iniciando WiFi AP...
   [BOOT] SSID: CABLEBOTTT  IP: 192.168.4.1 ...
   [BOOT] Servidor web iniciado
   ```
   The **NeoPixel also runs a self-test**: it blinks red → green → blue. If it doesn't blink, the problem is the library or the LED hardware, not the WiFi.
2. On your **phone**, open WiFi settings and connect to:
   - **Network:** `CABLEBOTTT`
   - **Password:** `cablebottt`
3. Open a browser and go to **`http://192.168.4.1`**
4. The webapp appears. Try each control:
   - **Motor:** press and hold *Left* / *Right* (release to stop).
   - **Neopixel:** turn it on and pick a color.
   - **Buzzer:** play the Mario tune.
   - **Servo:** move the slider.
   - **Distance:** it comes **disabled**; tap it (*Sensor: OFF → ON*) only if you have the VL53L1X connected over I²C (`D4`/`D5`).

---

## How it works (to explain to students)

| Part | What happens |
|---|---|
| **Access Point** | The board doesn't join your WiFi — it *creates* its own network (`CABLEBOTTT`). Your phone connects directly to the robot. |
| **Web server** | The board serves a small webpage. Every button/slider sends a request (e.g. `/motor?dir=left`) that the board acts on. |
| **Outputs** | Motor (`D9`/`D10`), color LED (`D7`), buzzer (`D6`), servo (`D8`) — all driven on command from the page. |
| **Input** | The distance sensor (`D4`/`D5`) is an *input*: when enabled, the board reads it and the page shows the value live. |
| **Safety** | If no command arrives for ~1 second, the motor stops on its own — so a dropped connection can't leave it running. |

The takeaway for STEAM: same motor as Sketch A, but now **the student is in the loop**. They decide what each input and output does — the door to programming their own behaviors.

---

## Pinout (Sketch B)

| Function | Pin |
|---|---|
| Motor driver 1 | `D9` |
| Motor driver 2 | `D10` |
| NeoPixel | `D7` |
| Buzzer | `D6` |
| Servo | `D8` |
| Distance sensor SDA / SCL | `D4` / `D5` |

---

## Quick troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Compiles but a **library is missing** | NeoPixel / VL53L1X not installed | Part 2. Remember **"Install All"** so BusIO comes in too. |
| Board **doesn't appear** under Port | Charge-only cable, or board stuck | Swap the USB-C cable. If it persists: **bootloader mode** → hold **BOOT**, press and release **RESET**, release **BOOT**. The port reappears; upload again. |
| **Serial Monitor blank** | CDC missing | **Tools › USB CDC On Boot → Enabled** and upload again. |
| `A fatal error occurred: Failed to connect` | Port busy or in run mode | Close the Serial Monitor, enter bootloader mode (BOOT+RESET) and retry. |
| IDE shows **DFRobot Beetle** on connect | Known auto-detection bug | Ignore it: manually select `XIAO_ESP32C3` under **Tools › Board** and upload anyway. |
| Can't see the `CABLEBOTTT` network | The AP didn't come up | Check the Serial: if it says `softAP() devolvió FALLO`, reset the board. Move away from crowded WiFi areas. |
| Connected to WiFi but page **won't load** | Wrong address | Type the full `http://192.168.4.1` (some phones auto-search instead of browsing). Disable mobile data so the phone uses the robot's network. |
| **NeoPixel** doesn't do the red/green/blue self-test | Library or LED wiring | Reinstall Adafruit NeoPixel; check the `D7` connection. It's independent of the WiFi. |
| **Motor spins by itself** for an instant on boot | Pin `D9` is a *strapping* pin (boots high) | Normal. The code forces it LOW as soon as it starts. |
| **Distance** shows *"Sensor no conectado"* | Sensor missing or I²C wiring | Only enable it if the VL53L1X is wired to `D4`/`D5`. Otherwise leave it OFF. |
