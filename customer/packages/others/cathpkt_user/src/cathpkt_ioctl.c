#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ioctl.h>
#include "cathpkt_user.h"
#include "cathpkt_ioctl.h"

int cathpkt__ioctl_set_enable(uint8_t enable)
{
    int fd = -1;
    CATHPKT_IOCTL_PARMS parms;

    memset(&parms, 0, sizeof(CATHPKT_IOCTL_PARMS));
    parms.len = enable;
    fd = open("/dev/cathpkt", O_RDWR);
    if(fd > 0)
    { 
        if(0 == ioctl(fd, CATHPKT_IOCTL_ENALBE_WRITE, &parms))
        {
            close(fd);
            return 0;
        }
        else
        {
            close(fd);
            CATHPKT_DEBUG("cathpkt__ioctl_set_enable ioctl error.");
            return -1;
        }
    }

    CATHPKT_DEBUG("cathpkt__ioctl_set_enable open dev cathpkt error.");
}

int cathpkt_ioctl_get_host_info(char *buf, uint32_t len)
{
    //CATHPKT_DEBUG("%s start.\n", __FUNCTION__);
    int fd = -1;
    CATHPKT_IOCTL_PARMS parms;

    memset(&parms, 0, sizeof(CATHPKT_IOCTL_PARMS));
    parms.buf = buf;
    parms.len = len;
    fd = open("/dev/cathpkt", O_RDWR);
    if(fd > 0)
    {
        if(0 == ioctl(fd, CATHPKT_IOCTL_HOST_INFO_READ, &parms))
        {
            close(fd);
            return 0;
        }
        else
        {
            close(fd);
            CATHPKT_DEBUG("cathpkt_ioctl_get_host_info ioctl error.");
            return -1;
        }
    }

    CATHPKT_DEBUG("cathpkt_ioctl_get_host_info open dev cathpkt error.");
}


