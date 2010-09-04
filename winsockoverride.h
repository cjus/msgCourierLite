#ifndef WINSOCKOVERRIDE_H
#define WINSOCKOVERRIDE_H

// Define FD_SETSIZE to avoid winsock.h 64-socket limitation
// See: http://tangentsoft.net/wskfaq/advanced.html

#undef FD_SETSIZE
#define FD_SETSIZE 1024

#include <winsock.h>

#endif //WINSOCKOVERRIDE_H