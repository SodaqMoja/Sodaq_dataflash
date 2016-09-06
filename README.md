This is an Arduino library for AT45DB dataflash.

The code was developed to be used for the Sodaq Moja
board.  But it is general purpose, so it can be used
in other environments too.

Should work for AT45DB011D, AT45DB021D, AT45DB041D,
AT45DB081D, AT45DB161D, AT45DB321D and AT45DB642D.
The library detects the chip by default, but can be
disabled by the init() function.

It is possible to #define the chip in your code
(detection disabled), rather then in the header
file, but it might be tricky to get it work with
Arduino IDE. If you disable the chip-detection, you
may want to check Sodaq_dataflash.h and verify that
the correct variant of the AT45DB is selected.

Use uint8_t df_page_addr_bits(), uint16_t df_page_size()
and uint8_t df_page_bits() rather then
 #define DF_PAGE_ADDR_BITS, #define DF_PAGE_SIZE and
 #define DF_PAGE_BITS in your code, when detection is
enabled!

Sodaq Moja has an on board AT45DB161D.  An earlier
development board Sodaq V2 has an AT45DB081D.
