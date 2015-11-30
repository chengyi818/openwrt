#ifndef __CATHPKT_USER_H
#define __CATHPKT_USER_H

#define CATHPKT_DEBUG(args...) cathpkt_show_debug_info(__FUNCTION__, __LINE__, args)

typedef struct
{
    char *buf;
    uint32_t len;
} CATHPKT_IOCTL_PARMS;

#endif
