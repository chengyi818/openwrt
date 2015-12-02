typedef struct
{
    char *buf;
    uint32_t len;
} CATHPKT_IOCTL_PARMS;

struct cathpkt_dev
{
    struct cdev cdev;
    struct list_head list;
    struct semaphore lock;
};

struct catpktNode
{
    unsigned char mac[6];
    time_t timestap;
    char devname[6];
    unsigned int hostlen;
    char *host;
    struct list_head list;
};

#define CATHPKT_IOC_MAGIC 0xEE
#define CATHPKT_IOCTL_ENALBE_WRITE       _IOWR(CATHPKT_IOC_MAGIC, 0, CATHPKT_IOCTL_PARMS)
#define CATHPKT_IOCTL_HOST_INFO_READ     _IOWR(CATHPKT_IOC_MAGIC, 1, CATHPKT_IOCTL_PARMS)

static void cathpkt_set(uint32_t enable);
static int cathpkt_addHostinfo(unsigned char *mac, char *host, char *dname);
int cathpkt_getpktinfo(struct cathpkt_dev *dev, char *pktbuf, unsigned int pktlen);
static int cathpkt_HostInfoClear(struct cathpkt_dev *dev);
