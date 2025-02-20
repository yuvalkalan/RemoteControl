from time import sleep_ms
import machine
import struct



ST7789_WIDTH = 240
ST7789_HEIGHT = 240
ST7789_BAUDRATE = 4 * 10 ** 6

ST7789_SCK = 10
ST7789_SDA = 11
ST7789_RES = 12
ST7789_DC  = 13

# ST7789 commands
_ST7789_SWRESET = b"\x01"
_ST7789_SLPIN = b"\x10"
_ST7789_SLPOUT = b"\x11"
_ST7789_NORON = b"\x13"
_ST7789_INVOFF = b"\x20"
_ST7789_INVON = b"\x21"
_ST7789_DISPOFF = b"\x28"
_ST7789_DISPON = b"\x29"
_ST7789_CASET = b"\x2a"
_ST7789_RASET = b"\x2b"
_ST7789_RAMWR = b"\x2c"
_ST7789_VSCRDEF = b"\x33"
_ST7789_COLMOD = b"\x3a"
_ST7789_MADCTL = b"\x36"
_ST7789_VSCSAD = b"\x37"
_ST7789_RAMCTL = b"\xb0"

const = lambda x: x

_ENCODE_POS = const(">HH")
_ENCODE_PIXEL = const(">H")
_BUFFER_SIZE = const(256)


class ST7789:
    def __init__(self, sck_pin, sda_pin, res_pin, dc_pin):
        self._sck = sck_pin
        self._sda = sda_pin
        self._res = machine.Pin(res_pin)
        self._dc = machine.Pin(dc_pin)
        self._spi = machine.SPI(1, baudrate=ST7789_BAUDRATE, polarity=1)
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
        """
        Soft reset display.
        """
        self._write(_ST7789_SWRESET)
        sleep_ms(150)

    def hard_reset(self):
        """
        Hard reset display.
        """
        self._res.on()
        sleep_ms(10)
        self._res.off()
        sleep_ms(10)
        self._res.on()
        sleep_ms(120)

    def fill_rect(self, x, y, width, height, color):
        """
        Draw a rectangle at the given location, size and filled with color.

        Args:
            x (int): Top left corner x coordinate
            y (int): Top left corner y coordinate
            width (int): Width in pixels
            height (int): Height in pixels
            color (int): 565 encoded color
        """
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
        """
        Fill the entire FrameBuffer with the specified color.

        Args:
            color (int): 565 encoded color
        """
        self.fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color)


def main():
    display = ST7789(ST7789_SCK, ST7789_SDA, ST7789_RES, ST7789_DC)
    display.fill(2**5)


if __name__ == "__main__":
    main()

