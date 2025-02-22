#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "graphics/graphics.h"

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
    GraphicsText text(0, 0, "hello world", 1);
    text.center_x(ST7789_WIDTH / 2);
    text.center_y(ST7789_HEIGHT / 2);
    while (true)
    {
        display.fill(COLOR_565_BLACK);
        text.draw(display, COLOR_565_WHITE);
        display.update();
        gpio_put(25, !gpio_get(25));
    }

    return 0;
}
