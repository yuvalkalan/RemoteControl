#include "stdio.h"
#include "pico/stdlib.h"
#include <string.h>
#include "hardware/spi.h"
#include "graphics/graphics.h"
#include "matrix/matrix.h"
#include "Socket/Socket.h"
#include "SerialIn/SerialIn.h"

#define LED_PIN 25
#define POWER_PIN 16

// millis function from arduino
#define millis() to_ms_since_boot(get_absolute_time())

#define is_debug() strcmp(PICO_CMAKE_BUILD_TYPE, "Debug") == 0

int main()
{
    // Initialize stdio
    stdio_init_all();
    // initialize gpio pins for power and LED
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, true);
    gpio_put(POWER_PIN, true);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    gpio_put(LED_PIN, true);
    sleep_ms(1000);

    // initialize display
    ST7789 display(ST7789_SPI_PORT, ST7789_SPI_BAUDRATE, ST7789_PIN_SCK, ST7789_PIN_SDA, ST7789_PIN_DC, ST7789_PIN_RES);
    display.init();

    // initialize matrix keypad
    uint8_t rows[] = {ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN};
    uint8_t cols[] = {COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN};
    Matrix keypad(rows, cols);
    // Matrix keypad({ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN}, {COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN});

    GraphicsText text(0, 0, "hello world!", 1);
    text.center_x(ST7789_WIDTH / 2);
    text.center_y(ST7789_HEIGHT / 2);
    auto start_time = millis();
    while (true)
    {
        auto current_time = millis();
        if (current_time - start_time > 9000 && !is_debug())
        {
            break;
        }
        char display_text[] = "0";
        display_text[0] += (uint8_t)(9 - (current_time - start_time) / 1000);
        GraphicsText text(0, 0, display_text, 1);
        text.center_x(ST7789_WIDTH / 2);
        text.center_y(ST7789_HEIGHT / 2);
        display.fill(COLOR_565_BLACK);
        text.draw(display, COLOR_565_WHITE);
        display.update();
        gpio_put(LED_PIN, !gpio_get(LED_PIN));
        auto keys = keypad.scan_all();
        if (keys.size() > 0)
        {
            printf("Keys pressed: ");
            for (auto key : keys)
            {
                printf("%d ", key);
            }
            printf("\n");
        }
    }
    gpio_put(POWER_PIN, false); // turn off
    return 0;
}
