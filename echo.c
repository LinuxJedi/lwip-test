// SPDX-License-Identifier: GPL-3.0-or-later

#include "lwip/debug.h"

#include "lwip/stats.h"

#include "echo.h"

#include "lwip/tcp.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

static void echo_msgclose(struct tcp_pcb *pcb)
{
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_arg(pcb, NULL);
    tcp_close(pcb);
}

static err_t echo_msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
                          err_t err)
{
    if (err == ERR_OK && p != NULL)
    {
        struct pbuf *q;

        for (q = p; q != NULL; q = q->next)
        {
            printf("Got: %.*s\n", q->len, q->payload);
        }
    }
    else if (err == ERR_OK && p == NULL)
    {
        echo_msgclose(pcb);
    }

    return ERR_OK;
}

static err_t echo_msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    return ERR_OK;
}

static void echo_msgerr(void *arg, err_t err)
{
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_msgerr: %s (%i)\n", lwip_strerr(err), err));
    printf("Err: %s\n", lwip_strerr(err));
}

static err_t echo_msgpoll(void *arg, struct tcp_pcb *pcb)
{
    return ERR_OK;
}

static err_t echo_msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    /* Accepted new connection */
    LWIP_PLATFORM_DIAG(("echo_msgaccept called\n"));

    printf("Connect from: %s port: %d\n", ipaddr_ntoa(&(pcb->remote_ip)), pcb->remote_port);

    /* Set an arbitrary pointer for callbacks. */
    //tcp_arg(pcb, esm);

    /* Set TCP receive packet callback. */
    tcp_recv(pcb, echo_msgrecv);

    /* Set a TCP packet sent callback. */
    tcp_sent(pcb, echo_msgsent);

    /* Set an error callback. */
    tcp_err(pcb, echo_msgerr);

    /* Set a TCP poll callback */
    tcp_poll(pcb, echo_msgpoll, 1);

    return ERR_OK;
}

void echo_init(void)
{
    struct tcp_pcb *pcb;

    pcb = tcp_new();
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init: pcb: %x\n", pcb));
    /* Bind port 11111 */
    int r = tcp_bind(pcb, IP_ADDR_ANY, 11111);
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init: tcp_bind: %d\n", r));
    /* Enable listening */
    pcb = tcp_listen(pcb);
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init: listen-pcb: %x\n", pcb));
    /* Set accept message callback */
    tcp_accept(pcb, echo_msgaccept);
}
