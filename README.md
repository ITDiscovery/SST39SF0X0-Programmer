# Arduino Mega Parallel Flash Programmer for SST39SF0X0

This project documents a reliable, breadboard-based programmer for SST39SF0X0 series parallel NOR flash chips (SST39SF010, SST39SF020, SST39SF040) using an Arduino Mega 2560. The system was developed to program custom multi-cartridges for the Tandy/Radio Shack Color Computer.

The programmer consists of an Arduino sketch that acts as a hardware interface and a Python script that streams a binary ROM file from a host computer (Mac, PC, or Linux) to the Arduino for programming.

---
## Hardware Required

* **Arduino Mega 2560 R3**: A 5V microcontroller with enough I/O pins to directly drive the flash chip.
* **SST39SF0X0 Series Chip**: e.g., SST39SF020 (256KB) or SST39SF040 (512KB).
* **Breadboard(s)**: Sufficient space for a 32-pin DIP chip and wiring.
* **Jumper Wires**: A good quality set is essential.
* **Two 0.1µF Ceramic Capacitors**: For power decoupling.

  Note: I tried initially to use a AVR128DA64, but timing and jitter created too many issues.

---
## Wiring Diagram

Proper wiring and power integrity are critical for success. Place one 0.1µF capacitor directly across the VCC and GND pins of the Arduino, and the other directly across the VCC and GND pins of the flash chip.



| Signal      | SST39SF0X0 Pin | Mega 2560 Pin | Notes                              |
| :---------- | :------------- | :------------ | :--------------------------------- |
| **Address** | A0 - A15       | D22 - D37     | 16 contiguous address lines        |
|             | A16 - A18      | D38 - D40     | Upper 3 address lines              |
| **Data** | D0 - D7        | D42 - D49     | Full 8-bit data bus                |
| **Control** | #OE (Pin 24)   | D50           | Output Enable                      |
|             | #WE (Pin 31)   | D51           | Write Enable                       |
|             | #CE (Pin 11)   | GND           | Chip Enable (tied permanently LOW) |
| **Power** | VCC (Pin 32)   | 5V            | Connect to 5V rail                 |
|             | GND (Pin 16)   | GND           | Connect to Ground rail             |

---
## How to Use

### Step 1: Build Your Binary ROM File

Use the provided shell scripts to combine your individual 8K or 16K ROM files into a single binary image.

* **For an 8K Multi-Cart (SST39SF020):**
    * Edit the `build_cart.sh` script to list your 8K ROM files.
    * Run the script: `./build_cart.sh`
    * This creates `multicart.bin` (256KB).

* **For a 16K Multi-Cart (SST39SF040):**
    * Edit the `build_16k_cart.sh` script to list your 16K ROM files.
    * Run the script: `./build_16k_cart.sh`
    * This creates `multicart_16k.bin` (512KB).

### Step 2: Prepare the Arduino

1.  Connect your Arduino Mega to your computer.
2.  Open the `SST39SF0X0_Serial_Programmer.ino` sketch in the Arduino IDE.
3.  Upload the sketch to the Mega.
4.  Open the Serial Monitor and confirm it prints "READY".
5.  **Close the Serial Monitor.** This is critical, as the Python script needs exclusive access to the port.

### Step 3: Program the Chip

1.  Make sure Python 3 and the `pyserial` library are installed (`pip3 install pyserial`).
2.  Run the appropriate Python script from your terminal.

* **For the 8K Multi-Cart:**
    ```bash
    python3 upload8k.py
    ```

* **For the 16K Multi-Cart:**
    ```bash
    python3 upload16k.py
    ```
The script will auto-detect the serial port, erase the chip, and then write the binary file, showing you the progress.

---
## Project Files

* `SST39SF0X0_Serial_Programmer.ino`: The firmware for the Arduino Mega. This sketch turns the Arduino into a dedicated hardware programmer that listens for commands over the serial port.
* `upload8k.py`: A Python script for your host computer to program 256KB binary files (e.g., for an SST39SF020).
* `upload16k.py`: A Python script for your host computer to program 512KB binary files (e.g., for an SST39SF040).
* `build_cart.sh`: A shell script template to assemble multiple 8K ROMs into a single 256KB binary image.
* `build_16k_cart.sh`: A shell script template to assemble multiple 16K ROMs into a single 512KB binary image.
