# Dragon64/CoCo Multi-Cartridge

This repository contains the complete hardware design files for a switchable multi-cartridge for the Dragon64/CoCo. The board supports various SST39SF0X0 series flash chips and uses an 8-position DIP switch for bank selection.

---
## Features

* **Chip Support**: Designed for SST39SF010 (128KB), SST39SF020 (256KB), and SST39SF040 (512KB) parallel NOR flash chips in a DIP-32 package.
* **Bank Switching**: An 8-position DIP switch allows for hardware-based bank selection.
* **8K/16K Mode**: A jumper allows for easy switching between 8K and 16K cartridge modes.
* **Custom Case**: Includes an STL file for a 3D-printable cartridge shell.

---
## Repository Contents

* `Gerber_CoCoMultiCart.zip`: Gerber files ready for submission to a PCB manufacturer (like JLCPCB, PCBWay, etc.).
* `CoCoCartridge1.stl`: Contains a matching `.stl` file for the 3D-printable cartridge case.
* `Schem_CoCoMultiCart.pdf`: A document containing the board schematic, layout, and other project details.

---
## How to Build Your Cartridge

### 1. Manufacture the PCB

Upload the `.zip` file from the `/Gerber/` directory to your preferred PCB fabrication service (shout out to JCLPCB).

### 2. Gather Components (Bill of Materials)

| Part                  | Quantity | Notes                                          |
| :-------------------- | :------- | :--------------------------------------------- |
| Custom PCB            | 1        | From the Gerber files.                         |
| SST39SF0X0 Chip       | 1        | e.g., SST39SF020 for a 32-in-1 8K cart.         |
| 32-pin DIP Socket     | 1        | **Highly recommended** to avoid soldering the chip directly. |
| 8-Position DIP Switch | 1        | For bank selection.                            |
| 10kΩ Resistor Array   | 1        | SIP-9 package, for switch pull-ups.            |
| 0.1µF Capacitor       | 1        | Power supply decoupling.                       |
| 3-pin Header          | 2        | For the 8K/16K mode jumper, autostart jumper.  |
| Jumper Shunt          | 2        | To place on the header.                        |

### 3. Assemble the Board

Solder all components onto the PCB according to the schematic in the PDF. Solder the DIP socket, not the flash chip itself.

### 4. Program the Flash Chip

The flash chip must be programmed **before** it is inserted into the socket on the cartridge.

Use the Arduino Mega Parallel Flash Programmer to build a multi-cart binary image and write it to the chip. Follow the instructions in that repository's README file.

### 5. Print the Case

Print the `.stl` file on your 3D printer to create a protective shell for your finished cartridge.

---
## Usage

1.  Insert the programmed chip into the socket on the cartridge board.
2.  Set the jumper to either **8K** or **16K** mode, depending on the multi-cart binary you created.
3.  Use the DIP switches to select the desired game bank. **Note:** The switches use inverted logic (ON = 0, OFF = 1) due to the pull-up resistors.
4.  Insert the cartridge into your Color Computer and power it on.
