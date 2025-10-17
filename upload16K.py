import serial
import time
import sys
import glob

def get_serial_port():
    """ Scans for available serial ports and lets the user choose one. """
    # Look for common Arduino port names on macOS
    ports = glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/tty.usbserial*')
    if not ports:
        print("Error: No Arduino found. Please check the USB connection.")
        sys.exit(1)
    
    if len(ports) == 1:
        print(f"Auto-selecting Arduino at: {ports[0]}")
        return ports[0]

    print("Please select the Arduino's serial port:")
    for i, p in enumerate(ports):
        print(f"  {i}: {p}")
    
    try:
        choice = int(input("Enter choice: "))
        return ports[choice]
    except (ValueError, IndexError):
        print("Invalid choice. Exiting.")
        sys.exit(1)

def main():
    # --- UPDATED FOR 16K MULTI-CART ---
    filename = "multicart_16k.bin"
    total_size = 524288  # 512KB
    num_sectors = 128    # 512KB / 4KB per sector
    # ------------------------------------
    
    chunk_size = 64  # Send data in 64-byte chunks for efficiency

    try:
        port_name = get_serial_port()
        ser = serial.Serial(port_name, 115200, timeout=5)
        print(f"Opened port {port_name}")
        time.sleep(2) # Wait for the Arduino to reset after connection
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    # Wait for the Arduino to send its "READY" signal
    print("Waiting for Arduino... (If stuck, please reset the Arduino)")
    response = ser.readline().decode().strip()
    if response != "READY":
        print(f"Error: Expected 'READY' signal from Arduino, but got '{response}'")
        return
    print("Arduino is ready.")

    # 1. Erase all sectors of the 512KB chip
    print(f"Erasing chip ({num_sectors} sectors)...")
    for i in range(num_sectors):
        ser.write(b'E') # Erase command
        ser.write(i.to_bytes(1, 'little')) # Sector number
        response = ser.readline().decode().strip()
        if response != "OK":
            print(f"\nError erasing sector {i}! Halting.")
            return
        # Update progress on a single line
        print(f"  Sector {i+1}/{num_sectors} erased.", end='\r')
    print("\nErase complete.")

    # 2. Write the binary file in chunks
    print(f"Writing {filename}...")
    try:
        with open(filename, "rb") as f:
            address = 0
            while True:
                chunk = f.read(chunk_size)
                if not chunk:
                    break # End of file
                
                ser.write(b'W') # Write command
                ser.write(address.to_bytes(4, 'little')) # 32-bit start address
                ser.write(len(chunk).to_bytes(2, 'little')) # 16-bit length
                ser.write(chunk) # The data
                
                response = ser.readline().decode().strip()
                if response != "OK":
                    print(f"\nError writing at address {address}! Halting.")
                    return
                
                address += len(chunk)
                progress = (address / total_size) * 100
                print(f"  Progress: {progress:.1f}%", end='\r')
    except FileNotFoundError:
        print(f"\nError: The file '{filename}' was not found in this directory.")
        return
            
    print("\nWrite complete.")
    print("✅ Multi-cartridge programmed successfully!")
    ser.close()

if __name__ == "__main__":
    main()
