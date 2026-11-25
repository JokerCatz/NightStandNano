/* AdPlug 2.4 database.h replaced (libadplug patch 0001): the record database (clock speeds, song lengths) is not built; players keep their
 * `CAdPlugDatabase *db` member, always 0. Only the file key (CRC16:CRC32 of the whole file) stays: got.cpp tells two songs apart with it
 * (code of CAdPlugDatabase::CKey::make in database.cpp, "CRC16 and CRC32 calculation routines (c) Zhengxi"). */
#ifndef H_ADPLUG_DATABASE
#define H_ADPLUG_DATABASE
#include <binio.h>
class CAdPlugDatabase
{
public:
  class CKey
  {
  public:
    unsigned short crc16;
    unsigned long crc32;
    CKey() : crc16(0), crc32(0) {}
    CKey(binistream &buf)
    {
      crc16 = 0; crc32 = 0xffffffffUL;
      while(!buf.eof()) {
        unsigned char byte = buf.readInt(1);
        for(int j = 0; j < 8; j++) {
          crc16 = ((crc16 ^ byte) & 1) ? (unsigned short)((crc16 >> 1) ^ 0xa001) : (unsigned short)(crc16 >> 1);
          crc32 = ((crc32 ^ byte) & 1) ? (crc32 >> 1) ^ 0xedb88320UL : crc32 >> 1;
          byte >>= 1;
        }
      }
      crc32 = ~crc32 & 0xffffffffUL;
    }
    bool operator==(const CKey &key) { return crc16 == key.crc16 && crc32 == key.crc32; }
  };
};
#endif
