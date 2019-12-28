#ifndef N2N_SN_UTILS_H
#define N2N_SN_UTILS_H

#define HASH_FIND_COMMUNITY(head, name, out) HASH_FIND_STR(head, name, out)
#define N2N_SN_LPORT_DEFAULT 7654
#define N2N_SN_PKTBUF_SIZE 2048

typedef struct sn_stats
{
    size_t errors;         /* Number of errors encountered. */
    size_t reg_super;      /* Number of REGISTER_SUPER requests received. */
    size_t reg_super_nak;  /* Number of REGISTER_SUPER requests declined. */
    size_t fwd;            /* Number of messages forwarded. */
    size_t broadcast;      /* Number of messages broadcast to a community. */
    time_t last_fwd;       /* Time when last message was forwarded. */
    time_t last_reg_super; /* Time when last REGISTER_SUPER was received. */
} sn_stats_t;

typedef struct n2n_sn
{
    time_t start_time; /* Used to measure uptime. */
    sn_stats_t stats;
    int daemon;           /* If non-zero then daemonise. */
    uint16_t lport;       /* Local UDP port to bind to. */
    int sock;             /* Main socket for UDP traffic with edges. */
    int mgmt_sock;        /* management socket. */
    int lock_communities; /* If true, only loaded communities can be used. */
    struct sn_community *communities;
} n2n_sn_t;

struct sn_community
{
    char community[N2N_COMMUNITY_SIZE];
    struct peer_info *edges; /* Link list of registered edges. */

    UT_hash_handle hh; /* makes this structure hashable */
};

extern int try_forward(n2n_sn_t *sss,
                       const n2n_common_t *cmn,
                       const n2n_mac_t dstMac,
                       const uint8_t *pktbuf,
                       size_t pktsize);

extern ssize_t sendto_sock(n2n_sn_t *sss,
                           const n2n_sock_t *sock,
                           const uint8_t *pktbuf,
                           size_t pktsize);

extern int try_broadcast(n2n_sn_t *sss,
                         const n2n_common_t *cmn,
                         const n2n_mac_t srcMac,
                         const uint8_t *pktbuf,
                         size_t pktsize);

extern int init_sn(n2n_sn_t *sss);

extern void deinit_sn(n2n_sn_t *sss);

extern uint16_t reg_lifetime(n2n_sn_t *sss);

extern int update_edge(n2n_sn_t *sss,
                       const n2n_mac_t edgeMac,
                       struct sn_community *comm,
                       const n2n_sock_t *sender_sock,
                       time_t now);

extern int process_mgmt(n2n_sn_t *sss,
                        const struct sockaddr_in *sender_sock,
                        const uint8_t *mgmt_buf,
                        size_t mgmt_size,
                        time_t now);

extern int process_udp(n2n_sn_t *sss,
                       const struct sockaddr_in *sender_sock,
                       const uint8_t *udp_buf,
                       size_t udp_size,
                       time_t now);

extern int run_sn_loop(n2n_sn_t *sss, int *keep_running);

#endif //N2N_SN_UTILS_H
