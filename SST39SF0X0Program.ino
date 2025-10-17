// =============================================================================
// == Pin Definitions for Arduino Mega 2560                                   ==
// =============================================================================
const int OE_PIN = 50;
const int WE_PIN = 51;

// Arrays to hold the pin numbers for easy iteration
const int ADDR_PINS[19] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
const int DATA_PINS[8]  = {42, 43, 44, 45, 46, 47, 48, 49};


// =============================================================================
// == Hardware Abstraction Layer (HAL) for Arduino Mega                       ==
// =============================================================================

// Initializes all the pins used by the programmer.
void hal_init_pins() {
  // Initialize control pins
  pinMode(OE_PIN, OUTPUT);
  pinMode(WE_PIN, OUTPUT);
  digitalWrite(OE_PIN, HIGH); // Inactive
  digitalWrite(WE_PIN, HIGH); // Inactive

  // Initialize address pins
  for (int i = 0; i < 19; i++) {
    pinMode(ADDR_PINS[i], OUTPUT);
  }
  
  // Data bus direction will be set as needed.
}

// Sets the direction of the 8-bit Data Bus
void hal_set_data_bus_direction(bool is_output) {
  for (int i = 0; i < 8; i++) {
    pinMode(DATA_PINS[i], is_output ? OUTPUT : INPUT);
  }
  delayMicroseconds(1); // Small delay for pins to settle
}

// Writes a byte to the Data Bus
void hal_write_data_bus(uint8_t data) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(DATA_PINS[i], (data >> i) & 0x01);
  }
}

// Reads a byte from the Data Bus
uint8_t hal_read_data_bus() {
  uint8_t data = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(DATA_PINS[i]) == HIGH) {
      data |= (1 << i);
    }
  }
  return data;
}

// Sets the 19-bit address on the address bus
void hal_set_address(uint32_t address) {
  for (int i = 0; i < 19; i++) {
    digitalWrite(ADDR_PINS[i], (address >> i) & 0x01);
  }
}

// Pulses the Write Enable (#WE) pin
void hal_pulse_write_enable() {
  digitalWrite(WE_PIN, LOW);
  delayMicroseconds(1); // Meets datasheet minimums
  digitalWrite(WE_PIN, HIGH);
  delayMicroseconds(1); // Recovery time
}

// Controls the Output Enable (#OE) pin
void hal_set_output_enable(bool is_enabled) {
  digitalWrite(OE_PIN, is_enabled ? LOW : HIGH);
}


// =============================================================================
// == High-Level SST39SF0X0 Functions                                         ==
// =============================================================================

// Sends a single command cycle to the flash chip
void sst_send_command(uint32_t address, uint8_t data) {
  hal_set_address(address);
  hal_write_data_bus(data);
  hal_pulse_write_enable();
}

// Reads a single byte from a specified address
uint8_t sst_read_byte(uint32_t address) {
  hal_set_data_bus_direction(false);
  hal_set_address(address);
  hal_set_output_enable(true);
  delayMicroseconds(1); // Give time for output drivers to enable
  uint8_t data = hal_read_data_bus();
  hal_set_output_enable(false);
  return data;
}

// Waits for the chip to finish an operation using Toggle Bit polling.
bool wait_for_completion(uint32_t address) {
  uint8_t read1, read2;
  unsigned long start_time = millis();

  // A real erase can take ~25ms, so a 200ms timeout is safe.
  while (millis() - start_time < 200) {
    read1 = sst_read_byte(address);
    read2 = sst_read_byte(address);
    // Check if DQ6 has stopped toggling
    if ((read1 & 0x40) == (read2 & 0x40)) {
      return true; // Success!
    }
  }
  return false; // Timeout
}

// Erases a 4-KByte sector.
bool sst_sector_erase(uint32_t sector_address) {
  hal_set_data_bus_direction(true);
  
  sst_send_command(0x5555, 0xAA);
  sst_send_command(0x2AAA, 0x55);
  sst_send_command(0x5555, 0x80);
  sst_send_command(0x5555, 0xAA);
  sst_send_command(0x2AAA, 0x55);
  sst_send_command(sector_address, 0x30);
  
  return wait_for_completion(sector_address);
}

// =============================================================================
// == Main Program Logic                                                      ==
// =============================================================================

void setup() {
  Serial.begin(115200);
  hal_init_pins();

  // The Arduino is now ready. It will wait for commands from the Python script.
  Serial.println("READY"); 
}

void loop() {
  // Check if a command has arrived from the computer
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'E') { // 'E' for Erase
      // Wait for and read the sector number from serial
      while (Serial.available() < 1);
      uint8_t sector_to_erase = Serial.read();
      
      if (sst_sector_erase(sector_to_erase * 4096UL)) {
        Serial.println("OK"); // Acknowledge success
      } else {
        Serial.println("FAIL");
      }
    } 
    else if (command == 'W') { // 'W' for Write
      // Wait for and read the 32-bit address (4 bytes)
      uint32_t address = 0;
      for (int i = 0; i < 4; i++) {
        while (Serial.available() < 1);
        address |= (uint32_t)Serial.read() << (i * 8);
      }

      // Wait for and read the number of bytes to write (2 bytes)
      uint16_t length = 0;
      for (int i = 0; i < 2; i++) {
        while (Serial.available() < 1);
        length |= (uint16_t)Serial.read() << (i * 8);
      }
      
      // Program the bytes one by one as they arrive over serial
      for (uint16_t i = 0; i < length; i++) {
        while (Serial.available() < 1);
        uint8_t data_byte = Serial.read();
        
        hal_set_data_bus_direction(true);
        sst_send_command(0x5555, 0xAA);
        sst_send_command(0x2AAA, 0x55);
        sst_send_command(0x5555, 0xA0);
        sst_send_command(address + i, data_byte);
        
        if (!wait_for_completion(address + i)) {
          Serial.println("FAIL"); // Report failure and stop
          return;
        }
      }
      Serial.println("OK"); // Acknowledge success for the entire block
    }
  }
}