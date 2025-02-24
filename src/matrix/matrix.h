#pragma once
#include "pico/stdlib.h"
#include <vector>

// Define the GPIO pins for the keypad matrix
#define ROW0_PIN 0 // GPIO 0
#define ROW1_PIN 1 // GPIO 1
#define ROW2_PIN 2 // GPIO 2
#define ROW3_PIN 3 // GPIO 3
#define COL0_PIN 4 // GPIO 4
#define COL1_PIN 5 // GPIO 5
#define COL2_PIN 6 // GPIO 6
#define COL3_PIN 7 // GPIO 7

// Keymap for the 4x4 keypad
static const uint8_t KEYS[4][4] = {
    {0, 1, 2, 3},
    {4, 5, 6, 7},
    {8, 9, 10, 11},
    {12, 13, 14, 15}};

// Keypad matrix class to scan a 4x4 matrix keypad
class Matrix
{
private:
    const uint8_t row_pins[4];
    const uint8_t col_pins[4];

public:
    // Constructor to initialize pins and keymap
    Matrix(const uint8_t rowPins[4], const uint8_t colPins[4]);

    // Scan the keypad and return the first pressed key
    int8_t scan() const;

    // Scan the keypad and return all pressed keys
    std::vector<uint8_t> scan_all() const;
};