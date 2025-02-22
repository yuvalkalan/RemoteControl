#pragma once
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <algorithm>

// display size
#define ST7789_WIDTH 240
#define ST7789_HEIGHT 240

// display configurations
#define ST7789_PIN_SCK 10
#define ST7789_PIN_SDA 11
#define ST7789_PIN_RES 12
#define ST7789_PIN_DC 13
#define ST7789_SPI_PORT spi1
#define ST7789_SPI_BAUDRATE 32000000 // 32MHZ

// ST7789 commands
#define ST7789_NOP 0x00        // No operation
#define ST7789_SWRESET 0x01    // Software reset
#define ST7789_RDDID 0x04      // Read display ID
#define ST7789_RDDST 0x09      // Read display status
#define ST7789_RDDPM 0x0A      // Read display power
#define ST7789_RDDMADCTL 0x0B  // Read display
#define ST7789_RDDCOLMOD 0x0C  // Read display pixel
#define ST7789_RDDIM 0x0D      // Read display image
#define ST7789_RDDSM 0x0E      // Read display signal
#define ST7789_RDDSDR 0x0F     // Read display self-diagnostic result
#define ST7789_SLPIN 0x10      // Sleep in
#define ST7789_SLPOUT 0x11     // Sleep out
#define ST7789_PTLON 0x12      // Partial mode on
#define ST7789_NORON 0x13      // Partial off (Normal)
#define ST7789_INVOFF 0x20     // Display inversion off
#define ST7789_INVON 0x21      // Display inversion on
#define ST7789_GAMSET 0x26     // Display inversion on
#define ST7789_DISPOFF 0x28    // Display off
#define ST7789_DISPON 0x29     // Display on
#define ST7789_CASET 0x2A      // Column address set
#define ST7789_RASET 0x2B      // Row address set
#define ST7789_RAMWR 0x2C      // Memory write
#define ST7789_RAMRD 0x2E      // Memory read
#define ST7789_PTLAR 0x30      // Partial sart/end address set
#define ST7789_VSCRDEF 0x33    // Vertical scrolling definition
#define ST7789_TEOFF 0x34      // Tearing effect line off
#define ST7789_TEON 0x35       // Tearing effect line on
#define ST7789_MADCTL 0x36     // Memory data access control
#define ST7789_VSCRSADD 0x37   // Vertical scrolling start address
#define ST7789_IDMOFF 0x38     // Idle mode off
#define ST7789_IDMON 0x39      // Idle mode on
#define ST7789_COLMOD 0x3A     // Interface pixel format
#define ST7789_RAMWRC 0x3C     // Memory write continue
#define ST7789_RAMRDC 0x3E     // Memory read continue
#define ST7789_TESCAN 0x44     // Set tear scanline
#define ST7789_RDTESCAN 0x45   // Get scanline
#define ST7789_WRDISBV 0x51    // Write display brightness
#define ST7789_RDDISBV 0x52    // Read display brightness value
#define ST7789_WRCTRLD 0x53    // Write CTRL display
#define ST7789_RDCTRLD 0x54    // Read CTRL value dsiplay
#define ST7789_WRCACE 0x55     // Write content adaptive brightness control and Color enhancemnet
#define ST7789_RDCABC 0x56     // Read content adaptive brightness control
#define ST7789_WRCABCMB 0x5E   // Write CABC minimum brightness
#define ST7789_RDCABCMB 0x5F   // Read CABC minimum brightness
#define ST7789_RDABCSDR 0x68   // Read Automatic Brightness Control Self-Diagnostic Result
#define ST7789_RAMCTRL 0xB0    // RAM Control
#define ST7789_RGBCTRL 0xB1    // RGB Control
#define ST7789_PORCTRL 0xB2    // Porch control
#define ST7789_FRCTRL1 0xB3    // Frame Rate Control 1
#define ST7789_PARCTRL 0xB5    // Partial control
#define ST7789_GCTRL 0xB7      // Gate control
#define ST7789_GTADJ 0xB8      // Gate on timing adjustment
#define ST7789_DGMEN 0xBA      // Digital Gamma Enable
#define ST7789_VCOMS 0xBB      // VCOM Setting
#define ST7789_POWSAVE 0xBC    // Power saving mode
#define ST7789_DLPOFFSAVE 0xBD // Display off power save
#define ST7789_LCMCTRL 0xC0    // LCM Control
#define ST7789_IDSET 0xC1      // ID Setting
#define ST7789_VDVVRHEN 0xC2   // VDV and VRH Command Enable
#define ST7789_VRHS 0xC3       // VRH Set
#define ST7789_VDVSET 0xC4     // VDV Setting
#define ST7789_VCMOFSET 0xC5   // VCOM Offset Set
#define ST7789_FRCTR2 0xC6     // FR Control 2
#define ST7789_CABCCTRL 0xC7   // CABC Control
#define ST7789_REGSEL1 0xC8    // Register value selection1
#define ST7789_REGSEL2 0xCA    // Register value selection2
#define ST7789_PWMFRSEL 0xCC   // PWM Frequency Selection
#define ST7789_PWCTRL1 0xD0    // Power Control 1
#define ST7789_VAPVANEN 0xD2   // Enable VAP/VAN signal output
#define ST7789_RDID1 0xDA      // Read ID1
#define ST7789_RDID2 0xDB      // Read ID2
#define ST7789_RDID3 0xDC      // Read ID3
#define ST7789_CMD2EN 0xDF     // Command 2 Enable
#define ST7789_PVGAMCTRL 0xE0  // Positive Voltage Gamma Control
#define ST7789_NVGAMCTRL 0xE1  // Negative Voltage Gamma Control
#define ST7789_DGMLUTR 0xE2    // Digital Gamma Look-up Table for Red
#define ST7789_DGMLUTB 0xE3    // Digital Gamma Look-up Table for Blue
#define ST7789_GATECTRL 0xE4   // Gate control
#define ST7789_SPI2EN 0xE7     // SPI2 enable
#define ST7789_PWCTRL2 0xE8    // Power Control 2
#define ST7789_EQCTRL 0xE9     // Equalize Time Control
#define ST7789_PROMCTRL 0xEC   // Program Control
#define ST7789_PROMEN 0xFA     // Program Mode Enable
#define ST7789_NVMSET 0xFC     // NVM Setting
#define ST7789_PROMACT 0xFE    // Program Action

// configurations values
static uint8_t ST7789_PORCTRL_VALUE[] = {0x0c, 0x0c, 0x00, 0x33, 0x33};
static uint8_t ST7789_VDVVRHEN_VALUE[] = {0x01, 0xff};
static uint8_t ST7789_PWCTRL1_VALUE[] = {0xa4, 0xa1};
static uint8_t ST7789_PVGAMCTRL_VALUE[] = {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0e, 0x12, 0x14, 0x17};
static uint8_t ST7789_NVGAMCTRL_VALUE[] = {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47, 0x0e, 0x1c, 0x17, 0x1b, 0x1e};

typedef const uint16_t color_t;
static color_t COLOR_565_BLACK = 0x0000;
static color_t COLOR_565_BLUE = 0xF800;
static color_t COLOR_565_RED = 0x001F;
static color_t COLOR_565_GREEN = 0x07E0;
static color_t COLOR_565_CYAN = 0xFFE0;
static color_t COLOR_565_MAGENTA = 0xF81F;
static color_t COLOR_565_YELLOW = 0x07FF;
static color_t COLOR_565_WHITE = 0xFFFF;

class ST7789
{
private:
    spi_inst_t *m_spi;
    uint m_baudrate;
    uint8_t m_sck_pin;
    uint8_t m_sda_pin;
    uint8_t m_dc_pin;
    uint8_t m_res_pin;
    uint16_t m_buffer[ST7789_WIDTH * ST7789_HEIGHT] = {};

private:
    void write_command(uint8_t cmd) const;
    void write_data(uint8_t data) const;
    void write_data_buffer(const uint8_t *buffer, size_t size) const;
    void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) const;

public:
    ST7789(spi_inst_t *spi, uint baudrate, uint sck_pin, uint sda_pin, uint dc_pin, uint res_pin);
    void hard_reset();
    void init();
    void turn_on(bool on);
    void set_sleep(bool mode);
    void draw_pixel(uint8_t x, uint8_t y, color_t color);
    void update();
    void fill(color_t color);
};
