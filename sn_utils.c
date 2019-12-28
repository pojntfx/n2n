#include "n2n.h"
#include "sn_utils.h"

extern int try_forward(n2n_sn_t *sss,
                       const n2n_common_t *cmn,
                       const n2n_mac_t dstMac,
                       const uint8_t *pktbuf,
                       size_t pktsize)
{
    struct peer_info *scan;
    struct sn_community *community;
    macstr_t mac_buf;
    n2n_sock_str_t sockbuf;

    HASH_FIND_COMMUNITY(sss->communities, (char *)cmn->community, community);

    if (!community)
    {
        traceEvent(TRACE_DEBUG, "try_forward unknown community %s", cmn->community);
        return (-1);
    }

    HASH_FIND_PEER(community->edges, dstMac, scan);

    if (NULL != scan)
    {
        int data_sent_len;
        data_sent_len = sendto_sock(sss, &(scan->sock), pktbuf, pktsize);

        if (data_sent_len == pktsize)
        {
            ++(sss->stats.fwd);
            traceEvent(TRACE_DEBUG, "unicast %lu to [%s] %s",
                       pktsize,
                       sock_to_cstr(sockbuf, &(scan->sock)),
                       macaddr_str(mac_buf, scan->mac_addr));
        }
        else
        {
            ++(sss->stats.errors);
            traceEvent(TRACE_ERROR, "unicast %lu to [%s] %s FAILED (%d: %s)",
                       pktsize,
                       sock_to_cstr(sockbuf, &(scan->sock)),
                       macaddr_str(mac_buf, scan->mac_addr),
                       errno, strerror(errno));
        }
    }
    else
    {
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
                           size_t pktsize)
{
    n2n_sock_str_t sockbuf;

    if (AF_INET == sock->family)
    {
        struct sockaddr_in udpsock;

        udpsock.sin_family = AF_INET;
        udpsock.sin_port = htons(sock->port);
        memcpy(&(udpsock.sin_addr.s_addr), &(sock->addr.v4), IPV4_SIZE);

        traceEvent(TRACE_DEBUG, "sendto_sock %lu to [%s]",
                   pktsize,
                   sock_to_cstr(sockbuf, sock));

        return sendto(sss->sock, pktbuf, pktsize, 0,
                      (const struct sockaddr *)&udpsock, sizeof(struct sockaddr_in));
    }
    else
    {
        /* AF_INET6 not implemented */
        errno = EAFNOSUPPORT;
        return -1;
    }
}

/** Try and broadcast a message to all edges in the community.
 *
 *  This will send the exact same datagram to zero or more edges registered to
 *  the supernode.
 */
extern int try_broadcast(n2n_sn_t *sss,
                         const n2n_common_t *cmn,
                         const n2n_mac_t srcMac,
                         const uint8_t *pktbuf,
                         size_t pktsize)
{
    struct peer_info *scan, *tmp;
    struct sn_community *community;
    macstr_t mac_buf;
    n2n_sock_str_t sockbuf;

    traceEvent(TRACE_DEBUG, "try_broadcast");

    HASH_FIND_COMMUNITY(sss->communities, (char *)cmn->community, community);

    if (community)
    {
        HASH_ITER(hh, community->edges, scan, tmp)
        {
            if (memcmp(srcMac, scan->mac_addr, sizeof(n2n_mac_t)) != 0)
            {
                /* REVISIT: exclude if the destination socket is where the packet came from. */
                int data_sent_len;

                data_sent_len = sendto_sock(sss, &(scan->sock), pktbuf, pktsize);

                if (data_sent_len != pktsize)
                {
                    ++(sss->stats.errors);
                    traceEvent(TRACE_WARNING, "multicast %lu to [%s] %s failed %s",
                               pktsize,
                               sock_to_cstr(sockbuf, &(scan->sock)),
                               macaddr_str(mac_buf, scan->mac_addr),
                               strerror(errno));
                }
                else
                {
                    ++(sss->stats.broadcast);
                    traceEvent(TRACE_DEBUG, "multicast %lu to [%s] %s",
                               pktsize,
                               sock_to_cstr(sockbuf, &(scan->sock)),
                               macaddr_str(mac_buf, scan->mac_addr));
                }
            }
        }
    }
    else
        traceEvent(TRACE_INFO, "ignoring broadcast on unknown community %s\n",
                   cmn->community);

    return 0;
}

/** Initialise the supernode structure */
extern int init_sn(n2n_sn_t *sss)
{
#ifdef WIN32
    initWin32();
#endif
    memset(sss, 0, sizeof(n2n_sn_t));

    sss->daemon = 1; /* By defult run as a daemon. */
    sss->lport = N2N_SN_LPORT_DEFAULT;
    sss->sock = -1;
    sss->mgmt_sock = -1;

    return 0; /* OK */
}

/** Deinitialise the supernode structure and deallocate any memory owned by
 *  it. */
extern void deinit_sn(n2n_sn_t *sss)
{
    struct sn_community *community, *tmp;

    if (sss->sock >= 0)
    {
        closesocket(sss->sock);
    }
    sss->sock = -1;

    if (sss->mgmt_sock >= 0)
    {
        closesocket(sss->mgmt_sock);
    }
    sss->mgmt_sock = -1;

    HASH_ITER(hh, sss->communities, community, tmp)
    {
        clear_peer_list(&community->edges);
        HASH_DEL(sss->communities, community);
        free(community);
    }
}