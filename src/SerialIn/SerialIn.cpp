#include "SerialIn.h"

SerialIn::SerialIn() : m_message(""), m_show_overloading(true)
{
}
void SerialIn::reset_bootsel()
{
    printf("Resetting to bootloader\n");
    reset_usb_boot(0, 0);
}
void SerialIn::reboot()
{
    watchdog_enable(0, 0);
    while (true)
        tight_loop_contents();
}
void SerialIn::update()
{
    // Check to see if anything is available in the serial receive buffer
    while (tud_cdc_available())
    {
        char chr = getchar();
        if (chr == CTRL_C)
            reset_bootsel();
        else if (chr == CTRL_D)
            reboot();
        // printf("char is %c (value=%d)\n", chr, chr);
        m_message += chr;
    }
    if (m_message.length() != 0)
    {
        // process_command(settings);
        m_message = "";
    }
}
