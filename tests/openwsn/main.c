/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       OpenWSN test application
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>

#include "od.h"
#include "shell.h"
#include "openwsn.h"
#include "net/ipv6/addr.h"

#include "opendefs.h"
#include "scheduler.h"
#include "02a-MAClow/IEEE802154E.h"
#include "02b-MAChigh/neighbors.h"
#include "03b-IPv6/icmpv6rpl.h"
#include "04-TRAN/openudp.h"
#include "cross-layers/openqueue.h"
#include "cross-layers/idmanager.h"
#include "cross-layers/packetfunctions.h"

extern idmanager_vars_t idmanager_vars;
extern icmpv6rpl_vars_t icmpv6rpl_vars;
extern neighbors_vars_t neighbors_vars;
extern openqueue_vars_t openqueue_vars;
extern schedule_vars_t schedule_vars;
extern scheduler_dbg_t  scheduler_dbg;

udp_resource_desc_t uinject_vars;
char addr_str[IPV6_ADDR_MAX_STR_LEN];

void uinject_sendDone(OpenQueueEntry_t *msg, owerror_t error)
{
    (void)error;

    openqueue_freePacketBuffer(msg);
    puts("Send success");
}

void uinject_receive(OpenQueueEntry_t *pkt)
{
    printf("Received %i bytes on port %i\n", (int)pkt->length, pkt->l4_destination_port);
    od_hex_dump(pkt->payload, pkt->length, OD_WIDTH_DEFAULT);
    openqueue_freePacketBuffer(pkt);
}

void uinject_init(void)
{
    uinject_vars.port = WKP_UDP_INJECT;
    uinject_vars.callbackReceive = &uinject_receive;
    uinject_vars.callbackSendDone = &uinject_sendDone;
    openudp_register(&uinject_vars);
}

char *_array_2_string(const uint8_t *addr, size_t addr_len, char *out)
{
    char *res = out;

    assert((out != NULL) && ((addr != NULL) || (addr_len == 0U)));
    out[0] = '\0';
    for (size_t i = 0; i < addr_len; i++) {
        out += fmt_byte_hex((out), *(addr++));
        *(out++) = (i == (addr_len - 1)) ? '\0' : ':';
    }
    return res;
}

static int ifconfig_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;


    open_addr_t* hwaddr;
    open_addr_t temp_my128bID;

    memcpy(&temp_my128bID.addr_128b[0], &idmanager_vars.myPrefix.prefix, 8);
    memcpy(&temp_my128bID.addr_128b[8], &idmanager_vars.my64bID.addr_64b, 8);

    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)temp_my128bID.addr_128b, sizeof(addr_str));
    printf("inet6 %s\n", addr_str);


    hwaddr = idmanager_getMyID(ADDR_16B);
    printf("hwaddr short: %s", _array_2_string(hwaddr->addr_16b, 2, addr_str));

    hwaddr = idmanager_getMyID(ADDR_64B);
    printf("    long: %s\n", _array_2_string(hwaddr->addr_64b, 8, addr_str));

    hwaddr = idmanager_getMyID(ADDR_PANID);
    printf("panid: %s\n\n", _array_2_string(hwaddr->panid, 2, addr_str));

    printf("IEEE802154E sync: %i\n\n", ieee154e_isSynch());

    if (idmanager_vars.isDAGroot) {
        puts("Node is DAG root");
    }
    else {
        if(icmpv6rpl_vars.haveParent) {
            printf("RPL rank: %i\n", icmpv6rpl_vars.myDAGrank);
            printf("RPL parent: %s\n",\
                _array_2_string(neighbors_vars.neighbors[icmpv6rpl_vars.ParentIndex].addr_64b.addr_64b, 8, addr_str));
            ipv6_addr_to_str(addr_str, (ipv6_addr_t *)icmpv6rpl_vars.dao.DODAGID, sizeof(addr_str));
            printf("RPL DODAG ID: %16s\n", addr_str);
        }
        else {
            puts("NO RPL parent");
        }
    }
    return 0;
}

static int nc_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (int i = 0; i < MAXNUMNEIGHBORS; i++) {
        _array_2_string(neighbors_vars.neighbors[i].addr_64b.addr_64b, 8, addr_str);
        if(memcmp(addr_str, "00:00:00:00:00:00:00:00", 8) != 0) {
            printf("%02i. %s\n", i, addr_str);
        }
    }
    return 0;
}

static int q_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (uint8_t i = 0; i < QUEUELENGTH; i++) {
      printf("Creator: 0x%2x, ", openqueue_vars.queue[i].creator);
      printf("Owner: 0x%2x\n", openqueue_vars.queue[i].owner);
    }

    return 0;
}

static int as_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (int i=0;i<MAXACTIVESLOTS;i++) {
        switch(schedule_vars.scheduleBuf[i].type) {
            case CELLTYPE_TX:
                printf("neigh: %s, slot: %03i, chan: %02i, type: TX\n", \
                    _array_2_string(schedule_vars.scheduleBuf[i].neighbor.addr_64b, 8, addr_str),
                    schedule_vars.scheduleBuf[i].slotOffset, \
                    schedule_vars.scheduleBuf[i].channelOffset);
                break;
            case CELLTYPE_RX:
                    printf("slot: %03i, chan: %02i, type: RX\n", \
                    schedule_vars.scheduleBuf[i].slotOffset, \
                    schedule_vars.scheduleBuf[i].channelOffset);
                break;
            case CELLTYPE_TXRX:
                    printf("neigh: %s, slot: %03i, chan: %02i, type: RXTX\n", \
                    _array_2_string(schedule_vars.scheduleBuf[i].neighbor.addr_64b, 8, addr_str),
                    schedule_vars.scheduleBuf[i].slotOffset, \
                    schedule_vars.scheduleBuf[i].channelOffset);
                break;
            default:
                break;
        }
    }

    return 0;
}

static int rpl_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* TODO allow other prefixes via shell ?!? */
    uint8_t temp[8] = { 0xbb, 0xbb, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00 };
    open_addr_t myPrefix;

    myPrefix.type = ADDR_PREFIX;
    memcpy(myPrefix.prefix, &temp, sizeof(myPrefix.prefix));
    idmanager_setMyID(&myPrefix);
    icmpv6rpl_init();
    idmanager_setIsDAGroot(TRUE);

    return 0;
}

static int sc_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* TODO allow other prefixes via shell ?!? */
    printf("Current tasks:%i\n", scheduler_dbg.numTasksCur);
    printf("Max tasks:    %i\n", scheduler_dbg.numTasksMax);

    return 0;
}

extern int udp_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "ifconfig", "Shows assigned IPv6 addresses", ifconfig_cmd },
    { "nc", "Shows neighbor table", nc_cmd },
    { "q", "Shows Openqueue", q_cmd },
    { "as", "Shows active cells", as_cmd },
    { "sc", "Shows scheduler (openos) dbg states", sc_cmd },
    { "udp", "Send UDP messages and listen for messages on UDP port", udp_cmd },
    { "rplroot", "Set node as RPL DODAG root node", rpl_cmd },
    { NULL, NULL, NULL }
};

#ifdef BTN0_PIN
static void _irq_handler(void *arg)
{
    (void)arg;
    puts("BTN");
}
#endif

int main(void)
{
#ifdef BTN0_PIN
    gpio_init_int(BTN0_PIN, GPIO_IN, GPIO_RISING, _irq_handler, NULL);
#endif
    puts("OpenWSN UDP test");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    openwsn_bootstrap();

    uinject_init();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}
