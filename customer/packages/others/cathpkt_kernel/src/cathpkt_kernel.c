#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/rwsem.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/sched.h>
#include "cathpkt_kernel.h"
#include <linux/version.h>

#define CATHPKT_DEV_NAME  "cathpkt"
//per 30s  max  Host  500
static unsigned int HostNum = 0;
extern int (*cathpkt_hook)(struct sk_buff *skb);
#define CATHPKT_MAX_HOST_NUM 500

static struct cathpkt_dev *cathpkt;
int cathpkt_open(struct inode *inode, struct file *filp);
int cathpkt_release(struct inode *inode, struct file *filp);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static int cathpkt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int cathpkt_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg);
#endif
static __init int cathpkt_init(void);
static __exit void cathpkt_cleanup(void);

struct file_operations cathpkt_fops = {
    .owner   = THIS_MODULE,
    .open    = cathpkt_open,
    .release = cathpkt_release,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    .unlocked_ioctl  =    cathpkt_ioctl,
#else
    ioctl  =    cathpkt_ioctl,
#endif

};


int cathpkt_open(struct inode *inode, struct file *filp)
{
    struct cathpkt_dev *dev;

    dev = container_of(inode->i_cdev, struct cathpkt_dev, cdev);
    filp->private_data = dev;

    return 0;
}

int cathpkt_release(struct inode *inode, struct file *filp)
{
    return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static int cathpkt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int cathpkt_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    struct cathpkt_dev *dev = filp->private_data;
    CATHPKT_IOCTL_PARMS parms;
    int rc = 0;

    if(_IOC_TYPE(cmd) != CATHPKT_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    switch(cmd) 
    {
        //printk("CATHPKT_IOCTL_ENALBE=%x,line = %d, func = %s, file = %s\n",cmd, __LINE__, __func__, __FILE__);
        case CATHPKT_IOCTL_ENALBE_WRITE:
        {
            if(copy_from_user(&parms, (void*)arg, sizeof(CATHPKT_IOCTL_PARMS))) 
            {
                rc = -EFAULT;
                break;
            }

            cathpkt_set(parms.len);
            break;
        }

        case CATHPKT_IOCTL_HOST_INFO_READ:
        {
            if(copy_from_user(&parms, (void*)arg, sizeof(CATHPKT_IOCTL_PARMS))) 
            {
                rc = -EFAULT;
                break;
            }

            cathpkt_getpktinfo(dev, parms.buf, parms.len);
            break;
        }
    }

    return rc;
}

static __init cathpkt_init(void)
{
    int result = 0;
    int dev_major = 0;
    dev_t dev;

    result = alloc_chrdev_region(&dev, 0, 1, CATHPKT_DEV_NAME);
    if( result < 0 )
    {
        printk("cathpkt: Could not register driver\n");
        return result;
    }
    dev_major = MAJOR(dev);

    cathpkt = kmalloc(sizeof(struct cathpkt_dev), GFP_KERNEL);
    if(cathpkt == NULL)
    {
        printk("cathpkt: Failed to allocate device structures\n");
        unregister_chrdev_region(dev, 1);
        return -ENOMEM;
    }

    memset(cathpkt, 0, sizeof(struct cathpkt_dev));
    //init_MUTEX(&cathpkt->lock);
    sema_init(&cathpkt->lock, 1);
    INIT_LIST_HEAD(&cathpkt->list);  
    cdev_init(&cathpkt->cdev, &cathpkt_fops);
    cathpkt->cdev.owner = THIS_MODULE;
    result = cdev_add(&cathpkt->cdev, dev, 1);
    if(result != 0)
    {
        printk("cathpkt: Error %d adding char driver", result);
        kfree(cathpkt);
        unregister_chrdev_region(dev, 1);
        return result;
    }

    return result;
}

int cathpkt_getpktinfo(struct cathpkt_dev *dev, char *pktbuf, unsigned int pktlen)
{
    struct catpktNode *catp = NULL;
    char *bufp = pktbuf;

    down(&dev->lock);
    //printk("Kerr HostNum = %d \n", HostNum);
    *(unsigned int *)bufp = HostNum;
    bufp += sizeof(unsigned int);

    if(!list_empty(&dev->list))
    {
        list_for_each_entry(catp, &dev->list, list)
        {
            if(((bufp - pktbuf) + 6 + sizeof(time_t) + 6 + catp->hostlen) < pktlen)
            {
                memcpy(bufp, catp->mac, 6);
                bufp += 6;
                *(time_t *)bufp = catp->timestap;
                bufp += sizeof(time_t);
                strncpy(bufp, catp->devname, 6);
                bufp += 6;
                if(catp->host)
                {
                    *(unsigned int *)bufp = catp->hostlen;
                    bufp += sizeof(unsigned int);
                    strncpy(bufp, catp->host, catp->hostlen);
                    bufp += catp->hostlen;
                }
                else
                {
                    printk("NULL hosturl in hostlist.");
                    break;
                }
            }
        }
    }

    cathpkt_HostInfoClear(dev);

    up(&dev->lock);
    return 0;
}

static int cathpkt_HostInfoClear(struct cathpkt_dev *dev)
{
    struct catpktNode *catp = NULL;

    while( !list_empty(&cathpkt->list) )
    {
		catp = list_entry(cathpkt->list.next, struct catpktNode, list);
		list_del(cathpkt->list.next);
		kfree(catp->host);
    	kfree(catp);
    }
#if 0
    if(!list_empty(&dev->list))
    {
        printk("%s[%d]\n", __FUNCTION__, __LINE__);
        list_for_each_entry(catp, &dev->list, list)
        {
            printk("%s[%d]\n", __FUNCTION__, __LINE__);
            list_del(&catp->list); 
            printk("%s[%d]\n", __FUNCTION__, __LINE__);
            kfree(catp->host);
            printk("%s[%d]\n", __FUNCTION__, __LINE__);
            kfree(catp);
            printk("%s[%d]\n", __FUNCTION__, __LINE__);
        }
    }
#endif
    HostNum = 0;
    return 0;
}

static int cathpkt_addHostinfo(unsigned char *mac, char *host, char *dname)
{
    struct catpktNode *catp = NULL;
    struct timeval tv;

    if((host == NULL) || (dname == NULL))
    {
        printk("cathpkt_addHostinfo invalid args, host or devname is NULL.");
        return -1;
    }

    if(HostNum >= CATHPKT_MAX_HOST_NUM)
    {
        printk("cathpkt_addHostinfo over flow, HostNum = %d.\n", HostNum++);
        return -1;
    }

    if((catp = (struct catpktNode *)kmalloc(sizeof(struct catpktNode), GFP_KERNEL)) != NULL)
    {
        memset(catp, 0, sizeof(struct catpktNode));
        INIT_LIST_HEAD(&catp->list);
        //0~5 dst mac 6~11 src mac
        memcpy(catp->mac, mac + 6, 6);
        strncpy(catp->devname, dname, 5); //athxx
        do_gettimeofday(&tv);
        catp->timestap = tv.tv_sec;
        catp->hostlen = strlen(host) + 1;
        if((catp->host = (char *)kmalloc(strlen(host) + 1, GFP_KERNEL)) != NULL)
        {
            memset(catp->host, 0, strlen(host) + 1);
            strcpy(catp->host, host);
        }
        else
        {
            printk("cathpkt_addHostinfo can not get memory to save host info.");
            kfree(catp);
            return -1;
        }

        down(&cathpkt->lock);
        list_add_tail(&catp->list, &cathpkt->list);       
        HostNum++;
        up(&cathpkt->lock);
        catp = NULL;
    }
    else
    {
        printk("cathpkt_addHostinfo can not get memory for cathpktnode.");
        return -1;
    }

    return 0;
}

static int cathpkt_decode( struct sk_buff *skb ) 
{
    struct iphdr *iphdrp = NULL;
    struct tcphdr *tcphdrp = NULL;
    char *cathp = NULL;
    char *cp = NULL;
    int Hlen = 0;

    if(ntohs(skb->protocol) == ETH_P_IP) //IPv4se And no tags
    {
        iphdrp = (struct iphdr *)skb->data;
        if(iphdrp->protocol == 6 )
        {
            if(iphdrp->version != 4)
                return 0;

            cathp = (char *)(skb->data + iphdrp->ihl*4);
            tcphdrp = (struct tcphdr *)cathp;

            if(ntohs(tcphdrp->dest) == 0x50)
            {
                Hlen = ntohs(iphdrp->tot_len) - iphdrp->ihl*4 - tcphdrp->doff*4;
                if((tcphdrp->psh == 0x1) && (tcphdrp->ack == 0x1) && Hlen > 4)
                {
                    cathp = (char *)(skb->data + iphdrp->ihl*4 + tcphdrp->doff*4);
                    if(cathp[0]==0x47 && cathp[1]==0x45 && cathp[2]==0x54 && cathp[3]==0x20)
                    {
                        if ((cp = strstr(cathp, "Host:")) != NULL) 
                        {
                            char url[128] = {0};
                            unsigned long url_len = 0;
                            char *cp2;
                            cp += strlen("Host: ");
                            if(((cp2 = strstr(cp, "\r\n")) != NULL) || ((cp2 = strstr(cp, "\n")) != NULL))
                                url_len = cp2 - cp;
                            memcpy(url, cp, url_len > 128 ? 128 : url_len);
                            url[128 - 1] = '\0';
                            if((cp = strstr(url, "/")) != NULL)
                                *cp = '\0';
                            cathpkt_addHostinfo((unsigned char *)skb->mac_header, url, skb->dev->name);
                            //printk("line = %d, func = %s, file = %s,url=%s,skb->dev->name=%s\n", __LINE__, __func__, __FILE__,url,skb->dev->name);
                        }
                    }
                }
            }
        }
    }

    return 0;
}

static void cathpkt_set(uint32_t enable)
{
    if(enable)
    {
        cathpkt_hook = cathpkt_decode;
    }
    else
    {
        cathpkt_hook = NULL;
    }

    //printk("cathpkt board set enable = %d.\n", enable);
}

static __exit void cathpkt_cleanup(void)
{
    cathpkt_HostInfoClear(cathpkt);
    unregister_chrdev_region(MKDEV(0, 0), 1);
}
/* ------------------------------------------------------------------
 * Generic module code.
 */
module_init(cathpkt_init);
module_exit(cathpkt_cleanup);

MODULE_AUTHOR("phicomm - vity.jia");
MODULE_LICENSE("GPL v2");

