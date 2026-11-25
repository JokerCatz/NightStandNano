#ifndef LIBCRSID_HEADER__HOST
#define LIBCRSID_HEADER__HOST //used  to prevent double inclusion of this header-file


#include <stdint.h>


enum cRSID_Specifications__File { CRSID_FILEVERSION_WEBSID=0x4E, CRSID_SECONDS_PER_MINUTE = 60 };

enum cRSID_File_Limitations {
 CRSID_FILESIZE_MAX = 100000,
};



#endif //LIBCRSID_HEADER__HOST