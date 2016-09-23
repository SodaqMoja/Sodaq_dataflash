/*
 * Copyright (c) 2013 Kees Bakker.  All rights reserved.
 *
 * This file is part of Sodaq_dataflash.
 *
 * Sodaq_dataflash is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published bythe Free Software Foundation, either version 3 of
 * the License, or(at your option) any later version.
 *
 * Sodaq_dataflash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Sodaq_dataflash.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/*
 * This library was inspired by dataflash libraries by Atmel, asynclabs,
 * and others.
 */

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>

#include "Sodaq_dataflash.h"

//Dataflash commands
#define FlashPageRead                0xD2     // Main memory page read *)
#define FlashContRead                0x0B     // Continuous Array Read *)
#define StatusReg                    0xD7     // Status register *)
#define ReadMfgID                    0x9F     // Read Manufacturer and Device ID *)
#define PageErase                    0x81     // Page erase *)
#define BlockErase                   0x50     // Block Erase *)
#define SectorErase                  0x7C     // Sector Erase *)
#define ReadSecReg                   0x77     // Read Security Register *)

#define FlashToBuf1Transfer          0x53     // Main memory page to buffer 1 transfer *)
#define Buf1Read                     0xD4     // Buffer 1 read *)
#define Buf1ToFlashWE                0x83     // Buffer 1 to main memory page program with built-in erase *)
#define Buf1Write                    0x84     // Buffer 1 write *)
#define Buf1ToFlashWoE               0x88     // Buffer 1 to Main Memory Page Program without Built-in Erase *)
#define Buf1WriteThenFlashWE         0x82     // Main Memory Page Program Through Buffer 1 *)
#define Buf1FlashComp                0x60     // Main Memory Page to Buffer 1 Compare *)

#define FlashToBuf2Transfer          0x55     // Main memory page to buffer 2 transfer - not used anywhere
#define Buf2Read                     0xD6     // Buffer 2 read                         - not used anywhere
#define Buf2ToFlashWE                0x86     // Buffer 2 to main memory page program with built-in erase
                                              //                                       - not used anywhere
#define Buf2Write                    0x87     // Buffer 2 write                        - not used anywhere

#define DeepPowerDown                0xB9     // Power mode to save current (from 25uA to 15uA) *)
#define DeepPowerUp                  0xAB     // Power-up to idle current (from 15uA to 25uA) *)
// *) Compatibility among all chips checked against datasheets

// Note that AT45DB011D and AT45DB021D have only one buffer. These chips
// use the commands the other chips use for buffer 1.

// Command used but not defined: Chip Erase C7H, 94H, 80H, 9AH

/*
 * Other commands not implemented yet:
 *   Continuous Array Read (Low Frequency) 03H
 *   Buffer 1 Read (Low Frequency) D1H
 *   Buffer 2 Read (Low Frequency) D3H
 *   Buffer 2 to Main Memory Page Program without Built-in Erase 89H
 *   Main Memory Page Program Through Buffer 2 85H
 *   Enable Sector Protection 3DH + 2AH + 7FH + A9H
 *   Disable Sector Protection 3DH + 2AH + 7FH + 9AH
 *   Erase Sector Protection Register 3DH + 2AH + 7FH + CFH
 *   Program Sector Protection Register 3DH + 2AH + 7FH + FCH
 *   Read Sector Protection Register 32H
 *   Sector Lockdown 3DH + 2AH + 7FH + 30H
 *   Read Sector Lockdown Register 35H
 *   Program Security Register 9BH + 00H + 00H + 00H
 *   Main Memory Page to Buffer 2 Compare 61H
 *   Auto Page Rewrite through Buffer 1 58H
 *   Auto Page Rewrite through Buffer 2 59H
 */

//Chip identity values
#define Manufacturer                 0x1F     // First byte from readID()
#define Family_shiftbits                5     // 3 first bits of second byte
#define Family_DF                    0x01     // Value of 3 bits shiftet right equal to 0x01 is DataFlash
#define Density_mask                 0x0F     // 4 last bits of second byte (Fifth bit is not used)
#define Density_1Mb                  0x02     // AT45DB011D
#define Density_2Mb                  0x03     // AT45DB021D
#define Density_4Mb                  0x04     // AT45DB041D
#define Density_8Mb                  0x05     // AT45DB081D
#define Density_16Mb                 0x06     // AT45DB161D
#define Density_32Mb                 0x07     // AT45DB321D
#define Density_64Mb                 0x08     // AT45DB642D
#define Detection_off                0x00     // Return value for df_page_addr_bits(), df_page_size_array(),
                                              // df_page_bits_array(), bytesPerPage(), blocksPerSector(),
					      // sectorsPerChip() and bytesPerChip().

// Translate density to DF_PAGE_ADDR_BITS format
// Density                                                    2    3    4    5    6    7     8
const uint8_t Sodaq_Dataflash::df_page_addr_bits_array[] = {  9,  10,  11,  12,  12,  13,   13};

// Translate density to DF_PAGE_SIZE format
// Density                                                    2    3    4    5    6    7     8
const uint16_t Sodaq_Dataflash::df_page_size_array[] =     {264, 264, 264, 264, 528, 528, 1056};

// Translate density to DF_PAGE_BITS format
// Density                                                    2    3    4    5    6    7     8
const uint8_t Sodaq_Dataflash::df_page_bits_array[] =      {  9,   9,   9,   9,  10,  10,   11};

// Translate density to blocks per sector
// Density                                                    2    3    4    5    6    7     8
const uint8_t Sodaq_Dataflash::blocksPerSector_array[] =   { 16,  16,  32,  32,  32,  16,   32};

// Translate density to total number of sectors
// Density                                                    2    3    4    5    6    7     8
const uint8_t Sodaq_Dataflash::sectorsPerChip_array[] =    {  4,   8,   8,  16,  16,  64,   32};

bool Sodaq_Dataflash::willdetect = true;            // Chip detection true by default
uint8_t Sodaq_Dataflash::dfpagebits = DF_PAGE_BITS; // Initialized for detect == false
uint8_t Sodaq_Dataflash::chip;                      // Density

uint8_t Sodaq_Dataflash::init(uint8_t csPin)
{
  // Setup the slave select pin
  _csPin = csPin;

  // Call the standard SPI initialisation
  SPI.begin();

  // This is used when CS != SS
  pinMode(_csPin, OUTPUT);

  // Do not run chipdetect if detect == false
  if (willdetect == true) {
    if ((chip = chipdetect()) > 0x08) {
      return(chip); // Return not_detected code
    }
    dfpagebits = df_page_bits(); // Lookup page bits in array
  }

#if DF_VARIANT == DF_AT45DB081D
  _pageAddrShift = 1;              // _pageAddrShift set to 1, but not used
#elif DF_VARIANT == DF_AT45DB161D
  _pageAddrShift = 1;
#elif DF_VARIANT == DF_AT45DB041D
  _pageAddrShift = 1;
#endif

return(0); //Chip detected or chipdetect not run
}

uint8_t Sodaq_Dataflash::init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint8_t ssPin)
{
  return (init(ssPin));
}

// Interface to set chip detection to true or false
uint8_t Sodaq_Dataflash::init(uint8_t ssPin, bool detect)
{ 
  willdetect = detect;
  return (init(ssPin));
}


uint8_t Sodaq_Dataflash::transmit(uint8_t data)
{
  // Call the standard SPI transfer method
  return SPI.transfer(data);
}

uint8_t Sodaq_Dataflash::readStatus()
{
  unsigned char result;

  activate();
  result = transmit(StatusReg);
  result = transmit(0x00);
  deactivate();

  return result;
}

// Monitor the status register, wait until busy-flag is high
void Sodaq_Dataflash::waitTillReady()
{
  while (!(readStatus() & 0x80)) {
    // WDT reset maybe??
  }
}

void Sodaq_Dataflash::readID(uint8_t *data)
{
  activate();
  transmit(ReadMfgID);
  data[0] = transmit(0x00);
  data[1] = transmit(0x00);
  data[2] = transmit(0x00);
  data[3] = transmit(0x00);
  deactivate();
}

// Reads a number of bytes from one of the Dataflash security register
void Sodaq_Dataflash::readSecurityReg(uint8_t *data, size_t size)
{
    activate();
    transmit(ReadSecReg);
    transmit(0x00);
    transmit(0x00);
    transmit(0x00);
    for (size_t i = 0; i < size; i++) {
      *data++ = transmit(0x00);
    }
    deactivate();
}

// Transfers a page from flash to Dataflash SRAM buffer
void Sodaq_Dataflash::readPageToBuf1(uint16_t pageAddr)
{
  activate();
  transmit(FlashToBuf1Transfer);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

// Transfers data from flash directly and shortcutting the buffer, not crossing page-borders.
void Sodaq_Dataflash::readStrPage(uint16_t pageAddr, uint16_t addr, uint8_t *data, size_t size)
{
  activate();
  transmit(FlashPageRead);
  setFullAddr(pageAddr, addr);
    transmit(0x00);             //don't care
    transmit(0x00);
    transmit(0x00);
    transmit(0x00);
    for (size_t i = 0; i < size; i++) {
      *data++ = transmit(0x00);
    }
  deactivate();
}

// Transfers data from flash directly and shortcutting the buffer, crossing page-borders.
void Sodaq_Dataflash::readStrCont(uint16_t pageAddr, uint16_t addr, uint8_t *data, size_t size)
{
  activate();
  transmit(FlashContRead);
  setFullAddr(pageAddr, addr);
    transmit(0x00);             //don't care
    for (size_t i = 0; i < size; i++) {
      *data++ = transmit(0x00);
    }
  deactivate();
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t Sodaq_Dataflash::readByteBuf1(uint16_t addr)
{
  unsigned char data = 0;

  activate();
  transmit(Buf1Read);
  transmit(0x00);               //don't care
  transmit((uint8_t) (addr >> 8));
  transmit((uint8_t) (addr));
  transmit(0x00);               //don't care
  data = transmit(0x00);        //read byte
  deactivate();

  return data;
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::readStrBuf1(uint16_t addr, uint8_t *data, size_t size)
{
  activate();
  transmit(Buf1Read);
  transmit(0x00);               //don't care
  transmit((uint8_t) (addr >> 8));
  transmit((uint8_t) (addr));
  transmit(0x00);               //don't care
  for (size_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeByteBuf1(uint16_t addr, uint8_t data)
{
  activate();
  transmit(Buf1Write);
  transmit(0x00);               //don't care
  transmit((uint8_t) (addr >> 8));
  transmit((uint8_t) (addr));
  transmit(data);               //write data byte
  deactivate();
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeStrBuf1(uint16_t addr, uint8_t *data, size_t size)
{
  activate();
  transmit(Buf1Write);
  transmit(0x00);               //don't care
  transmit((uint8_t) (addr >> 8));
  transmit((uint8_t) (addr));
  for (size_t i = 0; i < size; i++) {
    transmit(*data++);
  }
  deactivate();
}

// Transfers Dataflash SRAM buffer 1 to flash page including page erase
void Sodaq_Dataflash::writeBuf1ToPage(uint16_t pageAddr)
{
  activate();
  transmit(Buf1ToFlashWE);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

// Transfers Dataflash SRAM buffer 1 to flash page without page erase
void Sodaq_Dataflash::writeBuf1ToPageWoE(uint16_t pageAddr)
{
  activate();
  transmit(Buf1ToFlashWoE);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

// Writes a number of bytes to buffer 1 and transfer the buffer to flash page
// including page erase
void Sodaq_Dataflash::writeStrBuf1ThenFlashWE(uint16_t pageAddr, uint16_t addr, uint8_t *data, size_t size)
{
  activate();
  transmit(Buf1WriteThenFlashWE);
  setFullAddr(pageAddr, addr);
  for (size_t i = 0; i < size; i++) {
    transmit(*data++);
  }
  deactivate();
  waitTillReady();
}

// Compares buffer 1 to addressed page
bool Sodaq_Dataflash::buf1FlashCompare(uint16_t pageAddr)
{
  activate();
  transmit(Buf1FlashComp);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
  return !(readStatus() & 0x40);
}

void Sodaq_Dataflash::pageErase(uint16_t pageAddr)
{
  activate();
  transmit(PageErase);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

void Sodaq_Dataflash::blockErase(uint16_t pageAddr)
{
  activate();
  transmit(BlockErase);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

void Sodaq_Dataflash::sectorErase(uint16_t pageAddr)
{
  activate();
  transmit(SectorErase);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

/* 
 * From AT45DB321D and AT45DB642D datasheets:
 *
 * Errata - Chip Erase - Issue
 *
 * In a certain percentage of units, the Chip Erase feature may
 * not function correctly and may adversely affect device operation.
 * Therefore, it is recommended that the Chip Erase commands
 * (opcodes C7H, 94H, 80H, and 9AH) not be used.
 *
 * Workaround
 * Use Block Erase (opcode 50H) as an alternative. The Block Erase
 * function is not affected by the Chip Erase issue.
 */
void Sodaq_Dataflash::chipErase()
{
  // Apply workaround for possibly buggy chips
  // Doesn't work if detection is disabled. Must be done manually in sketch.
  if (willdetect == true && (chip == Density_32Mb || chip == Density_64Mb)) {
    for (size_t i = 0; i < (uint16_t) blocksPerSector() * sectorsPerChip(); i++) {
    blockErase(i * pagesPerBlock());  // Erase each block one by one
    }
  }
  else {            // Other chips should work ok
  activate();
  transmit(0xC7);
  transmit(0x94);
  transmit(0x80);
  transmit(0x9A);
  deactivate();
  waitTillReady();
  }
}

void Sodaq_Dataflash::settings(SPISettings settings)
{
  _settings = settings;
}

void Sodaq_Dataflash::deactivate()
{
    digitalWrite(_csPin,HIGH);
    SPI.endTransaction();
}
void Sodaq_Dataflash::activate()
{
    SPI.beginTransaction(_settings);
    digitalWrite(_csPin,LOW);
}

void Sodaq_Dataflash::setPageAddr(unsigned int pageAddr)
{
  transmit(getPageAddrByte0(pageAddr));
  transmit(getPageAddrByte1(pageAddr));
  transmit(getPageAddrByte2(pageAddr));
}

/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 */
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */
/*
 * Address fields - don't care, address bits (df_page_addr_bits), don't care:
 *   AT45DB011D 6, 9,   9  ------PP PPPPPPPB BBBBBBBB
 *   AT45DB021D 5, 10,  9  -----PPP PPPPPPPB BBBBBBBB
 *   AT45DB041D 4, 11,  9  ----PPPP PPPPPPPB BBBBBBBB
 *   AT45DB081D 3, 12,  9  ---PPPPP PPPPPPPB BBBBBBBB
 *   AT45DB161D 2, 12, 10  --PPPPPP PPPPPPBB BBBBBBBB
 *   AT45DB321D 1, 13, 10  -PPPPPPP PPPPPPBB BBBBBBBB
 *   AT45DB642D 0, 13, 11  PPPPPPPP PPPPPBBB BBBBBBBB
 *   -: don't care
 *   P: pageaddress
 *   B: don't care or byteaddress
 *   The last "B"-bits equal dfpagebits, which addresses a byte
 *   within a page (used in setFullAddr()).
 */
uint8_t Sodaq_Dataflash::getPageAddrByte0(uint16_t pageAddr)
{
  // More correct would be to use a 24 bits number
  // shift to the left by number of bits. But the uint16_t can be considered
  // as if it was already shifted by 8.
  return (pageAddr << (dfpagebits - 8)) >> 8;
}
uint8_t Sodaq_Dataflash::getPageAddrByte1(uint16_t page) // "page" typo? "pageAddr" above and in header file.
{
  return page << (dfpagebits - 8);
}
uint8_t Sodaq_Dataflash::getPageAddrByte2(uint16_t page) // "page" typo? "pageAddr" above and in header file.
{
  return 0;
}

void Sodaq_Dataflash::setFullAddr(uint16_t pageAddr, uint16_t addr)
{
  transmit(getPageAddrByte0(pageAddr));       // Works fine, since high order
                                              // byte only includes "P"-bits
  transmit(getFullAddrByte1(pageAddr, addr)); // Also includes "B"-bits
  transmit(getFullAddrByte2(addr));           // Only includes "B"-bits
}
uint8_t Sodaq_Dataflash::getFullAddrByte1(uint16_t pageAddr, uint16_t addr)
{
  pageAddr = pageAddr << dfpagebits;
  return (pageAddr | addr) >> 8; // Returns both "P"- and "B"-bits
}
uint8_t Sodaq_Dataflash::getFullAddrByte2(uint16_t addr)
{
  return addr;                   // Returns the rest of the "B"-bits
}

// Returns density of chip or error codes
uint8_t Sodaq_Dataflash::chipdetect()
{
  uint8_t chipid[4];
  /*
   * All chips use the same mechanism for identification. From the datasheets:
   *   "The identification method and the command opcode comply with the JEDEC standard for
   *   “Manufacturer and Device ID Read Methodology for SPI Compatible Serial Interface Memory
   *   Devices”. The type of information that can be read from the device includes the JEDEC
   *   defined Manufacturer ID, the vendor specific Device ID, and the vendor specific Extended 
   *   Device Information."
   */
  readID(chipid); // Should work for all chips
  if (chipid[0] == Manufacturer) {
    if (chipid[1] >> Family_shiftbits == Family_DF) {
      uint8_t density = chipid[1] & Density_mask;
      if (density == Density_1Mb || density == Density_2Mb || density == Density_4Mb ||
        density == Density_8Mb || density == Density_16Mb || density == Density_32Mb ||
        density == Density_64Mb) {
        return density;
      }
      else {
        return DENSITY_NOT_DETECTED;
      }
    }
    else {
      return FAMILY_NOT_DETECTED;
    }
  }
  else {
    return MANUFACTURER_NOT_DETECTED;
  }
}

// Returns detected chips DF_PAGE_ADDR_BITS
uint8_t Sodaq_Dataflash::df_page_addr_bits()
{
  if (willdetect == true) {
    return df_page_addr_bits_array[chip - 2]; // Lookup page address bits in array
  }
  return Detection_off;
}

// Returns detected chips DF_PAGE_SIZE
uint16_t Sodaq_Dataflash::df_page_size()
{
  if (willdetect == true) {
    return df_page_size_array[chip - 2];      // Lookup page size in array
  }
  return Detection_off;
}

// Returns detected chips DF_PAGE_BITS
uint8_t Sodaq_Dataflash::df_page_bits()
{
  if (willdetect == true) {
    return df_page_bits_array[chip - 2];      // Lookup page bits in array
  }
  return Detection_off;
}

// Returns detected chips bytes per page
uint16_t Sodaq_Dataflash::bytesPerPage()
{
  return df_page_size();                      // Same as df_page_size()
}

// Returns detected chips pages per block
uint8_t Sodaq_Dataflash::pagesPerBlock()
{
  return 8;                                   // 8 pages for all chips
}

// Returns detected chips blocks per sector
uint8_t Sodaq_Dataflash::blocksPerSector()
{
  if (willdetect == true) {
    return blocksPerSector_array[chip - 2];   // Lookup number of blocks in array
  }
  return Detection_off;
}

// Returns detected chips sectors
uint8_t Sodaq_Dataflash::sectorsPerChip()
{
  if (willdetect == true) {
    return sectorsPerChip_array[chip - 2];    // Lookup number of sectors in chip
  }
  return Detection_off;
}

// Returns chips total number of bytes
uint32_t Sodaq_Dataflash::bytesPerChip()
{
  if (willdetect == true) {
    return (uint32_t) df_page_size() * pagesPerBlock() * blocksPerSector() * sectorsPerChip();
  }
  return Detection_off;
}

// Returns blocknumber of given page address
uint8_t Sodaq_Dataflash::pageToBlockNumber(uint16_t pageAddr)
{
  if (willdetect == true) {
    return pageAddr / pagesPerBlock();
  }
  return Detection_off;
}

// Returns sectornumber of given page address
uint8_t Sodaq_Dataflash::pageToSectorNumber(uint16_t pageAddr)
{
  if (willdetect == true) {
    return pageAddr / ((uint16_t) blocksPerSector() * pagesPerBlock());
  }
  return Detection_off;
}

// Returns sectornumber of given block address
uint8_t Sodaq_Dataflash::blockToSectorNumber(uint8_t blockNumber)
{
  if (willdetect == true) {
    return blockNumber / blocksPerSector();
  }
  return Detection_off;
}

// Enters deep power-down mode (after 3us)
void Sodaq_Dataflash::sleepPower()
{
  activate();
  transmit(DeepPowerDown);
  deactivate();
}

// Resumes from deep power-down mode (after 35us)
void Sodaq_Dataflash::wakePower()
{
  activate();
  transmit(DeepPowerUp);
  deactivate();
  delayMicroseconds(35); // "activate()" must be disabled at least 35us
}

// Use a single common instance
Sodaq_Dataflash dflash;
