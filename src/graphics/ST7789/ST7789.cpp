#include "ST7789.h"

static color_t convert_color(color_t color)
{
    // convert color from rgb565 format to rbg565 format
    return (color >> 8) | (color << 8);
}

ST7789::ST7789(spi_inst_t *spi, uint baudrate, uint sck_pin, uint sda_pin, uint dc_pin, uint res_pin)
    : m_spi(spi),
      m_baudrate(baudrate),
      m_sck_pin(sck_pin),
      m_sda_pin(sda_pin),
      m_dc_pin(dc_pin),
      m_res_pin(res_pin),
      m_dma_channel(dma_claim_unused_channel(true)),
      m_dma_config(dma_channel_get_default_config(m_dma_channel)),
      m_dma_is_active(false)
{
    // Configure the control pins
    gpio_init(m_dc_pin);
    gpio_set_dir(m_dc_pin, GPIO_OUT);
    gpio_init(m_res_pin);
    gpio_set_dir(m_res_pin, GPIO_OUT);
    hard_reset();
    configure_dma();
}

void ST7789::write_command(uint8_t cmd) const
{
    // Low-level function to send commands
    gpio_put(m_dc_pin, 0); // DC low for command
    // gpio_put(m_cs_pin, 0); // CS low
    spi_write_blocking(m_spi, &cmd, 1);
    // gpio_put(m_cs_pin, 1); // CS high
}
void ST7789::write_data(uint8_t data) const
{
    // Low-level function to send data
    gpio_put(m_dc_pin, 1); // DC high for data
    // gpio_put(m_cs_pin, 0); // CS low
    spi_write_blocking(m_spi, &data, 1);
    // gpio_put(m_cs_pin, 1); // CS high
}
void ST7789::write_data_buffer(const uint8_t *buffer, size_t size) const
{
    // Write multiple data bytes
    gpio_put(m_dc_pin, 1); // DC high for data
    // gpio_put(m_cs_pin, 0); // CS low
    spi_write_blocking(m_spi, buffer, size);
    // gpio_put(m_cs_pin, 1); // CS high
}

void ST7789::set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) const
{
    // Set the address window for pixel updates
    // Column address set
    write_command(ST7789_CASET);
    uint16_t column_data[] = {x0, x1};
    write_data_buffer((uint8_t *)column_data, sizeof(column_data) * 2);
    // Row address set
    write_command(ST7789_RASET);
    uint16_t row_data[] = {y0, y1};
    write_data_buffer((uint8_t *)row_data, sizeof(row_data) * 2);
    // Write to RAM
    write_command(ST7789_RAMWR);
}
void ST7789::configure_dma()
{
    // Configure the DMA channel
    channel_config_set_transfer_data_size(&m_dma_config, DMA_SIZE_8);                           // 16-bit transfer, every pixel is 16 bits
    channel_config_set_read_increment(&m_dma_config, true);                                     // Increment source
    channel_config_set_write_increment(&m_dma_config, false);                                   // Fixed destination
    channel_config_set_dreq(&m_dma_config, spi_get_index(m_spi) ? DREQ_SPI1_TX : DREQ_SPI0_TX); // SPI TX DREQ
}
void ST7789::draw_pixel(int x, int y, color_t color)
{
    // check if the pixel is within the screen
    if (x < 0 || x >= ST7789_WIDTH || y < 0 || y >= ST7789_HEIGHT)
        return;
    // Draw a pixel at the specified coordinates
    m_buffer[x + y * ST7789_WIDTH] = convert_color(color);
}

#define CHUNK_SIZE (ST7789_WIDTH * ST7789_HEIGHT)

void ST7789::update()
{
    // TODO: use the dma to update the screen in the background
    // static int counter = 0;
    // color_t colors[] = {COLOR_565_RED, COLOR_565_GREEN, COLOR_565_BLUE, COLOR_565_CYAN, COLOR_565_MAGENTA, COLOR_565_YELLOW};
    // fill(colors[counter++ % 6]);

    set_addr_window(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    uint8_t *current_ptr = (uint8_t *)m_buffer;
    int remaining_bytes = sizeof(m_buffer);
    gpio_put(m_dc_pin, 1); // DC high for data
    while (remaining_bytes > 0)
    {
        // printf("Remaining bytes: %d\n", remaining_bytes);
        uint32_t transfer_size = remaining_bytes > CHUNK_SIZE ? CHUNK_SIZE : remaining_bytes;

        // Configure the DMA channel
        dma_channel_configure(
            m_dma_channel,                    // DMA channel
            &m_dma_config,                    // Channel config
            &spi_get_hw(ST7789_SPI_PORT)->dr, // Destination (SPI TX FIFO)
            current_ptr,                      // Source (memory buffer)
            transfer_size,                    // Number of transfers (every transfer is 4 bytes)
            true                              // Start immediately
        );
        m_dma_is_active = true;

        // Wait for the current DMA transfer to complete
        dma_channel_wait_for_finish_blocking(m_dma_channel);
        // Update pointer and remaining bytes
        current_ptr += transfer_size;
        remaining_bytes -= transfer_size;
    }

    // // Configure the DMA channel
    // printf("Configuring DMA\n");
    // gpio_put(m_dc_pin, 1); // DC high for data
    // dma_channel_configure(
    //     m_dma_channel,                    // DMA channel
    //     &m_dma_config,                    // Channel config
    //     &spi_get_hw(ST7789_SPI_PORT)->dr, // Destination (SPI TX FIFO)
    //     m_buffer,                         // Source (memory buffer)
    //     sizeof(m_buffer) / 2,             // Number of bytes to transfer (one pixel is 2 bytes)
    //     true                              // Start immediately
    // );
    // printf("Waiting for DMA to finish\n");
    // dma_channel_wait_for_finish_blocking(m_dma_channel);
    // dma_is_active = false;
    // write_data_buffer((uint8_t *)m_buffer, sizeof(m_buffer));
}

void ST7789::fill(color_t color)
{
    // Fill the entire screen with a color
    std::fill(m_buffer, m_buffer + ST7789_WIDTH * ST7789_HEIGHT, convert_color(color));
}
void ST7789::draw_char(int x, int y, char c, color_t color, uint8_t scale)
{
    // Ensure the character is within the bounds of the font array
    if (c < 32 || c > 126)
        return; // ASCII range check

    const uint16_t *bitmap = font_bitmap[c - 32]; // Get the bitmap for the character
    for (int i = 0; i < FONT_CHAR_HEIGHT; i++)
    { // Loop through the height of the character
        uint16_t row = bitmap[i];
        for (int j = 0; j < FONT_CHAR_WIDTH; j++)
        { // Loop through the width of the character
            if (row & (1 << j))
            {
                // printf("X");
                for (int sx = 0; sx < scale; sx++) // Draw scaled pixel
                    for (int sy = 0; sy < scale; sy++)
                        draw_pixel(x + (j * scale) + sx, y + (i * scale) + sy, color);
            }
        }
    }
}
void ST7789::draw_text(int x, int y, const std::string &text, uint16_t color, uint8_t scale)
{
    uint8_t ori_x = x;
    for (size_t i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            x = ori_x;
            y += (FONT_CHAR_HEIGHT + 1) * scale;
        }
        else
        {
            draw_char(x, y, text[i], color, scale);
            x += (FONT_CHAR_WIDTH + 1) * scale; // Move x cursor, 5 pixels for the character + 1 pixel space
        }
    }
}
void ST7789::hard_reset()
{
    gpio_put(m_res_pin, 1);
    sleep_ms(10);
    gpio_put(m_res_pin, 0);
    sleep_ms(10);
    gpio_put(m_res_pin, 1);
    sleep_ms(120);
}

void ST7789::init()
{
    // Initialize chosen SPI port
    spi_init(m_spi, m_baudrate);
    gpio_set_function(m_sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(m_sda_pin, GPIO_FUNC_SPI);
    // gpio_put(m_cs_pin, true);    // Deselect
    spi_set_format(m_spi, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);

    set_sleep(false);            // Exit sleep mode
    write_command(ST7789_NORON); // Enter normal mode
    sleep_ms(10);
    // Set pixel format to 16 bits per pixel (RGB565)
    write_command(ST7789_COLMOD);
    write_data(0x55);
    // Set porch control
    write_command(ST7789_PORCTRL);
    write_data_buffer(ST7789_PORCTRL_VALUE, sizeof(ST7789_PORCTRL_VALUE));
    // Set gate control
    write_command(ST7789_GCTRL);
    write_data(0x35);
    // Set VCOMS setting
    write_command(ST7789_VCOMS);
    write_data(0x28);
    // Set power control 1
    write_command(ST7789_LCMCTRL);
    write_data(0x0c);
    // Set power control 2
    write_command(ST7789_VDVVRHEN);
    write_data_buffer(ST7789_VDVVRHEN_VALUE, sizeof(ST7789_VDVVRHEN_VALUE));
    // Set power control 3
    write_command(ST7789_VRHS);
    write_data(0x10);
    // Set power control 4
    write_command(ST7789_VDVSET);
    write_data(0x20);
    // Set VCOM control 1
    write_command(ST7789_FRCTR2);
    write_data(0x0f);
    // Set power control 1
    write_command(ST7789_PWCTRL1);
    write_data_buffer(ST7789_PWCTRL1_VALUE, sizeof(ST7789_PWCTRL1_VALUE));
    // Set gamma curve positive polarity
    write_command(ST7789_PVGAMCTRL);
    write_data_buffer(ST7789_PVGAMCTRL_VALUE, sizeof(ST7789_PVGAMCTRL_VALUE));
    // Set gamma curve negative polarity
    write_command(ST7789_NVGAMCTRL);
    write_data_buffer(ST7789_NVGAMCTRL_VALUE, sizeof(ST7789_NVGAMCTRL_VALUE));
    // Enable display inversion
    write_command(ST7789_INVON);
    // clear the display
    fill(COLOR_565_BLACK);
    update();
    // Turn on the display
    turn_on(true);
}

void ST7789::turn_on(bool on)
{
    write_command(on ? ST7789_DISPON : ST7789_DISPOFF);
    write_data(0x00);
    sleep_ms(120);
}
void ST7789::set_sleep(bool mode)
{
    write_command(mode ? ST7789_SLPIN : ST7789_SLPOUT);
    write_data(0x00);
    sleep_ms(120);
}