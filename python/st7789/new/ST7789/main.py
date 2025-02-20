from time import sleep_ms
import machine  # type: ignore
import struct


ST7789_WIDTH = 240
ST7789_HEIGHT = 240
ST7789_BAUDRATE = 4 * 10 ** 6

ST7789_SCK = 10
ST7789_SDA = 11
ST7789_RES = 12
ST7789_DC = 13

# ST7789 commands
_ST7789_SWRESET = b'\x01'
_ST7789_SLPIN = b'\x10'
_ST7789_SLPOUT = b'\x11'
_ST7789_NORON = b'\x13'
_ST7789_INVOFF = b'\x20'
_ST7789_INVON = b'\x21'
_ST7789_DISPOFF = b'\x28'
_ST7789_DISPON = b'\x29'
_ST7789_CASET = b'\x2a'
_ST7789_RASET = b'\x2b'
_ST7789_RAMWR = b'\x2c'
_ST7789_VSCRDEF = b'\x33'
_ST7789_COLMOD = b'\x3a'
_ST7789_MADCTL = b'\x36'
_ST7789_VSCSAD = b'\x37'
_ST7789_RAMCTL = b'\xb0'


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
    (_ST7789_SLPOUT, b'\x00', 120),               # Exit sleep mode
    (_ST7789_NORON, b'\x00', 0),                 # Turn on the display
    (b'\xb6', b'\x0a\x82', 0),             # Set display function control
    # Set pixel format to 16 bits per pixel (RGB565)
    (_ST7789_COLMOD, b'\x55', 10),
    (b'\xb2', b'\x0c\x0c\x00\x33\x33', 0),  # Set porch control
    (b'\xb7', b'\x35', 0),                 # Set gate control
    (b'\xbb', b'\x28', 0),                 # Set VCOMS setting
    (b'\xc0', b'\x0c', 0),                 # Set power control 1
    (b'\xc2', b'\x01\xff', 0),             # Set power control 2
    (b'\xc3', b'\x10', 0),                 # Set power control 3
    (b'\xc4', b'\x20', 0),                 # Set power control 4
    (b'\xc6', b'\x0f', 0),                 # Set VCOM control 1
    (b'\xd0', b'\xa4\xa1', 0),             # Set power control A
    # Set gamma curve positive polarity
    (b'\xe0', b'\xd0\x00\x02\x07\x0a\x28\x32\x44\x42\x06\x0e\x12\x14\x17', 0),
    # Set gamma curve negative polarity
    (b'\xe1', b'\xd0\x00\x02\x07\x0a\x28\x31\x54\x47\x0e\x1c\x17\x1b\x1e', 0),
    (_ST7789_INVON, b'\x00', 0),                 # Enable display inversion
    (_ST7789_DISPON, b'\x00', 120)                # Turn on the display
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
    display.draw()
    led = machine.Pin(25, machine.Pin.OUT)
    k = 0
    while True:
        led.toggle()
        # sleep_ms(500)
        display.draw_pixel(k, k, COLOR_RED)
        k += 1
        display.draw_pixel(k, k, COLOR_GREEN)
        k += 1
        display.draw_pixel(k, k, COLOR_BLUE)
        k += 1
        display.draw()


if __name__ == "__main__":
    main()
