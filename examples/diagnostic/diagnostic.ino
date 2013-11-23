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
 * This is an example to show the working of Sodaq_dataflash.  It can
 * also be used as a utility to diagnose the dataflash.
 */

#include <Arduino.h>
#include <avr/wdt.h>

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_dataflash.h>

//#########   pin definitions   ########
#define GPRSBEE_PWRPIN  7
#define XBEECTS_PIN     8

#define DIAGPORT_RX     4
#define DIAGPORT_TX     5

#define DF_MOSI         11
#define DF_MISO         12
#define DF_SPICLOCK     13
#define DF_SLAVESELECT  10


//#########   diagnostic    #############
// Uncomment or make it an #undef to disable diagnostic output
#define ENABLE_DIAG     1

#ifdef ENABLE_DIAG
#define DIAGPRINT(...)          Serial.print(__VA_ARGS__)
#define DIAGPRINTLN(...)        Serial.println(__VA_ARGS__)
#else
#define DIAGPRINT(...)
#define DIAGPRINTLN(...)
#endif


//######### forward declare #############

//#########    setup        #############
void setup()
{

  dflash.init(DF_MISO, DF_MOSI, DF_SPICLOCK, DF_SLAVESELECT);
  Serial.begin(9600);
  rtc.begin();
}

void loop()
{
  getCommand();
}

static inline bool isTimedOut(uint32_t ts)
{
  return (long)(millis() - ts) >= 0;
}

// Read a line of input. Must be terminated with <CR> and optional <LF>
void readLine(char *line, size_t size)
{
  int c;
  size_t len = 0;
  uint32_t ts_waitLF = 0;
  bool seenCR = false;

  while (1) {
    c = Serial.read();
    if (c < 0) {
      if (seenCR && isTimedOut(ts_waitLF)) {
        // Line ended with just <CR>. That's OK too.
        goto end;
      }
      continue;
    }
    if (c != '\r') {
      // Echo the input, but skip <CR>
      Serial.print((char)c);
    }

    if (c == '\r') {
      seenCR = true;
      ts_waitLF = millis() + 100;       // Wait another .1 sec for an optional LF
    } else if (c == '\n') {
      goto end;
    } else {
      // Any other character is stored in the line buffer
      if (len < size - 1) {
        *line++ = c;
        len++;
      }
    }
  }
end:
  *line = '\0';
}

// Read commands from Serial (the default Arduino serial port)
void getCommand()
{
  char line[100];

  Serial.print("> ");
  readLine(line, sizeof(line));
  Serial.println();
  if (*line == '\0') {
  } else if (strcmp(line, "E Y") == 0) {
    DIAGPRINT("DF chip erase ...");
    dflash.chipErase();
    DIAGPRINTLN(" done");

  } else if (line[0] == 'D' || line[0] == 'd') {
    int page = strtol(line + 1, NULL, 0);
    dumpPage(page);

  } else if (line[0] == 'E' || line[0] == 'e') {
    int page = strtol(line + 1, NULL, 0);
    dflash.pageErase(page);

  } else if (strncasecmp(line, "1w", 2) == 0) {
    int page = strtol(line + 2, NULL, 0);
    writePage(page, 1);
  } else if (strncasecmp(line, "3w", 2) == 0) {
    int page = strtol(line + 2, NULL, 0);
    writePage(page, 3);
  }
}


/*
 * \brief Dump the contents of a data flash page
 */
void dumpPage(int page)
{
  if (page < 0)
    return;

  DIAGPRINT("page "); DIAGPRINTLN(page);
  dflash.readPageToBuf1(page);
  uint8_t buffer[16];
  for (uint16_t i = 0; i < DF_PAGE_SIZE; i += sizeof(buffer)) {
    size_t nr = sizeof(buffer);
    if ((i + nr) > DF_PAGE_SIZE) {
      nr = DF_PAGE_SIZE - i;
    }
    dflash.readStrBuf1(i, buffer, nr);

    dumpBuffer(buffer, nr);
  }
}

void dumpBuffer(uint8_t * buf, size_t size)
{
  while (size > 0) {
    size_t size1 = size >= 16 ? 16 : size;
    for (size_t j = 0; j < size1; j++) {
      // Silly Arduino Print has very limited formatting capabilities
      DIAGPRINT((*buf >> 4) & 0xF, HEX);        // High nibble
      DIAGPRINT(*buf & 0xF, HEX);               // Low nibble
      buf++;
    }
    DIAGPRINTLN();
    size -= size1;
  }
}

void writePage(int page, uint8_t value)
{
  uint8_t buffer[100];
  for (size_t i = 0; i < sizeof(buffer); ++i) {
    buffer[i] = value;
  }
  dflash.writeStrBuf1(0, buffer, sizeof(buffer));
  dflash.writeBuf1ToPage(page);
}

