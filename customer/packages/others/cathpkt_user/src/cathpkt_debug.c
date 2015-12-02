#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

//for debug
static int debug = 0;
int8_t cathpkt_debug_set(int dbg)
{
    debug = dbg;
}

void cathpkt_show_debug_info(const char *func, uint32_t lineNum, char *format, ...)
{
    va_list argp;
    char debugbuf[1024] = {0};

    if(debug)
    {
        va_start(argp, format);
        vsprintf(debugbuf, format, argp);
        debugbuf[1023] = '\0';
        printf("%s[%u] %s\n", func, lineNum, debugbuf);
        fflush(stdout);
    }
}

