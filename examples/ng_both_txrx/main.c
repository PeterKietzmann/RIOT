/*
 * Copyright (C) 2015
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief
 *
 * @author
 *
 * @}
 */

 #include <stdio.h>

#include "ng_at86rf2xx.h"
#include "net/ng_nomac.h"
#include "net/ng_ipv6.h"
#include "net/ng_ipv6/addr.h"
#include "net/ng_netbase.h"
#include "net/ng_udp.h"
#include "net/ng_pktdump.h"
#include "ps.h"


#define IS_SERVER (0)

#if IS_SERVER
static ng_netreg_entry_t server = {NULL, NG_NETREG_DEMUX_CTX_ALL,
                                   KERNEL_PID_UNDEF};
#endif

#define UDP_PAYLOAD_SIZE (20)

#define ATRF_SPI_SPEED          (SPI_SPEED_5MHZ)


static char ipv6_str[NG_IPV6_ADDR_MAX_STR_LEN];


#define MAX_ADDR_LEN            (8U)

static void _print_netconf_state(ng_netconf_state_t state)
{
    switch (state) {
        case NETCONF_STATE_OFF:
            printf("OFF");
            break;
        case NETCONF_STATE_SLEEP:
            printf("SLEEP");
            break;
        case NETCONF_STATE_IDLE:
            printf("IDLE");
            break;
        case NETCONF_STATE_RX:
            printf("RX");
            break;
        case NETCONF_STATE_TX:
            printf("TX");
            break;
        case NETCONF_STATE_RESET:
            printf("RESET");
            break;
        default:
            /* nothing to do then */
            break;
    }
}

static void _netif_list(kernel_pid_t dev)
{
    uint8_t hwaddr[MAX_ADDR_LEN];
    uint16_t u16;
    int16_t i16;
    int res;
    ng_netconf_state_t state;
    ng_netconf_enable_t enable;
    bool linebreak = false;
#ifdef MODULE_NG_IPV6_NETIF
    ng_ipv6_netif_t *entry = ng_ipv6_netif_get(dev);
    char ipv6_addr[NG_IPV6_ADDR_MAX_STR_LEN];
#endif


    printf("Iface %2d  ", dev);

    res = ng_netapi_get(dev, NETCONF_OPT_ADDRESS, 0, hwaddr, sizeof(hwaddr));

    if (res >= 0) {
        char hwaddr_str[res * 3];
        printf(" HWaddr: ");
        printf("%s", ng_netif_addr_to_str(hwaddr_str, sizeof(hwaddr_str),
                                          hwaddr, res));
        printf(" ");
    }

    res = ng_netapi_get(dev, NETCONF_OPT_CHANNEL, 0, &u16, sizeof(u16));

    if (res >= 0) {
        printf(" Channel: %" PRIu16 " ", u16);
    }

    res = ng_netapi_get(dev, NETCONF_OPT_NID, 0, &u16, sizeof(u16));

    if (res >= 0) {
        printf(" NID: 0x%" PRIx16 " ", u16);
    }

    res = ng_netapi_get(dev, NETCONF_OPT_TX_POWER, 0, &i16, sizeof(i16));

    if (res >= 0) {
        printf(" TX-Power: %" PRIi16 "dBm ", i16);
    }

    res = ng_netapi_get(dev, NETCONF_OPT_STATE, 0, &state, sizeof(state));

    if (res >= 0) {
        printf(" State: ");
        _print_netconf_state(state);
    }

    printf("\n           ");

    res = ng_netapi_get(dev, NETCONF_OPT_ADDRESS_LONG, 0, hwaddr, sizeof(hwaddr));

    if (res >= 0) {
        char hwaddr_str[res * 3];
        printf("Long HWaddr: ");
        printf("%s", ng_netif_addr_to_str(hwaddr_str, sizeof(hwaddr_str),
                                          hwaddr, res));
        printf("\n           ");
    }

    res = ng_netapi_get(dev, NETCONF_OPT_PROMISCUOUSMODE, 0, &enable, sizeof(enable));

    if ((res >= 0) && (enable == NETCONF_ENABLE)) {
        printf("PROMISC  ");
        linebreak = true;
    }

    res = ng_netapi_get(dev, NETCONF_OPT_AUTOACK, 0, &enable, sizeof(enable));

    if ((res >= 0) && (enable == NETCONF_ENABLE)) {
        printf("AUTOACK  ");
        linebreak = true;
    }

    res = ng_netapi_get(dev, NETCONF_OPT_PRELOADING, 0, &enable, sizeof(enable));

    if ((res >= 0) && (enable == NETCONF_ENABLE)) {
        printf("PRELOAD  ");
        linebreak = true;
    }

    res = ng_netapi_get(dev, NETCONF_OPT_RAWMODE, 0, &enable, sizeof(enable));

    if ((res >= 0) && (enable == NETCONF_ENABLE)) {
        printf("RAWMODE  ");
        linebreak = true;
    }

#ifdef MODULE_NG_IPV6_NETIF
    if ((entry != NULL) && (entry->flags & NG_IPV6_NETIF_FLAGS_SIXLOWPAN)) {
        printf("6LO ");
        linebreak = true;
    }
#endif

    if (linebreak) {
        printf("\n           ");
    }

    res = ng_netapi_get(dev, NETCONF_OPT_SRC_LEN, 0, &u16, sizeof(u16));

    if (res >= 0) {
        printf("Source address length: %" PRIu16 "\n           ", u16);
    }

#ifdef MODULE_NG_IPV6_NETIF
    for (int i = 0; i < NG_IPV6_NETIF_ADDR_NUMOF; i++) {
        if (!ng_ipv6_addr_is_unspecified(&entry->addrs[i].addr)) {
            printf("inet6 addr: ");

            if (ng_ipv6_addr_to_str(ipv6_addr, &entry->addrs[i].addr,
                                    NG_IPV6_ADDR_MAX_STR_LEN)) {
                printf("%s/%" PRIu8 "  scope: ", ipv6_addr,
                       entry->addrs[i].prefix_len);

                if ((ng_ipv6_addr_is_link_local(&entry->addrs[i].addr))) {
                    printf("local");
                }
                else {
                    printf("global");
                }

                if (entry->addrs[i].flags & NG_IPV6_NETIF_ADDR_FLAGS_NON_UNICAST) {
                    if (ng_ipv6_addr_is_multicast(&entry->addrs[i].addr)) {
                        printf(" [multicast]");
                    }
                    else {
                        printf(" [anycast]");
                    }
                }
            }
            else {
                printf("error in conversion");
            }

            printf("\n           ");
        }
    }
#endif

    puts("");
}

int main(void)
{

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    ng_pktsnip_t *payload_snip, *udp, *ip;
    ng_netreg_entry_t *sendto;
    ng_ipv6_addr_t dest_addr, myaddr;
    memset(&dest_addr, 0, sizeof(ng_ipv6_addr_t));

    uint8_t hwaddr[8];
    int hwaddr_len;
    uint16_t port = 123;
    char payload[UDP_PAYLOAD_SIZE];

    for (int j = 0; j < UDP_PAYLOAD_SIZE; j++){
        payload[j] = j;
    }

    kernel_pid_t ifs[NG_NETIF_NUMOF];
    ng_netif_get(ifs);

    ng_ipv6_netif_get(ifs[0])->flags |= NG_IPV6_NETIF_FLAGS_SIXLOWPAN;

    ng_netapi_get(ifs[0], NETCONF_OPT_ADDRESS, 0, hwaddr, sizeof(hwaddr));

    hwaddr_len = ng_netapi_get(ifs[0], NETCONF_OPT_ADDRESS, 0, hwaddr, sizeof(hwaddr));
/*
    if (ng_ipv6_nc_add(ifs[0], &dest_addr, hwaddr, hwaddr_len, 0) < 0) {
        puts("error: unable to add address to neighbor cache.");
        return 1;
    }
*/
     vtimer_usleep(2000*1000);

    _netif_list(ifs[0]);

#if IS_SERVER
    // check if server is already running
    if (server.pid != KERNEL_PID_UNDEF) {
        printf("Error: server already running on port %" PRIu32 "\n",
                server.demux_ctx);
        return 1;
    }
    server.pid = ng_pktdump_getpid();
    printf("pktdump pid: %i\n", server.pid);
    server.demux_ctx = (uint32_t)port;
    ng_netreg_register(NG_NETTYPE_UDP, &server);
    printf("Success: started UDP server on port %" PRIu16 "\n", port);
#else
    ng_ipv6_addr_set_link_local_prefix(&dest_addr);
    ng_ipv6_addr_set_iid(&dest_addr, 0xfffe009e2a); /* This needs to be adapted. General is 0xfffe00HWADDR */

    int iter = 1;

    while(true) {

        printf("[msg] %d, sending to %s\n", iter, ng_ipv6_addr_to_str(ipv6_str, &(dest_addr), sizeof(ipv6_str)));
        // allocate payload (has no type)
        payload_snip = ng_pktbuf_add(NULL, &payload[0], UDP_PAYLOAD_SIZE, NG_NETTYPE_UNDEF);

        // allocate UDP header, set source port := destination port
        udp = ng_udp_hdr_build(payload_snip, (uint8_t*)&port, sizeof(port), (uint8_t*)&port, sizeof(port));

        // allocate IPv6 header
        ip = ng_ipv6_hdr_build(udp, NULL, 0, (uint8_t *)&dest_addr, sizeof(dest_addr));

        // send packet (NG_NETREG_DEMUX_CTX_ALL: no limitations about the finding)
        sendto = ng_netreg_lookup(NG_NETTYPE_UDP, NG_NETREG_DEMUX_CTX_ALL);

        ng_netapi_send(sendto->pid, ip);

        iter++;

        vtimer_usleep(2000*1000);
    }

#endif

    thread_yield();
    return 0;
}
