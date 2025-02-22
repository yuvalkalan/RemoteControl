#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ST7789/ST7789.h"

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    gpio_init(25);
    gpio_set_dir(25, true);
    printf("f0\n");
    ST7789 display(ST7789_SPI_PORT, ST7789_SPI_BAUDRATE, ST7789_PIN_SCK, ST7789_PIN_SDA, ST7789_PIN_DC, ST7789_PIN_RES);
    display.init();
    // display.fill(COLOR_565_BLUE);
    display.update();
    uint8_t x = 0, y = 0;
    uint16_t color = 0;
    while (true)
    {
        display.draw_pixel(x++, y, COLOR_565_RED);
        if (x == 240)
        {
            x = 0;
            y++;
        }
        display.update();
        gpio_put(25, !gpio_get(25));
        // if (k == 50)
        //     display.hard_reset();
        // sleep_ms(1000);
    }

    return 0;
}
