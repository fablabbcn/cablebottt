# CABLEBOTTT — Sketch A: `cablebottt_laser`

**Autonomous light-following robot.** The cablebot reads two light-dependent resistors, or LDRs, and drives its motor towards the side receiving more light. It demonstrates a simple **sensor → decision → actuator** loop.

**Board:** Seeed Studio XIAO ESP32-C3
**File:** `cablebottt_laser/cablebottt.ino`
**Repository:** `github.com/fablabbcn/cablebottt`

---

## 📚 Libraries needed: none

This sketch does not use any external libraries. It relies only on functions included in the standard Arduino core for ESP32, including:

* `Serial`
* `analogRead`
* `analogWrite`
* `digitalWrite`
* `pinMode`
* `millis`
* `delay`

Once the ESP32 board package is installed, you can compile and upload the sketch directly. No additional installation through the Library Manager is required.

---

## Part 1 — One-time setup

You only need to complete this section once on each computer.

### 1.1 · Install the Arduino IDE

Download and install the stable version of **Arduino IDE 2.x** from:

`arduino.cc/en/software`

### 1.2 · Add ESP32 board support

1. Open **File › Preferences**. On macOS, open **Arduino IDE › Settings**.

2. Find **Additional Boards Manager URLs**.

3. Add:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

4. Press **OK**.

### 1.3 · Install the ESP32 boards package

1. Open **Tools › Board › Boards Manager**, or select the boards icon in the left sidebar.
2. Search for `esp32`.
3. Install **esp32 by Espressif Systems**.

Installation may take a few minutes.

### 1.4 · Select the board and port

1. Connect the XIAO using a **USB-C data cable**. Some USB cables provide power but do not transfer data.
2. Select **Tools › Board › esp32 › XIAO_ESP32C3**.
3. Open **Tools › Port** and select the port that appears when the board is connected.

Typical port names are:

* Windows: `COM3`, `COM4`, etc.
* macOS: `/dev/cu.usbmodem…`
* Linux: `/dev/ttyACM0` or similar

### 1.5 · Enable USB serial output

Under **Tools**, set:

```text
USB CDC On Boot → Enabled
```

Without this setting, the Serial Monitor may remain blank when using the XIAO ESP32-C3 over USB-C.

Leave the other board options at their default values unless your setup requires otherwise.

---

## Part 2 — Upload Sketch A

### 2.1 · Open the sketch

Open:

```text
cablebottt_laser/cablebottt.ino
```

You can double-click the `.ino` file or use **File › Open** in the Arduino IDE.

### 2.2 · Check the board and port

Confirm that:

* the selected board is `XIAO_ESP32C3`;
* the correct serial port is selected.

### 2.3 · Upload

Press the **Upload** button.

The Arduino IDE will compile the sketch and write it to the board. A message such as:

```text
Hard resetting via RTS pin...
```

normally indicates that the upload completed successfully.

### 2.4 · Test the robot

1. Open **Tools › Serial Monitor**.
2. Set the baud rate to `115200`.
3. Restart the board.
4. Keep the laser or flashlight away from both sensors while the robot calibrates.
5. Shine light onto one LDR.

The motor should move towards the side receiving the stronger light.

Calibration takes approximately 1.5 seconds after startup. The robot should remain still during this period.

---

## How the sketch works

| Step               | What happens                                                                                                              |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------- |
| **Stabilise**      | The sketch waits briefly after startup so the power supply and analogue readings can settle.                              |
| **Calibrate**      | It averages 25 readings from each LDR to establish the ambient-light baseline.                                            |
| **Read**           | It continuously reads both sensors and calculates a moving average from the latest five samples.                          |
| **Measure change** | It compares each filtered reading with its startup baseline. In this circuit, more light produces a lower analogue value. |
| **Ignore noise**   | Small changes are ignored using an activation threshold. Similar readings on both sides are ignored using a deadband.     |
| **Decide**         | The sketch identifies which LDR is receiving the stronger increase in light.                                              |
| **Act**            | It drives the motor towards that side. A larger difference between the sensors produces a higher motor speed.             |
| **Stop**           | The motor stops when there is no strong light signal or when both sensors receive similar amounts of light.               |

This creates a basic **closed feedback loop**: the robot senses its environment, evaluates the information and changes its movement in response.

---

## Key parameters

The behaviour can be adjusted near the beginning of the sketch:

```cpp
#define FILTER_SAMPLES 5
#define CALIBRATION_SAMPLES 25

const int ACTIVATION_THRESHOLD = 200;
const int DIFFERENCE_DEADBAND = 80;
const int MIN_MOTOR_SPEED = 70;
```

### `FILTER_SAMPLES`

Controls how many recent readings are averaged.

* Higher values produce smoother readings.
* Lower values respond more quickly.

### `CALIBRATION_SAMPLES`

Controls how many readings are used to establish the ambient-light baseline.

A higher value produces a more stable calibration, but takes slightly longer.

### `ACTIVATION_THRESHOLD`

Sets how large the change from ambient light must be before the robot reacts.

Increase this value if the robot reacts to normal room-light variation.

### `DIFFERENCE_DEADBAND`

Sets how different the two sensors must be before the motor chooses a direction.

Increase this value if the robot oscillates when both sensors receive similar light.

### `MIN_MOTOR_SPEED`

Sets the minimum PWM value used once movement is activated.

This helps ensure that the motor receives enough power to start moving rather than humming or remaining stalled.

---

## Pinout

| Function                   | Pin   |
| -------------------------- | ----- |
| Motor driver input 1       | `D9`  |
| Motor driver input 2       | `D10` |
| Left LDR                   | `A1`  |
| Right LDR                  | `A0`  |
| Buzzer, currently inactive | `D6`  |

---

## Quick troubleshooting

| Symptom                                      | Likely cause                                                                          | Fix                                                                                                                                            |
| -------------------------------------------- | ------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| The board does not appear under **Port**     | Charge-only USB cable or board not entering upload mode                               | Try another USB-C cable. To enter bootloader mode, hold **BOOT**, press and release **RESET**, then release **BOOT**.                          |
| Serial Monitor is blank                      | USB CDC is disabled or the wrong baud rate is selected                                | Set **USB CDC On Boot → Enabled**, upload again and select `115200` baud.                                                                      |
| `Failed to connect` during upload            | Incorrect port, port in use or board not in bootloader mode                           | Close the Serial Monitor, confirm the port and retry in bootloader mode.                                                                       |
| The IDE identifies the board incorrectly     | Automatic board detection is unreliable                                               | Manually select `XIAO_ESP32C3` under **Tools › Board**.                                                                                        |
| The motor moves briefly during startup       | `D9` is involved in the ESP32-C3 boot configuration                                   | A short movement may occur before the sketch takes control. Hardware pull-downs or a motor-driver enable pin provide a more reliable solution. |
| The motor does not react to light            | Signal is below the threshold, sensors are swapped or the room is already very bright | Cover one LDR or shine a focused light onto the other. Check the `A0` and `A1` connections.                                                    |
| The motor moves in the wrong direction       | Motor wiring or direction labels are reversed                                         | Swap the motor wires or exchange the calls to `driveLeft()` and `driveRight()`.                                                                |
| The motor hums but does not move             | PWM is too low or the power supply cannot provide enough current                      | Increase `MIN_MOTOR_SPEED` and check the battery, regulator and motor-driver supply.                                                           |
| The robot reacts unpredictably after startup | A sensor was illuminated during calibration                                           | Restart the board with both LDRs exposed only to normal ambient light.                                                                         |
