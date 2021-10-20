// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <pcap/pcap.h>

#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/ethip6.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include <lwip/mld6.h>
#include <lwip/timeouts.h>

#include "echo.h"

int dbg_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vprintf(fmt, args);
    va_end(args);
    return r;
}

static err_t pcap_output(struct netif *netif, struct pbuf *p)
{
    pcap_t *pcap = netif->state;
    //printf("Sending packet with length %d\n", p->tot_len);

    int r = pcap_sendpacket(pcap, (uint8_t *)p->payload, p->tot_len);

    if (r != 0)
    {
        printf("Error sending packet\n");
        printf("Error: %s\n", pcap_geterr(pcap));
        return ERR_IF;
    }

    return ERR_OK;
}

static err_t init_callback(struct netif *netif)
{
    netif->name[0] = 't';
    netif->name[1] = 'p';
    netif->linkoutput = pcap_output;
    netif->output = etharp_output;

    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;

    netif_set_link_up(netif);

    return ERR_OK;
}

int main(size_t argc, char **argv)
{
    pcap_t *pcap = pcap_open_live("eth0", 65536, 1, 100, NULL);
    char errbuf[PCAP_ERRBUF_SIZE];

    struct netif netif;
    memset(&netif, 0, sizeof netif);
    netif.hwaddr_len = 6;
    memcpy(netif.hwaddr, "\xaa\x00\x00\x00\x00\x01", 6);

    /* This is the hard-coded listen IP iaddress */
    ip4_addr_t ip, mask, gw;
    IP4_ADDR(&ip, 172, 17, 0, 5);
    IP4_ADDR(&mask, 255, 255, 0, 0);
    IP4_ADDR(&gw, 172, 17, 0, 1);

    netif_add(&netif, &ip, &mask, &gw, pcap, init_callback, ethernet_input);
    netif_set_up(&netif);

    NETIF_SET_CHECKSUM_CTRL(&netif, 0x00FF);

    echo_init();

    sys_restart_timeouts();

    struct pcap_pkthdr *hdr = NULL;
    const unsigned char *data = NULL;

    while (1)
    {
        sys_check_timeouts();
        int r = pcap_next_ex(pcap, &hdr, &data);

        switch (r)
        {
            case 0:
                // Timeout
                continue;

            case -1:
                printf("Error: %s\n", pcap_geterr(pcap));
                continue;

            case 1:
                break;

            default:
                printf("Unknown result: %d\n", r);
                continue;
        }

        //printf("Packet length: %d / %d\n", hdr->len, hdr->caplen);
        struct pbuf *pbuf = pbuf_alloc(PBUF_RAW, hdr->len, PBUF_RAM);
        memcpy(pbuf->payload, data, hdr->len);
        netif.input(pbuf, &netif);
    }

    return 0;
}

