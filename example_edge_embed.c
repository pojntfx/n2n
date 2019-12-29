#include "n2n.h"

static int keep_on_running;

int main()
{
    n2n_edge_conf_t conf;
    n2n_priv_config_t ec;
    tuntap_dev tuntap;
    n2n_edge_t *eee;
    int rc;

    edge_init_conf_defaults(&conf);
    memset(&ec, 0, sizeof(ec));
    ec.mtu = DEFAULT_MTU;
    ec.daemon = 0;

    // TODO: Demonstrate setting the other options

    snprintf(ec.tuntap_dev_name, sizeof(ec.tuntap_dev_name), "edge0");
    snprintf(ec.ip_mode, sizeof(ec.ip_mode), "static");
    snprintf(ec.netmask, sizeof(ec.netmask), "255.255.255.0");

    if (tuntap_open(&tuntap, ec.tuntap_dev_name, ec.ip_mode, ec.ip_addr, ec.netmask, ec.device_mac, ec.mtu) < 0)
    {
        return (-1);
    }

    eee = edge_init(&tuntap, &conf, &rc);
    if (eee == NULL)
    {
        exit(1);
    }

    keep_on_running = 1;
    rc = run_edge_loop(eee, &keep_on_running);

    edge_term(eee);
    tuntap_close(&tuntap);

    return rc;
}