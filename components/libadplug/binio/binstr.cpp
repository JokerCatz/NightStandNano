/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * binstr.cpp - Binary I/O on standard C strings in memory
 * Copyright (C) 2003, 2010 Simon Peter <dn.tlp@gmx.net>
 */

#include "binstr.h"

/* libadplug patch 0003: a stream that is read past its end more than this many times calls adp_eof_overrun(), which abandons the song (a loop that
 * keeps reading zeros at the end of the data never ends otherwise). Reads past the end themselves return 0 and set Eof, as before. */
extern "C" void adp_eof_overrun(void);
#define ADP_EOF_OVERRUN_MAX 262144

/***** binsbase *****/

binsbase::binsbase(void *str, unsigned long len)
  : pastend(false), overrun(0), data((Byte *)str), spos((Byte *)str), length(len)
{
}

binsbase::~binsbase()
{
}

void binsbase::seek(long p, Offset offs)
{
  /* libadplug patch 0003: integer arithmetic (data + p overflowed the pointer on a hostile offset); never leaves [data, data + length] */
  long long np;
  switch(offs) {
  case Add: np = (long long)(spos - data) + p; break;
  case End: np = (long long)length + p; break;
  default: np = p; break;
  }
  if(np < 0) { spos = data; return; }
  if(np > (long long)length) {
    err |= Eof; pastend = true;
    np = (long long)length;
  }
  spos = data + np;
}

long binsbase::pos()
{
  return (long)(spos - data);
}

/***** binisstream *****/

binisstream::binisstream(void *str, unsigned long len)
  : binsbase(str, len)
{
}

binisstream::~binisstream()
{
}

binisstream::Byte binisstream::getByte()
{
  Byte in = 0;

  if((unsigned long)(spos - data) >= length) {
    err |= Eof; pastend = true;
    if(++overrun > ADP_EOF_OVERRUN_MAX) adp_eof_overrun();
  }
  else {
    in = *spos;
    spos++;
  }

  return in;
}

/***** binosstream *****/

binosstream::binosstream(void *str, unsigned long len)
  : binsbase(str, len)
{
}

binosstream::~binosstream()
{
}

void binosstream::putByte(Byte b)
{
  if(spos - data >= length) {
    err |= Eof;
  } else {
    *spos = b;
    spos++;
  }
}

/***** binsstream *****/

binsstream::binsstream(void *str, unsigned long len)
  : binsbase(str, len), binisstream(str, len), binosstream(str, len)
{
}

binsstream::~binsstream()
{
}
