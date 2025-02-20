#include "pico/stdlib.h"
#include "hardware/spi.h"

int main()
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, true);
    while (true)
    {
        gpio_put(25, !gpio_get(25));
        sleep_ms(1000);
    }

    return 0;
}
