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

#define BLACK_NUM 50
#define TIME_NUM 50
#define TIME_THRESHOLD HZ
struct Time_Port{
    /*int src_port;*/
    unsigned long  jiffies; 
};

typedef struct Data_list{
    unsigned int sour_ip;
    unsigned long  jiffies; 
    int index;
    struct Time_Port Timport[TIME_NUM];
    unsigned int flag;//num is full
    struct Data_list *next;
}DATA_LIST;


typedef struct Black_list{
    /*int src_port;*/
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

static DATA_LIST *pSyn_List=NULL;
static DATA_LIST *pUdp_List=NULL;


void sort(DATA_LIST **h)
{
    DATA_LIST *p,*q,*r,*s,*h1;
    //h1=p=(DATA_LIST*)malloc(sizeof(DATA_LIST));
    h1=p=kmalloc(sizeof(DATA_LIST),GFP_KERNEL);
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


/*static unsigned int process_ip_data(DATA_LIST * *data_list, const unsigned int sip, const unsigned short source)*/
static unsigned int process_ip_data(DATA_LIST * *data_list, const unsigned int sip)
{
    int i;

    if(*data_list == NULL)
    {
        *data_list = (DATA_LIST*)kmalloc(sizeof(struct Data_list),GFP_KERNEL);

        if(*data_list != NULL)
        {
            (*data_list)->sour_ip = sip;
            (*data_list)->next = NULL;
            (*data_list)->jiffies = jiffies;
            (*data_list)->index = 0;
            (*data_list)->flag = 0;
            (*data_list)->Timport[0].jiffies = jiffies;
            /*(*data_list)->Timport[0].src_port = source;*/
        }
    }
    else
    {
        DATA_LIST *p1=NULL,*p2=NULL;  //DATA_LIST
        if (jiffies - (*data_list)->jiffies > TIME_THRESHOLD)
        {
            p2=*data_list;
            while(p2 != NULL)
            {
                p1=p2;
                p2=p2->next;
                kfree(p1);
            }
            p1=NULL;
            p2=NULL;
            *data_list=NULL;
            return NF_ACCEPT;
        }

        p1=p2=*data_list;

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
                    /*p1->Timport[p1->index].src_port         = source;*/
                    if (p1->Timport[TIME_NUM-1].jiffies - p1->Timport[0].jiffies <= TIME_THRESHOLD)
                    {
                        if (p1->sour_ip == (*data_list)->sour_ip)
                        {
                            *data_list = (*data_list)->next;
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
                                /*black_total.pblack[i].src_port = source;*/
                                black_total.pblack[i].flag     = 1;
                                black_total.pblack[i].sour_ip  = sip;
                                printk("add %d in black list\n", sip);
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
                    /*p1->Timport[p1->index].src_port         = source;*/
                    //printk(" line = %d, func = %s, syn = 0x%x, ack = 0x%x,source=%d,ack=%d,p1->Timport[p1->index].jiffies=%lu,p1->Timport[p1-index+1].jiffies=%lu,p1->index=%d\n", __LINE__, __func__, syn, ack,source,ack,p1->Timport[p1->index].jiffies,p1->Timport[p1->index+1].jiffies,p1->index);
                    break;
                }
                else
                {
                    p1->Timport[p1->index].jiffies          = jiffies;
                    /*p1->Timport[p1->index].src_port         = source;*/

                    if (p1->Timport[p1->index].jiffies - p1->Timport[(p1->index)+1].jiffies <= TIME_THRESHOLD)
                    {
                        if (p1->sour_ip == (*data_list)->sour_ip)
                        {
                            *data_list = (*data_list)->next;
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
                                /*black_total.pblack[i].src_port = source;*/
                                black_total.pblack[i].flag     = 1;
                                black_total.pblack[i].sour_ip  = sip;
                                printk("add %d in black list\n", sip);
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
                p1->sour_ip = sip;
                p1->jiffies = jiffies;
                p1->index   = 0;
                p1->flag    = 0;
                p1->Timport[0].jiffies  = jiffies;
                /*p1->Timport[0].src_port = source;*/
			  p1->next           = *data_list;
                *data_list         = p1;
            }
        }
        else
        {
            if ((*data_list)->next != NULL&& (p1!= p2))
            {
                p2->next = p1->next;
                p1->next = *data_list;
                *data_list = p1;
            }
        }
    }

    return NF_ACCEPT;
}

static unsigned int dos_detect_fun(unsigned int hook,
        struct sk_buff *pskb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    struct sk_buff *skb=pskb;
    struct iphdr *iph = ip_hdr(skb);
    unsigned  int sip, dip ,syn ,ack, udp;
    struct ethhdr *eth = eth_hdr(skb);
    struct tcphdr *tcph = NULL;
    struct udphdr *udph=NULL;
    /*unsigned short source, dest;*/
    int i;

    if(!skb)
    {
        return NF_ACCEPT;
    }

    if(!skb->dev)
    {
        return NF_ACCEPT;
    }

    if(!strcmp(skb->dev->name, "ath"))
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

        if( (iph->protocol != 1) && (iph->protocol != 6) && (iph->protocol != 17))   // ICMP ,TCP,UDP 
        {
            /* not tcp, just accept */
            return NF_ACCEPT;
        }

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

        if (iph->protocol == 1)  // ICMP
        {
            return NF_ACCEPT;
        }
        else if (iph->protocol == 6)   // SYN(TCP)    IPPROTO_TCP
        {
            tcph = (struct tcphdr *)((unsigned char *)iph+iph->ihl*4);
            /*source = ntohs(tcph->source);
            dest = ntohs(tcph->dest);*/
            syn  = tcph->syn;
            ack  = tcph->ack;

            if(syn == 1 && ack == 0) 
            {
                /*return process_ip_data(&pSyn_List, sip, source);*/
                return process_ip_data(&pSyn_List, sip);
            }
        }	
        else     // UDP
        {
            udph = (struct udphdr *)((char *) iph + iph->ihl * 4);   
            /*source = ntohs(udph->source);*/
            /*return process_ip_data(&pSyn_List, sip, source);*/

            return process_ip_data(&pUdp_List, sip);
        }


        return NF_ACCEPT;
    }
    return NF_ACCEPT;
}


static struct nf_hook_ops dos_detect_ops =
{
    .hook = dos_detect_fun,
    .owner		= THIS_MODULE,
    .pf		= NFPROTO_IPV4,
    //    .hooknum = NF_INET_PRE_ROUTING,
    .hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FIRST,
}; 


static int __init auth_init(void)
{
    nf_register_hook(&dos_detect_ops);
    return 0;
}

static void __exit auth_eixt(void)
{   
    nf_unregister_hook(&dos_detect_ops);
}

module_init(auth_init);
module_exit(auth_eixt);
MODULE_LICENSE("GPL");

