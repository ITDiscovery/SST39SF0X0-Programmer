#include <avr/io.h>
#include <util/delay.h>

// --- Hardware Abstraction Layer (HAL) based on your wiring ---

// Pin Definitions
#define LED_PIN    7 // PA7
#define OE_PIN     6 // PA6
#define WE_PIN     5 // PA5

// Helper function to initialize all microcontroller pins
void hal_init_pins() {
    // Configure Control Port A (PA5, PA6, PA7) as outputs
    PORTA.DIRSET = (1 << WE_PIN) | (1 << OE_PIN) | (1 << LED_PIN);
    
    // Configure Address Ports C, D, E as outputs
    PORTC.DIR = 0xFF; // A0-A7
    PORTD.DIR = 0xFF; // A8-A15
    PORTE.DIRSET = 0b00000111; // A16-A18
    
    // Set initial states for control lines (HIGH = inactive)
    PORTA.OUTSET = (1 << WE_PIN) | (1 << OE_PIN);
}

// Sets the direction of the 8-bit Data Bus (Port B)
void hal_set_data_bus_direction(bool is_output) {
    if (is_output) {
        PORTB.DIR = 0xFF; // All pins on Port B to Output
    } else {
        PORTB.DIR = 0x00; // All pins on Port B to Input
    }
}

// Writes a byte to the Data Bus (Port B)
void hal_write_data_bus(uint8_t data) {
    PORTB.OUT = data;
}

// Reads a byte from the Data Bus (Port B)
uint8_t hal_read_data_bus() {
    return PORTB.IN;
}

// Sets the 19-bit address on Ports C, D, and E
void hal_set_address(uint32_t address) {
    PORTC.OUT = (uint8_t)(address & 0xFF);         // A0-A7
    PORTD.OUT = (uint8_t)((address >> 8) & 0xFF);  // A8-A15
    
    // A16-A18 on PE0, PE1, PE2
    uint8_t port_e_val = PORTE.OUT & 0b11111000; // Clear lower 3 bits
    if (address & (1UL << 16)) port_e_val |= (1 << 0); // A16 -> PE0
    if (address & (1UL << 17)) port_e_val |= (1 << 1); // A17 -> PE1
    if (address & (1UL << 18)) port_e_val |= (1 << 2); // A18 -> PE2
    PORTE.OUT = port_e_val;
}

// Pulses the Write Enable pin LOW then HIGH with correct timing
void hal_pulse_write_enable() {
    PORTA.OUTCLR = (1 << WE_PIN); // Set #WE LOW
    _delay_us(1);                 // CRITICAL DELAY for Write Pulse Width
    PORTA.OUTSET = (1 << WE_PIN); // Set #WE HIGH
    _delay_us(1);                 // Pause before next operation
}

// Controls the Output Enable pin
void hal_set_output_enable(bool is_enabled) {
    if (is_enabled) {
        PORTA.OUTCLR = (1 << OE_PIN); // Set #OE LOW (Active)
    } else {
        PORTA.OUTSET = (1 << OE_PIN); // Set #OE HIGH (Inactive)
    }
}


// --- High-Level Logic (Now clean and portable) ---

/**
 * @brief Sends a single command cycle to the flash chip.
 * THIS IS THE MODIFIED FUNCTION.
 */
void sst_send_command(uint32_t address, uint8_t data) {
    hal_set_address(address);
    hal_write_data_bus(data);

    // --- NEW DELAY ---
    // This adds an explicit 1 microsecond delay to ensure the address
    // and data lines are perfectly stable before the #WE pulse.
    _delay_us(10000);

    hal_pulse_write_enable();
}

// Reads a byte from a specific address
uint8_t sst_read_byte(uint32_t address) {
    hal_set_address(address);
    hal_set_output_enable(true);
    _delay_us(1); // Give time for output drivers to enable
    uint8_t data = hal_read_data_bus();
    hal_set_output_enable(false);
    return data;
}


// --- Main Application ---

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for serial connection
    
    hal_init_pins();
    hal_set_data_bus_direction(true); // Start with bus as output for sending commands

    Serial.println("--- SST39SF020 Programmer Initialized (with setup delay) ---");
    Serial.println("Attempting to read Manufacturer and Device ID...");

    // --- Execute "Software Product ID Entry" Command ---
    sst_send_command(0x5555, 0xAA);
    sst_send_command(0x2AAA, 0x55);
    sst_send_command(0x5555, 0x90);

    // --- Switch to Read Mode ---
    hal_set_data_bus_direction(false); // Bus is now an input

    // --- Read the ID Bytes ---
    uint8_t manufacturer_id = sst_read_byte(0x00000);
    uint8_t device_id = sst_read_byte(0x00001);

    Serial.print("Manufacturer ID: 0x");
    Serial.print(manufacturer_id, HEX);
    Serial.println((manufacturer_id == 0xBF) ? " -> Correct (SST)" : " -> INCORRECT!");

    Serial.print("Device ID: 0x");
    Serial.print(device_id, HEX);
    Serial.println((device_id == 0xB6) ? " -> Correct (SST39SF020)" : " -> INCORRECT!");
    
    // --- Exit ID Mode ---
    hal_set_data_bus_direction(true);
    sst_send_command(0x5555, 0xAA);
    sst_send_command(0x2AAA, 0x55);
    sst_send_command(0x5555, 0xF0);
    hal_set_data_bus_direction(false);
}

void loop() {
    // Test runs once in setup
}