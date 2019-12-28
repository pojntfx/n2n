#include <stdio.h>
#include "n2n.h"

#define N2N_SN_MGMT_PORT 5645

static n2n_sn_t sss_node;
static int keep_running;

int main()
{
    sn_init(&sss_node);

    sss_node.lport = 1234;
    sss_node.daemon = 0; // Don't daemonize

    sss_node.sock = open_socket(sss_node.lport, 1);
    if (-1 == sss_node.sock)
    {
        exit(-2);
    }

    sss_node.mgmt_sock = open_socket(N2N_SN_MGMT_PORT, 0);
    if (-1 == sss_node.mgmt_sock)
    {
        exit(-2);
    }

    keep_running = 1;
    return run_sn_loop(&sss_node, &keep_running);
}