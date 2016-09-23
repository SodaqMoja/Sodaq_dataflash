/*
 * Copyright (c) 2013 Kees Bakker.  All rights reserved.
 *
 * This file is part of Sodaq_dataflash.
 *
 * Sodaq_dataflash is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published bythe Free Software Foundation, either version 3 of
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
 * Since the library now detects the chip by default, use
 * uint8_t df_page_addr_bits(), uint16_t df_page_size() and uint8_t df_page_bits(),
 * rather then
 * #define DF_PAGE_ADDR_BITS, #define DF_PAGE_SIZE and #define DF_PAGE_BITS!
 * Alternatively, disable detection with the init() function and #define chip!
 */

/*
 * This sketch demonstrates the added functions "sleepPower()" and "wakePower()".
 * Current should be redused from 25 uA (idle) to 15 uA. It will take 3 us to
 * enter deep power-down mode and 35 us to wake up again.
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


void chipalive() {
  uint8_t id[4];
  Serial.println("Trying to read the chips identity 5 times ...");
  for (byte i = 0; i < 5; i++) {
    dflash.readID(id);
    for (byte i = 0; i < 4; i++) {
      Serial.print(i + 1);
      Serial.print(". byte: ");
      Serial.println(id[i], HEX);
    }
    Serial.println();
  }
}


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);          // Preserve returned detection result

  const uint16_t pageadr = 64;               // Address of page to read
  const uint16_t addr = 0;                   // Offset address of page to read
  const size_t size = dflash.df_page_size(); // Number of bytes to read
  uint8_t data[size];                        // Received data
  
  long int timer;

  if (failed != 0) {
    switch (failed) {
      case MANUFACTURER_NOT_DETECTED: Serial.println("Manufacturer not recognized!"); break;
      case FAMILY_NOT_DETECTED: Serial.println("Chip family not recognized!"); break;
      case DENSITY_NOT_DETECTED: Serial.println("Chip size not recognized!"); break;
      default: Serial.println("Chip not detected! Check your wiring!"); break;
    }
    return;
  }

  // Print the detected chips properties
  Serial.print("Number bits for selecting pages: "); Serial.println(dflash.df_page_addr_bits());
  Serial.print("Number of bytes per page: "); Serial.println(dflash.df_page_size());
  Serial.print("Number of bits to address byte in page: "); Serial.println(dflash.df_page_bits());
  Serial.println();

  // Methods to control deep power-down mode
  Serial.println("Chip is now awake");
  chipalive();
  Serial.println("Going to sleep ...");
  dflash.sleepPower();
  // Now, the only command the chip accepts is the one to wake it up
  chipalive();
  // Beware that arbitrary values would be read from the SO-pin,
  // due to the high impedance state of the output.
  Serial.println("Wake up ...");
  dflash.wakePower();
  chipalive();
}


void loop() {
}
