#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include "kantiportscan.h"

#define BLACK_NUM 20
#define TIME_NUM 10
#define TIME_THRESHOLD HZ
struct Time_Port{
    int src_port;
    unsigned long  jiffies; 
};

typedef struct Data_list{
    //int src_port;
    unsigned int sour_ip;
    unsigned long  jiffies; 
    int index;
    struct Time_Port Timport[TIME_NUM];
    unsigned int flag;//num is full
    struct Data_list *next;
}SYN_LIST;
typedef struct Black_list{
    int src_port;
    unsigned int sour_ip;
    //int attack_num;
    unsigned int flag;//0 white  1 black
}BLACK_LIST;
extern unsigned long volatile jiffies; 
//time_t start_time;
//unsigned long int start_time;
//unsigned long int enter_time;
struct Black_Total{
    unsigned int flag;//0 no black  1 yes
    BLACK_LIST pblack[BLACK_NUM];
}black_total;

SYN_LIST *pSyn_List=NULL;

void sort(SYN_LIST **h)
{
   SYN_LIST *p,*q,*r,*s,*h1;
   //h1=p=(SYN_LIST*)malloc(sizeof(SYN_LIST));
   h1=p=kmalloc(sizeof(SYN_LIST),GFP_KERNEL);
   p->next=*h;
   while(p->next!=NULL)
   { 
     q=p->next;
     r=p;
     while(q->next!=NULL)
     {
        if(q->next->jiffies<r->next->jiffies)
        {
           r=q;
        }
        q=q->next;
     }
     if(r!=p)
     { 
         s=r->next;
         r->next=s->next;
         s->next=p->next;
         p->next=s;
     }
     p=p->next;
   }
   *h1->next;
   kfree(h1);
}

static unsigned int direct_fun(unsigned int hook,
        struct sk_buff *pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb=pskb;
    struct iphdr *iph = ip_hdr(skb);
    unsigned int sip, dip ,syn ,ack;
    int i;
    struct ethhdr *eth = eth_hdr(skb);
    struct tcphdr *tcph = NULL;
    //struct udphdr *udph=NULL;
    unsigned short source, dest;

    if(!skb)
    {
        return NF_ACCEPT;
    }

    if(!skb->dev)
    {
        return NF_ACCEPT;
    }

    if(!strcmp(skb->dev->name, "br0"))
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

    //return NF_ACCEPT;
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

        if(iph->protocol != 6)
        {
            /* not tcp, just accept */
            return NF_ACCEPT;
        }

        tcph = (struct tcphdr *)((unsigned char *)iph+iph->ihl*4);
        source = ntohs(tcph->source);
        dest = ntohs(tcph->dest);
        syn  = tcph->syn;
        ack  = tcph->ack;

       // printk(" line = %d, func = %s, syn = 0x%x, ack = 0x%x,source=%d,ack=%d,tcph->syn=%0x,tcph->ack=%0x,jiffies=%d,TIME_THRESHOLD=%d\n", __LINE__, __func__, syn, ack,source,ack,tcph->syn,tcph->ack,jiffies,TIME_THRESHOLD);
        //msleep(5);

        if(syn == 1 && ack == 0)   // syn 
        {         

            /* black list check, TODO: add black list ip count check */
            if (black_total.flag == 1)
            {
              for(i=0;i<BLACK_NUM;i++)
              {
                  if(black_total.pblack[i].sour_ip == sip)
                  {
                      if(black_total.pblack[i].flag == 1)
                      {
                            return NF_DROP;
                      }
                  }
              }            
            }

            if(pSyn_List == NULL)
            {
                pSyn_List = kmalloc(sizeof(struct Data_list),GFP_KERNEL);

                if(pSyn_List != NULL)
                {
                     
                    pSyn_List->sour_ip = sip;
                    //pblack->src_port = source;
                    pSyn_List->next = NULL;
                    pSyn_List->jiffies = jiffies;
                    pSyn_List->index = 0;
                    pSyn_List->flag = 0;
                    pSyn_List->Timport[0].jiffies = jiffies;
                    pSyn_List->Timport[0].src_port = source;
                }
            }
            else
            {
                SYN_LIST *p1=NULL,*p2=NULL;
                if (jiffies - pSyn_List->jiffies > TIME_THRESHOLD)
                {
                    p2=pSyn_List;
                    while(p2 != NULL)
                    {
                        p1=p2;
                        p2=p2->next;
                        kfree(p1);
                    }
                    p1=NULL;
                    p2=NULL;
                    pSyn_List=NULL;
                    return NF_ACCEPT;
                }

                p1=p2=pSyn_List;

                //printk(" line = %d, func = %s, syn = 0x%x, ack = 0x%x,source=%d,ack=%d,p1->sour_ip=%x\n,szIP=%s", __LINE__, __func__, syn, ack,source,ack,p1->sour_ip,szIP);
                while(p1 != NULL)
                {
                    if (p1->sour_ip == sip)
                    {
                        p1->jiffies = jiffies;
                        p1->index++;
                        
                        if (p1->index == TIME_NUM)
                        {
                            p1->index = 0;
                        }

                        if(p1->index == (TIME_NUM-1))
                        {
                            p1->Timport[p1->index].jiffies          = jiffies;
                            p1->flag                                = 1;
                            p1->Timport[p1->index].src_port         = source;
                            if (p1->Timport[TIME_NUM-1].jiffies - p1->Timport[0].jiffies <= TIME_THRESHOLD)
                            {
                                if (p1->sour_ip == pSyn_List->sour_ip)
                                {
                                    pSyn_List = pSyn_List->next;
                                    kfree(p1);
                                }
                                else 
                                {
                                    p2->next = p1->next;
                                    kfree(p1);
                                }

                                /* add this ip to black list */
                                for(i=0;i<BLACK_NUM;i++)
                                {
                                    //printk(" line = %d, func = %s, syn = 0x%x, ack = 0x%x,source=%d,ack=%d,i=%d\n", __LINE__, __func__, syn, ack,source,ack,i);
                                    if(black_total.pblack[i].flag == 0)
                                    {
                                        black_total.flag               = 1;
                                        black_total.pblack[i].src_port = source;
                                        black_total.pblack[i].flag     = 1;
                                        black_total.pblack[i].sour_ip  = sip;
                                        break;
                                    }
                                }
                                return NF_DROP;
                            }
                            break;
                        }
                        else if(p1->flag == 0)
                        {
                            p1->Timport[p1->index].jiffies          = jiffies;
                            p1->Timport[p1->index].src_port         = source;
                            //printk(" line = %d, func = %s, syn = 0x%x, ack = 0x%x,source=%d,ack=%d,p1->Timport[p1->index].jiffies=%lu,p1->Timport[p1-index+1].jiffies=%lu,p1->index=%d\n", __LINE__, __func__, syn, ack,source,ack,p1->Timport[p1->index].jiffies,p1->Timport[p1->index+1].jiffies,p1->index);
                            break;
                        }
                        else
                        {
                            p1->Timport[p1->index].jiffies          = jiffies;
                            p1->Timport[p1->index].src_port         = source;

                            if (p1->Timport[p1->index].jiffies - p1->Timport[(p1->index)+1].jiffies <= TIME_THRESHOLD)
                            {
                                if (p1->sour_ip == pSyn_List->sour_ip)
                                {
                                    pSyn_List = pSyn_List->next;
                                    kfree(p1);
                                }
                                else 
                                {
                                    p2->next = p1->next;
                                    kfree(p1);
                                }

                                /* add this ip to black list */
                                for(i=0;i<BLACK_NUM;i++)
                                {
                                    if(black_total.pblack[i].flag == 0)
                                    {
                                        black_total.flag               = 1;
                                        black_total.pblack[i].src_port = source;
                                        black_total.pblack[i].flag     = 1;
                                        black_total.pblack[i].sour_ip  = sip;
                                    }
                                }
                                return NF_DROP;
                            }
                            break;
                        }
                    }
                    p2 = p1;
                    p1 = p1->next;
                }

                if (p1 == NULL)
                {
                    p1 = kmalloc(sizeof(struct Data_list),GFP_KERNEL);
                    if(p1 != NULL)
                    {
                     
                        p1->next           = pSyn_List;
                        pSyn_List          = p1;
                        pSyn_List->sour_ip = sip;
                        pSyn_List->jiffies = jiffies;
                        //pblack->src_port = source;
                        //pSyn_List->next    = NULL;
                        pSyn_List->index   = 0;
                        pSyn_List->flag    = 0;
                        pSyn_List->Timport[0].jiffies  = jiffies;
                        pSyn_List->Timport[0].src_port = source;
                    }
                }
                else
                {
                    if ((pSyn_List->next != NULL) && (p1 != p2))
                    {
                       p2->next = p1->next;
                       p1->next = pSyn_List;
                       pSyn_List = p1;
                       //sort(&pSyn_List);
                 
                    }
                }
            }
        }

        return NF_ACCEPT;
    }
    return NF_ACCEPT;
}


static struct nf_hook_ops portal_auth_ops =
{
    .hook = direct_fun,
    .owner		= THIS_MODULE,
    .pf		= NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    //.hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FIRST,
}; 


static int __init auth_init(void)
{
    //printk("line = %d, func = %s\n", __LINE__, __func__);
    nf_register_hook(&portal_auth_ops);
    return 0;
}

static void __exit auth_eixt(void)
{   
    nf_unregister_hook(&portal_auth_ops);
}

module_init(auth_init);
module_exit(auth_eixt);
MODULE_LICENSE("GPL");
