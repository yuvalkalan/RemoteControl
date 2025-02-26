#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/multicore.h"
#include <string.h>
#include "graphics/graphics.h"
#include "matrix/matrix.h"
#include "Socket/Socket.h"

#define LED_PIN 25
#define POWER_PIN 15

// millis function from arduino
#define millis() to_ms_since_boot(get_absolute_time())

#define is_debug() strcmp(PICO_CMAKE_BUILD_TYPE, "Debug") == 0

enum LedMode
{
    OFF,
    BLINK,
    FADE,
    RAINBOW
};
struct Settings
{
    // this can not be bigger than TCPPacket.body size!
    LedMode mode;
    uint8_t volumn;
    uint8_t brightness;
};
struct RemoteControlPacket : protected TCPPacket
{
    RemoteControlPacket(const Settings &settings) : TCPPacket()
    {
        if (sizeof(Settings) > sizeof(TCPPacket::body))
        {
            printf("Error: Settings size is too big for packet body!\n");
            return;
        }
        memcpy(this->body, &settings, sizeof(Settings));
    }
};

void print_serial()
{
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    printf("Board ID: {");
    for (int len = 0; len < 8; len++)
    {
        printf("0x%02X", id.id[len]);
        if (len < 7)
            printf(", ");
    }
    printf("}\n");
}

void setup()
{
    stdio_init_all();
    // initialize gpio pins for power and LED
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, true);
    gpio_put(POWER_PIN, true);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);
    gpio_put(LED_PIN, true);
    if (is_debug())
    {
        int tries = 0;
        while (!stdio_usb_connected())
        {
            sleep_ms(1);
            if (tries++ > 2000)
            {
                break;
            }
        }
        sleep_ms(1000);
    }
}

void core0()
{
    // initialize display
    ST7789 display(ST7789_SPI_PORT, ST7789_SPI_BAUDRATE, ST7789_PIN_SCK, ST7789_PIN_SDA, ST7789_PIN_DC, ST7789_PIN_RES);
    display.init();

    // initialize matrix keypad
    uint8_t rows[] = {ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN};
    uint8_t cols[] = {COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN};
    Matrix keypad(rows, cols);
    // Matrix keypad({ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN}, {COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN});

    auto start_time = millis();
    while (true)
    {
        auto current_time = millis();
        if (current_time - start_time > 9000 && !is_debug())
        {
            break;
        }
        char display_text[] = "0";
        display_text[0] += (uint8_t)(9 - (current_time - start_time) / 100);
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
        sleep_ms(10);
    }
    gpio_put(POWER_PIN, false); // turn off
}

int main()
{
    setup();
    print_serial();
    core0();
}
