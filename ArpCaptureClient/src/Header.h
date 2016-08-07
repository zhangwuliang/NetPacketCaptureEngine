#ifndef __HEADER_H__
#define __HEADER_H__

#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <list>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <iomanip>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <asm/types.h>
#include <linux/netfilter.h>

#include <net/ethernet.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <dlfcn.h>
#include <linux/types.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <list>
#include <sstream>


#ifndef USE_NETINET
    #include <linux/ip.h>
  //  #include <linux/tcp.h>
  //  #include <linux/udp.h>
    #include <linux/if_ether.h>
#else
    #include <netinet/ip.h>
    #include <netinet/ip6.h>
   // #include <netinet/tcp.h>
   // #include <netinet/udp.h>
    #include <netinet/if_ether.h>
#endif


#endif