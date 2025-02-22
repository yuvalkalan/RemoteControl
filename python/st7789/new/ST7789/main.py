from time import sleep_ms
import machine  # type: ignore
import struct


ST7789_WIDTH = 240
ST7789_HEIGHT = 240
ST7789_BAUDRATE = 15 * 10 ** 6

ST7789_SCK = 10
ST7789_SDA = 11
ST7789_RES = 12
ST7789_DC = 13

# ST7789 commands
_ST7789_NOP = b'\x00'           # No operation
_ST7789_SWRESET = b'\x01'       # Software reset
_ST7789_RDDID = b'\x04'         # Read display ID
_ST7789_RDDST = b'\x09'         # Read display status
_ST7789_RDDPM = b'\x0A'         # Read display power
_ST7789_RDDMADCTL = b'\x0B'     # Read display
_ST7789_RDDCOLMOD = b'\x0C'     # Read display pixel
_ST7789_RDDIM = b'\x0D'         # Read display image
_ST7789_RDDSM = b'\x0E'         # Read display signal
_ST7789_RDDSDR = b'\x0F'        # Read display self-diagnostic result
_ST7789_SLPIN = b'\x10'         # Sleep in
_ST7789_SLPOUT = b'\x11'        # Sleep out
_ST7789_PTLON = b'\x12'         # Partial mode on
_ST7789_NORON = b'\x13'         # Partial off (Normal)
_ST7789_INVOFF = b'\x20'        # Display inversion off
_ST7789_INVON = b'\x21'         # Display inversion on
_ST7789_GAMSET = b'\x26'        # Display inversion on
_ST7789_DISPOFF = b'\x28'       # Display off
_ST7789_DISPON = b'\x29'        # Display on
_ST7789_CASET = b'\x2a'         # Column address set
_ST7789_RASET = b'\x2b'         # Row address set
_ST7789_RAMWR = b'\x2c'         # Memory write
_ST7789_RAMRD = b'\x2e'         # Memory read
_ST7789_PTLAR = b'\x30'         # Partial sart/end address set
_ST7789_VSCRDEF = b'\x33'       # Vertical scrolling definition
_ST7789_TEOFF = b'\x34'         # Tearing effect line off
_ST7789_TEON = b'\x35'          # Tearing effect line on
_ST7789_MADCTL = b'\x36'        # Memory data access control
_ST7789_VSCRSADD = b'\x37'      # Vertical scrolling start address
_ST7789_IDMOFF = b'\x38'        # Idle mode off
_ST7789_IDMON = b'\x39'         # Idle mode on
_ST7789_COLMOD = b'\x3a'        # Interface pixel format
_ST7789_RAMWRC = b'\x3c'        # Memory write continue
_ST7789_RAMRDC = b'\x3e'        # Memory read continue
_ST7789_TESCAN = b'\x44'        # Set tear scanline
_ST7789_RDTESCAN = b'\x45'      # Get scanline
_ST7789_WRDISBV = b'\x51'       # Write display brightness
_ST7789_RDDISBV = b'\x52'       # Read display brightness value
_ST7789_WRCTRLD = b'\x53'       # Write CTRL display
_ST7789_RDCTRLD = b'\x54'       # Read CTRL value dsiplay 
_ST7789_WRCACE = b'\x55'        # Write content adaptive brightness control and Color enhancemnet
_ST7789_RDCABC = b'\x56'        # Read content adaptive brightness control
_ST7789_WRCABCMB = b'\x5e'      # Write CABC minimum brightness
_ST7789_RDCABCMB = b'\x5f'      # Read CABC minimum brightness
_ST7789_RDABCSDR = b'\x68'      # Read Automatic Brightness Control Self-Diagnostic Result
_ST7789_RAMCTRL = b'\xB0'       # RAM Control
_ST7789_RGBCTRL = b'\xB1'       # RGB Control
_ST7789_PORCTRL = b'\xB2'       # Porch control
_ST7789_FRCTRL1 = b'\xB3'       # Frame Rate Control 1
_ST7789_PARCTRL = b'\xB5'       # Partial control
_ST7789_GCTRL = b'\xB7'         # Gate control
_ST7789_GTADJ = b'\xB8'         # Gate on timing adjustment
_ST7789_DGMEN = b'\xBA'         # Digital Gamma Enable
_ST7789_VCOMS = b'\xBB'         # VCOM Setting
_ST7789_POWSAVE = b'\xBC'       # Power saving mode
_ST7789_DLPOFFSAVE = b'\xBD'    # Display off power save
_ST7789_LCMCTRL = b'\xC0'       # LCM Control
_ST7789_IDSET = b'\xC1'         # ID Setting
_ST7789_VDVVRHEN = b'\xC2'      # VDV and VRH Command Enable
_ST7789_VRHS = b'\xC3'          # VRH Set
_ST7789_VDVSET = b'\xC4'        # VDV Setting
_ST7789_VCMOFSET = b'\xC5'      # VCOM Offset Set
_ST7789_FRCTR2 = b'\xC6'        # FR Control 2
_ST7789_CABCCTRL = b'\xC7'      # CABC Control
_ST7789_REGSEL1 = b'\xC8'       # Register value selection1
_ST7789_REGSEL2 = b'\xCA'       # Register value selection2
_ST7789_PWMFRSEL = b'\xCC'      # PWM Frequency Selection
_ST7789_PWCTRL1 = b'\xD0'       # Power Control 1
_ST7789_VAPVANEN = b'\xD2'      # Enable VAP/VAN signal output
_ST7789_RDID1 = b'\xda'         # Read ID1
_ST7789_RDID2 = b'\xdb'         # Read ID2
_ST7789_RDID3 = b'\xdc'         # Read ID3
_ST7789_CMD2EN = b'\xDF'        # Command 2 Enable
_ST7789_PVGAMCTRL = b'\xE0'     # Positive Voltage Gamma Control
_ST7789_NVGAMCTRL = b'\xE1'     # Negative Voltage Gamma Control
_ST7789_DGMLUTR = b'\xE2'       # Digital Gamma Look-up Table for Red
_ST7789_DGMLUTB = b'\xE3'       # Digital Gamma Look-up Table for Blue
_ST7789_GATECTRL = b'\xE4'      # Gate control
_ST7789_SPI2EN = b'\xE7'        # SPI2 enable
_ST7789_PWCTRL2 = b'\xE8'       # Power Control 2
_ST7789_EQCTRL = b'\xE9'        # Equalize Time Control
_ST7789_PROMCTRL = b'\xEC'      # Program Control
_ST7789_PROMEN = b'\xFA'        # Program Mode Enable
_ST7789_NVMSET = b'\xFC'        # NVM Setting
_ST7789_PROMACT = b'\xFE'       # Program Action 

# Color definitions
COLOR_BLACK = 0x0000
COLOR_BLUE = 0x001F
COLOR_RED = 0xF800
COLOR_GREEN = 0x07E0
COLOR_CYAN = 0x07FF
COLOR_MAGENTA = 0xF81F
COLOR_YELLOW = 0xFFE0
COLOR_WHITE = 0xFFFF


_ENCODE_POS = ">HH"
_ENCODE_PIXEL = ">H"
_BUFFER_SIZE = 256

# init tuple format (b'command', b'data', delay_ms)
_ST7789_INIT_CMDS = (
    (_ST7789_SLPOUT, None, 120),                                                         # Exit sleep mode
    (_ST7789_NORON, None, 10),                                                            # Turn on the display
    (_ST7789_COLMOD, b'\x55', 0),                                                          # Set pixel format to 16 bits per pixel (RGB565)
    (_ST7789_PORCTRL, b'\x0c\x0c\x00\x33\x33', 0),                                          # Set porch control
    (_ST7789_GCTRL, b'\x35', 0),                                                            # Set gate control
    (_ST7789_VCOMS, b'\x28', 0),                                                            # Set VCOMS setting
    (_ST7789_LCMCTRL, b'\x0c', 0),                                                          # Set power control 1
    (_ST7789_VDVVRHEN, b'\x01\xff', 0),                                                     # Set power control 2
    (_ST7789_VRHS, b'\x10', 0),                                                             # Set power control 3
    (_ST7789_VDVSET, b'\x20', 0),                                                           # Set power control 4
    (_ST7789_FRCTR2, b'\x0f', 0),                                                           # Set VCOM control 1
    (_ST7789_PWCTRL1, b'\xa4\xa1', 0),                                                      # Set power control A
    (_ST7789_PVGAMCTRL, b'\xd0\x00\x02\x07\x0a\x28\x32\x44\x42\x06\x0e\x12\x14\x17', 0),    # Set gamma curve positive polarity
    (_ST7789_NVGAMCTRL, b'\xd0\x00\x02\x07\x0a\x28\x31\x54\x47\x0e\x1c\x17\x1b\x1e', 0),    # Set gamma curve negative polarity
    (_ST7789_INVON, None, 0),                                                            # Enable display inversion
    (_ST7789_DISPON, None, 120)                                                          # Turn on the display
)


class ST7789:
    def __init__(self, sck_pin, sda_pin, res_pin, dc_pin):
        self._sck = sck_pin
        self._sda = sda_pin
        self._res = machine.Pin(res_pin)
        self._dc = machine.Pin(dc_pin)
        self._spi = machine.SPI(1, baudrate=ST7789_BAUDRATE, polarity=1)
        self.data_buffer = bytearray(ST7789_WIDTH*ST7789_HEIGHT*2)
        self.hard_reset()

    def _write(self, command=None, data=None):
        if command is not None:
            self._dc.off()
            self._spi.write(command)
        if data is not None:
            self._dc.on()
            self._spi.write(data)

    def _set_window(self, x0, y0, x1, y1):
        if x0 <= x1 <= ST7789_WIDTH and y0 <= y1 <= ST7789_HEIGHT:
            self._write(
                _ST7789_CASET,
                struct.pack(_ENCODE_POS, x0, x1),
            )
            self._write(
                _ST7789_RASET,
                struct.pack(_ENCODE_POS, y0, y1),
            )
            self._write(_ST7789_RAMWR)

    def soft_reset(self):
        self._write(_ST7789_SWRESET)
        sleep_ms(150)

    def hard_reset(self):
        self._res.on()
        sleep_ms(10)
        self._res.off()
        sleep_ms(10)
        self._res.on()
        sleep_ms(120)

    def fill_rect(self, x, y, width, height, color):
        self._set_window(x, y, x + width - 1, y + height - 1)
        chunks, rest = divmod(width * height, _BUFFER_SIZE)
        pixel = struct.pack(_ENCODE_PIXEL, color)
        self._dc.on()
        if chunks:
            data = pixel * _BUFFER_SIZE
            for _ in range(chunks):
                self._write(None, data)
        if rest:
            self._write(None, pixel * rest)

    def fill(self, color):
        self.fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color)

    def init(self):
        """
        Initialize display.
        """
        for command, data, delay in _ST7789_INIT_CMDS:
            print(command, data, delay)
            self._write(command, data)
            sleep_ms(delay)

    def draw(self):
        self._set_window(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1)
        self._dc.on()
        self._spi.write(self.data_buffer)

    def draw_pixel(self, x, y, color):
        self.data_buffer[2*(x + y * ST7789_WIDTH)] = color % 256
        self.data_buffer[2*(x + y * ST7789_WIDTH) + 1] = color // 256


def main():
    display = ST7789(ST7789_SCK, ST7789_SDA, ST7789_RES, ST7789_DC)
    display.init()
    # display.fill(COLOR_RED)
    # display.data_buffer[100] = 0xff
    # display.draw()
    led = machine.Pin(25, machine.Pin.OUT)
    k = 0
    while True:
        led.toggle()
        sleep_ms(500)
        # display.draw_pixel(k, k, COLOR_RED)
        # k += 1
        # # display.draw_pixel(k, k, COLOR_GREEN)
        # # k += 1
        # # display.draw_pixel(k, k, COLOR_BLUE)
        # # k += 1
        # # display.draw()
        # # for i in range(len(display.data_buffer)):
        # #     display.data_buffer[i] = 0x00
        # display.draw()
            


if __name__ == "__main__":
    main()
