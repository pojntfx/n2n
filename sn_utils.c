#include "n2n.h"
#include "sn_utils.h"

extern int try_forward(n2n_sn_t *sss,
                       const n2n_common_t *cmn,
                       const n2n_mac_t dstMac,
                       const uint8_t *pktbuf,
                       size_t pktsize) {
    struct peer_info *scan;
    struct sn_community *community;
    macstr_t mac_buf;
    n2n_sock_str_t sockbuf;

    HASH_FIND_COMMUNITY(sss->communities, (char *) cmn->community, community);

    if (!community) {
        traceEvent(TRACE_DEBUG, "try_forward unknown community %s", cmn->community);
        return (-1);
    }

    HASH_FIND_PEER(community->edges, dstMac, scan);

    if (NULL != scan) {
        int data_sent_len;
        data_sent_len = sendto_sock(sss, &(scan->sock), pktbuf, pktsize);

        if (data_sent_len == pktsize) {
            ++(sss->stats.fwd);
            traceEvent(TRACE_DEBUG, "unicast %lu to [%s] %s",
                       pktsize,
                       sock_to_cstr(sockbuf, &(scan->sock)),
                       macaddr_str(mac_buf, scan->mac_addr));
        } else {
            ++(sss->stats.errors);
            traceEvent(TRACE_ERROR, "unicast %lu to [%s] %s FAILED (%d: %s)",
                       pktsize,
                       sock_to_cstr(sockbuf, &(scan->sock)),
                       macaddr_str(mac_buf, scan->mac_addr),
                       errno, strerror(errno));
        }
    } else {
        traceEvent(TRACE_DEBUG, "try_forward unknown MAC");

        /* Not a known MAC so drop. */
        return (-2);
    }

    return (0);
}

/** Send a datagram to the destination embodied in a n2n_sock_t.
 *
 *  @return -1 on error otherwise number of bytes sent
 */
extern ssize_t sendto_sock(n2n_sn_t *sss,
                           const n2n_sock_t *sock,
                           const uint8_t *pktbuf,
                           size_t pktsize) {
    n2n_sock_str_t sockbuf;

    if (AF_INET == sock->family) {
        struct sockaddr_in udpsock;

        udpsock.sin_family = AF_INET;
        udpsock.sin_port = htons(sock->port);
        memcpy(&(udpsock.sin_addr.s_addr), &(sock->addr.v4), IPV4_SIZE);

        traceEvent(TRACE_DEBUG, "sendto_sock %lu to [%s]",
                   pktsize,
                   sock_to_cstr(sockbuf, sock));

        return sendto(sss->sock, pktbuf, pktsize, 0,
                      (const struct sockaddr *) &udpsock, sizeof(struct sockaddr_in));
    } else {
        /* AF_INET6 not implemented */
        errno = EAFNOSUPPORT;
        return -1;
    }
}