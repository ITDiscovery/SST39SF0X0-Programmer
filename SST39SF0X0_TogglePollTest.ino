#include <avr/io.h>
#include <util/delay.h>

// =============================================================================
// == Hardware Abstraction Layer (HAL) - (Copy from previous sketch)         ==
// =============================================================================
// NOTE: HAL functions are identical and should be copied from the previous sketch.
void hal_init_pins() { PORTA.DIRSET = 0xE0; PORTC.DIR = 0xFF; PORTD.DIR = 0xFF; PORTE.DIRSET = 0x07; PORTA.OUTSET = 0x60; }
void hal_set_data_bus_direction(bool is_output) { PORTB.DIR = is_output ? 0xFF : 0x00; }
void hal_write_data_bus(uint8_t data) { PORTB.OUT = data; }
uint8_t hal_read_data_bus() { return PORTB.IN; }
void hal_set_address(uint32_t address) { PORTC.OUT = (uint8_t)address; PORTD.OUT = (uint8_t)(address >> 8); uint8_t pe = PORTE.OUT & 0xF8; if (address & (1UL << 16)) pe |= 1; if (address & (1UL << 17)) pe |= 2; if (address & (1UL << 18)) pe |= 4; PORTE.OUT = pe; }
void hal_pulse_write_enable() { PORTA.OUTCLR = (1 << 5); _delay_us(1); PORTA.OUTSET = (1 << 5); }
void hal_set_output_enable(bool is_enabled) { if (is_enabled) PORTA.OUTCLR = (1 << 6); else PORTA.OUTSET = (1 << 6); }
void hal_set_led(bool is_on) { if (is_on) PORTA.OUTSET = (1 << 7); else PORTA.OUTCLR = (1 << 7); }


// =============================================================================
// == High-Level SST39SF0X0 Functions                                         ==
// =============================================================================

// Sends a single command cycle to the flash chip
void sst_send_command(uint32_t address, uint8_t data) {
    hal_set_address(address);
    hal_write_data_bus(data);
    _delay_us(10000); // Critical stabilization delay
    hal_pulse_write_enable();
}

// Reads a single byte from a specified address
uint8_t sst_read_byte(uint32_t address) {
    hal_set_address(address);
    hal_set_output_enable(true);
    _delay_us(1);
    uint8_t data = hal_read_data_bus();
    hal_set_output_enable(false);
    return data;
}

/**
 * @brief NEW FUNCTION: Waits for the chip to finish an operation
 * using the robust Toggle Bit (DQ6) polling method.
 */
bool wait_for_completion(uint32_t address) {
    Serial.print("Polling (DQ6 Toggle)...");
    
    hal_set_data_bus_direction(false); // Ensure bus is ready for reading
    
    uint8_t read1, read2;
    unsigned long start_time = millis();

    while (true) {
        read1 = sst_read_byte(address);
        read2 = sst_read_byte(address);

        // Check if DQ6 is toggling
        if ((read1 & 0x40) == (read2 & 0x40)) {
            // DQ6 has stopped toggling, operation is complete.
            Serial.println("\nPolling complete. ✅");
            return true;
        }

        if (millis() - start_time > 100) { // Timeout
            Serial.println("\nError: Polling timed out!");
            return false;
        }
    }
}

// =============================================================================
// == Main Program                                                            ==
// =============================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    hal_init_pins();

    Serial.println("\n--- SST39SF0X0 Toggle Poll Test ---");
    
    const uint32_t TARGET_ADDRESS = 0x00000;
    const uint8_t TEST_PATTERN = 0xCC;

    // --- STEP 1: SEND THE WRITE COMMAND ---
    Serial.println("Sending write command...");
    hal_set_data_bus_direction(true); // Output for commands
    sst_send_command(0x5555, 0xAA);
    sst_send_command(0x2AAA, 0x55);
    sst_send_command(0x5555, 0xA0);
    sst_send_command(TARGET_ADDRESS, TEST_PATTERN);

    // --- STEP 2: WAIT FOR COMPLETION USING NEW POLLING METHOD ---
    bool success = wait_for_completion(TARGET_ADDRESS);
    
    // --- STEP 3: VERIFY THE RESULT ---
    if (success) {
        uint8_t final_value = sst_read_byte(TARGET_ADDRESS);
        Serial.print("Final value at 0x");
        Serial.print(TARGET_ADDRESS, HEX);
        Serial.print(" is 0x");
        Serial.print(final_value, HEX);

        if (final_value == TEST_PATTERN) {
            Serial.println(" -> SUCCESS! 🎉");
            hal_set_led(true);
        } else {
            Serial.println(" -> VERIFICATION FAILED!");
        }
    } else {
        Serial.println("Write operation failed to complete.");
    }
     Serial.println("\n--- Test Complete ---");
}

void loop() { }
