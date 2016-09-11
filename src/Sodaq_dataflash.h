#ifndef SODAQ_DATAFLASH_H
#define SODAQ_DATAFLASH_H
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

#include <stddef.h>
#include <stdint.h>
#include <SPI.h>

#define DF_AT45DB011D   1
#define DF_AT45DB021D   2
#define DF_AT45DB041D   3
#define DF_AT45DB081D   4
#define DF_AT45DB161D   5
#define DF_AT45DB321D   6
#define DF_AT45DB642D   7

#ifndef DF_VARIANT
#define DF_VARIANT DF_AT45DB161D
#endif

#if DF_VARIANT == DF_AT45DB011D
// configuration for the Atmel AT45DB011D device
#define DF_PAGE_ADDR_BITS       9    //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            264  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            9
/*
 * From AT45DB011D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of five don’t care bits, nine page
 *   address bits (PA8 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB021D
// configuration for the Atmel AT45DB021D device
#define DF_PAGE_ADDR_BITS       10   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            264  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            9
/*
 * From AT45DB021D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of five don’t care bits, 10 page
 *   address bits (PA9 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB041D
// configuration for the Atmel AT45DB041D device
#define DF_PAGE_ADDR_BITS       11   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            264  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            9
/*
 * From AT45DB041D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of four don’t care bits, 11 page
 *   address bits (PA10 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB081D
// configuration for the Atmel AT45DB081D device, Sodaq v2 has AT45DB081D, see doc3596.pdf, 4096 pages of 256/264 bytes
#define DF_PAGE_ADDR_BITS       12   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            264  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            9
/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB161D
// configuration for the Atmel AT45DB161D device
#define DF_PAGE_ADDR_BITS       12   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            528  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            10
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB321D
// configuration for the Atmel AT45DB321D device
#define DF_PAGE_ADDR_BITS       13   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            528  //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            10
/*
 * From AT45DB321D documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 1 don’t care bit, 13 page
 *    address bits (PA12 - PA0) that specify the page in the main memory to
 *    be written, and 10 don’t care bits."
 */

#elif DF_VARIANT == DF_AT45DB642D
// configuration for the Atmel AT45DB642D device
#define DF_PAGE_ADDR_BITS       13   //DF_PAGE_ADDR_BITS are not used by library
#define DF_PAGE_SIZE            1056 //DF_PAGE_SIZE are not used by library
#define DF_PAGE_BITS            11
/*
 * From AT45DB642D documentation
 *   "For the standard DataFlash page size (1056-bytes), the opcode must be
 *   followed by three address bytes consist of 13 page
 *   address bits (PA12 -  PA0) that specify the page in the main memory to
 *   be written and 11 don’t care bits."
 */

#else
#error "Unknown DF_VARIANT"
#endif
#define DF_NR_PAGES     (1 << DF_PAGE_ADDR_BITS) // DF_PAGE_ADDR_BITS and DF_NR_PAGES are not used by library

#define MANUFACTURER_NOT_DETECTED    0xFD     // Return value for chipdetect()
#define FAMILY_NOT_DETECTED          0xFE     // Return value for chipdetect()
#define DENSITY_NOT_DETECTED         0xFF     // Return value for chipdetect()

class Sodaq_Dataflash
{
public:
  uint8_t init(uint8_t csPin=SS);
  uint8_t init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint8_t ssPin) __attribute__((deprecated("Use: void init(uint8_t csPin=SS)")));
  uint8_t init(uint8_t ssPin, bool detect);
  void readID(uint8_t *data);
  void readSecurityReg(uint8_t *data, size_t size);

  uint8_t readByteBuf1(uint16_t pageAddr);
  void readStrBuf1(uint16_t addr, uint8_t *data, size_t size);
  void writeByteBuf1(uint16_t addr, uint8_t data);
  void writeStrBuf1(uint16_t addr, uint8_t *data, size_t size);

  void writeBuf1ToPage(uint16_t pageAddr);
  void readPageToBuf1(uint16_t PageAdr);

  void readStrPage(uint16_t PageAdr,uint16_t addr, uint8_t *data, size_t size);

  void pageErase(uint16_t pageAddr);
  void chipErase();

  void settings(SPISettings settings);

  uint8_t df_page_addr_bits();
  uint16_t df_page_size();
  uint8_t df_page_bits();

  void sleepPower();
  void wakePower();

private:
  uint8_t readStatus();
  void waitTillReady();
  uint8_t transmit(uint8_t data);
  void activate();
  void deactivate();
  void setPageAddr(unsigned int PageAdr); // "PageAdr" typo? "pageAddr" in cpp-file.
  uint8_t getPageAddrByte0(uint16_t pageAddr);
  uint8_t getPageAddrByte1(uint16_t pageAddr);
  uint8_t getPageAddrByte2(uint16_t pageAddr);
  void setFullAddr(uint16_t pageAddr, uint16_t addr);
  uint8_t getFullAddrByte1(uint16_t pageAddr, uint16_t addr);
  uint8_t getFullAddrByte2(uint16_t addr);
  uint8_t chipdetect();

  uint8_t _csPin;
  size_t _pageAddrShift; //Set to 1 in Sodaq_dataflash.cpp, but not used
  SPISettings _settings;
  static bool willdetect;
  static uint8_t dfpagebits;
  static uint8_t chip;

  static const uint8_t df_page_addr_bits_array[];
  static const uint16_t df_page_size_array[];
  static const uint8_t df_page_bits_array[];
};

extern Sodaq_Dataflash dflash;


#endif // SODAQ_DATAFLASH_H
