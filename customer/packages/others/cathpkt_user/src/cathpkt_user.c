/***********************************************************************
 *
 *  Copyright (c) 2014  phicomm Corporation
 *  All Rights Reserved
 *
 *  Auth: vity jia
 *  Modified by: ming
 *  Date: 2015/11/25
 *
 ************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <getopt.h>
#include <signal.h>
#include <uci.h>
#include <unistd.h>
#include "cathpkt_user.h"

#define CATHPKT_DEV_NAME 6
#define CATHPKT_MAX_SSID 16
#define CATHPKT_MAX_CHLEN 32
#define CATHPKT_MAX_LINENUM 1024
#define CATHPKT_MAX_URL 128
#define GET_PKT_INFO_LEN 20*1024

char sn[CATHPKT_MAX_CHLEN] = {0};
char FtpUrlorIp[CATHPKT_MAX_URL] = {0};
char user[CATHPKT_MAX_CHLEN] = {0};
char passwd[CATHPKT_MAX_CHLEN] = {0};
char key[CATHPKT_MAX_CHLEN] = {0};
//char ssid[CATHPKT_MAX_CHLEN] = {0};
unsigned int  periodTime = 20;

static int uci_get_ssid_by_dname(char *ssid, char *dname, int len)
{
    static struct uci_context * ctx = NULL;
    static struct uci_package * pkg;
    struct uci_element *e   = NULL;
    const char *value = NULL;
    const char *ifname;

    ctx = uci_alloc_context();
    if (UCI_OK != uci_load(ctx, "wireless", &pkg))
    {
        CATHPKT_DEBUG("Failed to uci load\n");
        uci_free_context(ctx);  
        return -1;
    }
    if (!pkg || !ctx)
    {
        printf("uci context not ready!\n");
        return -1;
    }

    uci_foreach_element(&pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        if (0 == strcmp(s->type, "wifi-iface"))
        {
            if (NULL != (ifname = uci_lookup_option_string(ctx, s, "device")))  
            {
                if (0 == strcmp(ifname, dname))
                {
                    value = uci_lookup_option_string(ctx, s, "ssid"); 
                    if (value)
                        strncpy(ssid, value, len - 1);
                }
            }
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;
}

static int cathpkt_write_pkt_info(char *buf)
{
    unsigned int hostnum = 0;
    char *bfp = buf;
    char mac[32] = {0};
    char timestap[32] = {0};
    char dname[CATHPKT_DEV_NAME] = {0};
    char hurl[128] = {0};
    char ssid[32] = {0};
    char linebuf[128] = {0};
   
    FILE *fp = NULL;
    char *tmp_file;
    FILE *tmp_fp = NULL;

    time_t sec = 0;
    struct tm *tp = NULL;

    int ii = 0;
    
    if(bfp == NULL)
    {
        CATHPKT_DEBUG("buf is NULL.");
        return 0;
    }
    
    hostnum = *(unsigned int *)bfp;
    CATHPKT_DEBUG("hostnum = %d", hostnum);
    bfp += sizeof(unsigned int);

    if(hostnum > 0)
    {
        tmp_file = tempnam("/tmp/", "tmpcathpkt");
        if((tmp_fp = fopen(tmp_file, "w+")) == NULL)
        {
           CATHPKT_DEBUG("Could not open tmp file %s\n", tmp_file);
           return -1;
        }

        for(ii = hostnum; ii > 0; --ii)
        {
            memset(mac, 0, sizeof(mac));
            memset(timestap, 0, sizeof(timestap));
            memset(hurl, 0, sizeof(hurl));
            memset(dname, 0, sizeof(dname));
            memset(ssid, 0, sizeof(ssid));

            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", *(unsigned char*)bfp, *(unsigned char*)(bfp + 1),
              *(unsigned char*)(bfp + 2), *(unsigned char*)(bfp + 3), *(unsigned char*)(bfp + 4), *(unsigned char*)(bfp + 5));
            CATHPKT_DEBUG("mac = %s", mac);
            bfp += 6;
            sec = *(time_t *)bfp;
            tp = localtime(&sec);
            bfp += sizeof(time_t);
            strncpy(dname, bfp, sizeof(dname));
            bfp += 6;
            CATHPKT_DEBUG("dname = %s", dname);
            uci_get_ssid_by_dname(ssid, dname, sizeof(ssid));            
            CATHPKT_DEBUG("ssid = %s\n", ssid);
            strncpy(hurl, bfp + sizeof(unsigned int), *(unsigned int *)bfp);
            CATHPKT_DEBUG("Hurl = %s", hurl);
            bfp += (*(unsigned int *)bfp + sizeof(unsigned int));
            snprintf(timestap, sizeof(timestap), "%02d-%02d-%02d %02d:%02d:%02d", 1900 + tp->tm_year, 1 + tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
            CATHPKT_DEBUG("timestap = %s", timestap);
            fprintf(tmp_fp, "%s;%s;%s;%s;%s%s", mac, sn, ssid, timestap, hurl, "\r\n");
        }
    
        if((fp = fopen("/tmp/cathpkt", "r")) != NULL)
        {                               
            memset(linebuf, 0, sizeof(linebuf));
            while(NULL != fgets(linebuf, 128, fp))
            {
                linebuf[127] = '\0';
                fprintf(tmp_fp, "%s", linebuf);
                memset(linebuf, 0, sizeof(linebuf));
            }        
            fclose(fp);
        }
        else
            CATHPKT_DEBUG("Open /tmp/cathpkt failed.\n");
        
        fclose(tmp_fp);
        rename(tmp_file, "/tmp/cathpkt");
        CATHPKT_DEBUG("tmp_file=%s\n", tmp_file);
    }

    return 0;
}

static int cathpkt_get_host_info()
{
    char catbuf[GET_PKT_INFO_LEN] = {0};
    
    cathpkt_ioctl_get_host_info(catbuf, GET_PKT_INFO_LEN);
    cathpkt_write_pkt_info(catbuf);

    return 0;
}

void cathpkt_report_host_info(void)   
{
    FILE *fp = NULL;
    time_t sec = 0;
    struct tm *tp = NULL;
    char Fname[64] = {0};
    char Hname[64] = {0};
    char cmd[256] = {0};

    if((fp = fopen("/tmp/cathpkt", "r")) != NULL)
    {
        fclose(fp);
        time(&sec);
        tp = localtime(&sec);
        snprintf(Fname, sizeof(Fname), "%02d%02d%02d%02d%02d%02d_SN%s", 1900 + tp->tm_year, 1 + tp->tm_mon, tp->tm_mday, 
            tp->tm_hour, tp->tm_min, tp->tm_sec, sn);
        snprintf(cmd, sizeof(cmd), "cd /tmp/ && cp cathpkt %s.data && tar -zcvf %s.tar.gz %s.data", Fname, Fname, Fname);
        CATHPKT_DEBUG("cmd = %s", cmd);
        system(cmd);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "cd /tmp/ && openssl enc -aes-128-cbc -in %s.tar.gz -out %s.tar.gz.ef -k %s", Fname, Fname, key);
        CATHPKT_DEBUG("cmd = %s", cmd);
        system(cmd);
        memset(cmd, 0, sizeof(cmd));
        snprintf(Hname, sizeof(Hname), "%02d%02d%02d%02d", 1900 + tp->tm_year, 1 + tp->tm_mon, tp->tm_mday, 
            tp->tm_hour);
        snprintf(cmd, sizeof(cmd), "cd /tmp/ && ftpput -u %s -p %s %s /%s/%s.tar.gz.ef %s.tar.gz.ef", user, passwd, FtpUrlorIp, Hname, Fname, Fname);
        CATHPKT_DEBUG("cmd = %s", cmd);
        system(cmd);
        memset(cmd, 'a', sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "cd /tmp/ && rm -rf cathpkt && rm -rf %s.data && rm -rf %s.tar.gz && rm -rf %s.tar.gz.ef", Fname, Fname, Fname);
        system(cmd);
        CATHPKT_DEBUG("cmd = %s", cmd);
    }
    else
    {
        CATHPKT_DEBUG("No pkt info report.\n");
    }
}

static int mknod_cathpkt(void)
{
    FILE *fp = NULL;
    char device[128] = {0},linebuf[128] = {0},cmd[64 ] = {0};
    int  device_num = 0,tmp,i;

    if((fp = fopen("/proc/devices","r")) != NULL)
    {
        memset(linebuf, 0, sizeof(linebuf));
        while(NULL != fgets(linebuf, 128, fp))
        {
            linebuf[127] = '\0';
            tmp = strlen(linebuf);
            if (strstr(linebuf,"cathpkt") != NULL)
            {
                for (i=0 ; i < tmp ; i++)
                {
                        if(isdigit(linebuf[i]))
                        {
                            device[i] = linebuf[i];
                        }
                }
                device_num = atoi(device);
                break;
            }
            memset(linebuf, 0, sizeof(linebuf));
        }
        sprintf(cmd,"mknod /dev/cathpkt c %s 0",device);
        CATHPKT_DEBUG("Device number=%d\tcmd=%s\n", device_num, cmd);
        system(cmd);        
        fclose(fp);
        return 0;        
    }
    else
    {
        CATHPKT_DEBUG("Open /proc/devices failed.\n");
        return -1;
    }
}


int main(int argc, char **argv)
{
    int32_t c = 0;
    int debug = 0;
    int enable = 0;
    static struct uci_context * uci_ctx = NULL;
    static struct uci_package * uci_cathpkt;
    struct uci_element *e   = NULL;
    const char *value;
    int time;

    while ((c = getopt(argc, argv, "d:e:")) != -1)
    {
        switch(c)
        {
            case 'd':
                debug = atoi(optarg);
                CATHPKT_DEBUG("cathpkt user optarg=%s debug=%d \n", optarg, debug );
                cathpkt_debug_set(debug);
                break;

            case 'e':
                enable = atoi(optarg);
                if(enable)
                {
                    CATHPKT_DEBUG("cathpkt ioctl set enabled.\n");
                    cathpkt__ioctl_set_enable(1);
                }
                else
                {
                    CATHPKT_DEBUG("cathpkt ioctl set disabled.\n");
                    cathpkt__ioctl_set_enable(0);
                }
                break;

            default:
                CATHPKT_DEBUG("No such command.\n");
                return -1;
                break;
        }
    }

    if (0 != mknod_cathpkt())
    {
        CATHPKT_DEBUG("Failed to mknod device cathpkt.\n");
        return -1;
    }
    
    uci_ctx = uci_alloc_context();
    if (UCI_OK != uci_load(uci_ctx, "cathpkt", &uci_cathpkt))
    {
        CATHPKT_DEBUG("Failed to uci load\n");
        uci_free_context(uci_ctx);  
        return -1;
    }
    uci_foreach_element(&uci_cathpkt->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        if (NULL != (value = uci_lookup_option_string(uci_ctx, s, "Key")))  
        {
            strncpy(key, value, sizeof(key));
            CATHPKT_DEBUG("key=%s\n", key);
        }
        if (NULL != (value = uci_lookup_option_string(uci_ctx, s, "FtpUrl")))  
        {
            strncpy(FtpUrlorIp, value, sizeof(FtpUrlorIp));
            CATHPKT_DEBUG("FtpUrlorIp=%s\n", FtpUrlorIp);
        }
        if (NULL != (value = uci_lookup_option_string(uci_ctx, s, "FtpUser")))  
        {
            strncpy(user, value, sizeof(user));
            CATHPKT_DEBUG("user=%s\n", user);
        }
        if (NULL != (value = uci_lookup_option_string(uci_ctx, s, "FtpPassword")))  
        {
            strncpy(passwd, value, sizeof(passwd));
            CATHPKT_DEBUG("passwd=%s\n", passwd);
        }
        if (NULL != (value = uci_lookup_option_string(uci_ctx, s, "PeriodTime")))  
        {
            periodTime = atoi(value);
            CATHPKT_DEBUG("periodTime=%d\n", periodTime);
        }
    }
    uci_unload(uci_ctx, uci_cathpkt);
    uci_free_context(uci_ctx);

    while(enable)
    {
        sleep(periodTime);
        cathpkt_get_host_info();
        time++;
        if (time >= 6)
        {
            cathpkt_report_host_info();
            time = 0;
        }
    }
    return 0;
}

