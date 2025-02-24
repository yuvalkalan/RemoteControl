#include "matrix.h"

Matrix::Matrix(const uint8_t rowPins[4], const uint8_t colPins[4])
    : row_pins{ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN},
      col_pins{COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN}
{
    // initialize the pins
    for (int i = 0; i < 4; i++)
    {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], 0); // Default LOW for rows

        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_down(col_pins[i]); // Enable pull-down for columns
    }
}
int8_t Matrix::scan() const
{
    int8_t key = -1;
    for (int row = 0; row < 4 && key == -1; row++)
    {
        gpio_put(row_pins[row], 1); // Set the current row HIGH

        for (int col = 0; col < 4 && key == -1; col++)
        {
            if (gpio_get(col_pins[col]))
            {                         // Check if a column pin is HIGH
                key = KEYS[row][col]; // Return the key value
            }
        }
        gpio_put(row_pins[row], 0); // Reset the row to LOW
    }
    return key; // No key pressed
}
std::vector<uint8_t> Matrix::scan_all() const
{
    std::vector<uint8_t> keys;
    for (int row = 0; row < 4; row++)
    {
        gpio_put(row_pins[row], 1); // Set the current row HIGH
        sleep_ms(1);                // Debounce delay
        for (int col = 0; col < 4; col++)
        {
            if (gpio_get(col_pins[col]))
            { // Check if a column pin is HIGH
                keys.push_back(KEYS[row][col]);
            }
        }
        gpio_put(row_pins[row], 0); // Reset the row to LOW
    }
    return keys;
}