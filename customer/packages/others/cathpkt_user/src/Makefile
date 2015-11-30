DLIB = -luci
temp = $(wildcard *~)
all_programs = cathpkt
all:$(all_programs)
.PHONY:all
 
cathpkt:cathpkt_user.c cathpkt_ioctl.c cathpkt_debug.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(DLIB_FLAGS)  $^ -o $@ $(DLIB)
 
clean:
	rm -rf $(temp) $(all_programs)
