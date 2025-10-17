import serial
import time
import sys
import glob

def get_serial_port():
    """ Scans for serial ports and lets the user choose one. """
    ports = glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/tty.usbserial*')
    if not ports:
        print("Error: No Arduino found. Check connection.")
        sys.exit(1)
    
    print("Please select the Arduino's serial port:")
    for i, p in enumerate(ports):
        print(f"  {i}: {p}")
    
    choice = int(input("Enter choice: "))
    return ports[choice]

def main():
    filename = "multicart.bin"
    chunk_size = 64  # Send data in 64-byte chunks

    try:
        port_name = get_serial_port()
        ser = serial.Serial(port_name, 115200, timeout=5)
        print(f"Opened port {port_name}")
        time.sleep(2) # Wait for Arduino to reset
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    # Wait for Arduino to be ready
    print("Waiting for Arduino... (If stuck, reset the Arduino)")
    response = ser.readline().decode().strip()
    if response != "READY":
        print(f"Error: Expected 'READY', got '{response}'")
        return
    print("Arduino is ready.")

    # 1. Erase all 64 sectors of the 256KB chip
    print("Erasing chip (64 sectors)...")
    for i in range(64):
        ser.write(b'E') # Erase command
        ser.write(i.to_bytes(1, 'little'))
        response = ser.readline().decode().strip()
        if response != "OK":
            print(f"Error erasing sector {i}!")
            return
        print(f"  Sector {i+1}/64 erased.", end='\r')
    print("\nErase complete.")

    # 2. Write the binary file in chunks
    print(f"Writing {filename}...")
    with open(filename, "rb") as f:
        address = 0
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            
            ser.write(b'W') # Write command
            ser.write(address.to_bytes(4, 'little'))
            ser.write(len(chunk).to_bytes(2, 'little'))
            ser.write(chunk)
            
            response = ser.readline().decode().strip()
            if response != "OK":
                print(f"Error writing at address {address}!")
                return
            
            address += len(chunk)
            progress = (address / 262144) * 100
            print(f"  Progress: {progress:.1f}%", end='\r')
            
    print("\nWrite complete.")
    print("✅ Multi-cartridge programmed successfully!")
    ser.close()

if __name__ == "__main__":
    main()

