#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include "antiportscan.h"
 

static unsigned int direct_fun(unsigned int hook,
                           struct sk_buff **pskb,
                           const struct net_device *in,
                           const struct net_device *out,
                           int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb=*pskb;
    struct iphdr *iph = ip_hdr(skb);
    struct ethhdr *eth = eth_hdr(skb);
    struct tcphdr *tcph = NULL;
    struct udphdr *udph=NULL;
    unsigned int sip, dip;
    unsigned short source, dest;
    int plen;

    if(!skb)
    {
        return NF_ACCEPT;
    }

    if(!skb->dev)
    {
        return NF_ACCEPT;
    }

    if(strcmp(skb->dev->name, "br0"))
    {
        return NF_ACCEPT;
    }
 
    if(!eth)
    {
        return NF_ACCEPT;
    }
 
    if(!iph)
    {
        return NF_ACCEPT;
    }


    if(skb->pkt_type == PACKET_BROADCAST)
    {
        return NF_ACCEPT;
    }
 
    if((skb->protocol==htons(ETH_P_8021Q)||skb->protocol==htons(ETH_P_IP))&&skb->len>=sizeof(struct ethhdr))
    {
        if(skb->protocol==htons(ETH_P_8021Q))
        {
            iph=(struct iphdr *)((u8*)iph+4);
        }
 
        if(iph->version!= 0x04)
        {
            return NF_ACCEPT;
        }
 
        if (skb->len < 20)
        {
            return NF_ACCEPT;
        }
 
        if ((iph->ihl * 4) > skb->len || skb->len < ntohs(iph->tot_len) || (iph->frag_off & htons(0x1FFF)) != 0)
        {
            return NF_ACCEPT;
        }
 
        sip = iph->saddr;
        dip = iph->daddr;
    }

    return NF_ACCEPT;
}


static struct nf_hook_ops portal_auth_ops =
{
    .hook = direct_fun,
    .pf = PF_INET,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
}; 


static int __init auth_init(void)
{
    printk("line = %d, func = %s\n", __LINE__, __func__);
    nf_register_hook(&portal_auth_ops);
    return 0;
}

static void __exit auth_eixt(void)
{   
    printk("line = %d, func = %s\n", __LINE__, __func__);
    nf_unregister_hook(&portal_auth_ops);
}

module_init(auth_init);
module_exit(auth_eixt);
MODULE_LICENSE("GPL");
