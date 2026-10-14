
#include "Particle.h"

SYSTEM_MODE(MANUAL);

// Particle Argon I2C LCD backpack test
// Supports common PCF8574 backpacks
// Default display: 16 columns x 2 rows

const uint8_t LCD_RS = 0x01;
const uint8_t LCD_EN = 0x04;
const uint8_t LCD_BL = 0x08;

uint8_t lcdAddress = 0;
bool lcdReady = false;

// Find a PCF8574 or PCF8574A backpack
uint8_t findLCD() {
    for (uint8_t addr = 0x20; addr <= 0x3F; addr++) {

        // Valid address ranges
        if (addr > 0x27 && addr < 0x38)
            continue;

        Wire.beginTransmission(addr);

        if (Wire.endTransmission() == 0) {
            return addr;
        }
    }
    return 0;
}

// Send byte to the I2C backpack
void lcdExpanderWrite(uint8_t data) {
    Wire.beginTransmission(lcdAddress);
    Wire.write(data | LCD_BL);
    Wire.endTransmission();
}

// Pulse LCD enable
void lcdPulse(uint8_t data) {
    lcdExpanderWrite(data | LCD_EN);
    delayMicroseconds(1);
    lcdExpanderWrite(data & ~LCD_EN);
    delayMicroseconds(50);
}

// Send 4 bits
void lcdNibble(uint8_t nibble, uint8_t mode) {
    lcdPulse((nibble << 4) | mode);
}

// Send command or character
void lcdSend(uint8_t value, uint8_t mode) {
    lcdNibble(value >> 4, mode);
    lcdNibble(value & 0x0F, mode);
}

void lcdCommand(uint8_t command) {
    lcdSend(command, 0);

    if (command == 0x01 || command == 0x02) {
        delay(2);
    }
}

// Display text
void lcdPrint(const char* text) {
    while (*text) {
        lcdSend((uint8_t)*text++, LCD_RS);
    }
}

// Set cursor position
void lcdCursor(uint8_t col, uint8_t row) {
    const uint8_t offsets[] = {
        0x00, 0x40, 0x14, 0x54
    };

    if (row > 3) return;

    lcdCommand(0x80 | (offsets[row] + col));
}

// Initialize LCD in 4-bit mode
void lcdBegin() {
    delay(50);

    lcdNibble(0x03, 0);
    delay(5);

    lcdNibble(0x03, 0);
    delay(5);

    lcdNibble(0x03, 0);
    delayMicroseconds(200);

    lcdNibble(0x02, 0);

    lcdCommand(0x28);  // 4-bit, 2-line
    lcdCommand(0x0C);  // Display ON
    lcdCommand(0x06);  // Entry mode
    lcdCommand(0x01);  // Clear screen
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    delay(100);

    lcdAddress = findLCD();

    if (lcdAddress == 0) {
        Serial.println("LCD NOT FOUND");
        return;
    }

    Serial.printf("LCD found at 0x%02X\n", lcdAddress);

    lcdBegin();
    lcdReady = true;

    lcdCursor(0, 0);
    lcdPrint("PARTICLE ARGON");

    lcdCursor(0, 1);
    lcdPrint("LCD TEST OK");
}

void loop() {
    if (!lcdReady) {
        delay(1000);
        return;
    }

    static unsigned long count = 0;
    char line[17];

    snprintf(line, sizeof(line),
             "Count: %-9lu", count++);

    lcdCursor(0, 1);
    lcdPrint(line);

    delay(1000);
}
