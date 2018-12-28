#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
void getmac();
 
int main()
{
    getmac();
    return 0;
}
 
void getmac()
{
#define MAXINTERFACES 16
    int fd, interface;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    char mac[32] = {0};
 
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        int i = 0;
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
        {
            interface = ifc.ifc_len / sizeof(struct ifreq);
            printf("interface num is %d\n", interface);
            while (i < interface)
            {
                printf("net device %s\n", buf[i].ifr_name);
                if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&buf[i])))
                {
                    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[0],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[1],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[2],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[3],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[4],
                        (unsigned char)buf[i].ifr_hwaddr.sa_data[5]);
                    printf("HWaddr %s\n", mac);
                }
                printf("\n");
                i++;
            }
        }
	}
}

