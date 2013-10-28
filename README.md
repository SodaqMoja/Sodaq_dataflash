This is an Arduino library for AT45DB dataflash.

The code was developed to be used for the Sodaq Moja
board.  But it is general purpose, so it can be used
in other environments too.

Sodaq Moja has an on board AT45DB161D.  An earlier
development board Sodaq V2 has an AT45DB081D.  You may
want to check Sodaq_dataflash.h and verify that the
correct variant of the AT45DB is selected.  Perhaps
someday we can detect this dynamically.
