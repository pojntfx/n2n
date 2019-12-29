#include "n2n.h"

static int keep_on_running;

int main()
{
    n2n_edge_conf_t conf;
    n2n_priv_config_t ec;
    tuntap_dev tuntap;
    n2n_edge_t *eee;
    int rc;

    // TODO: Stop using `ec`
    memset(&ec, 0, sizeof(ec));
    ec.daemon = 0;                                                       // Don't daemonize
    snprintf(ec.device_mac, sizeof(ec.device_mac), "DE:AD:BE:EF:01:10"); // Set mac address
    ec.groupid = 0;                                                      // Run as root
    ec.userid = 0;                                                       // Run as root
    snprintf(ec.ip_addr, sizeof(ec.ip_addr), "10.0.0.1");                // Set ip address
    snprintf(ec.ip_mode, sizeof(ec.ip_mode), "static");                  // IP mode; static|dhcp
    ec.mtu = DEFAULT_MTU;                                                // MTU to use
    snprintf(ec.netmask, sizeof(ec.netmask), "255.255.255.0");           // Netmask to use
    snprintf(ec.tuntap_dev_name, sizeof(ec.tuntap_dev_name), "edge0");   // Name of the device to use
    // ec.got_s = 1; // Whether it has a subnet (internal property)

    edge_init_conf_defaults(&conf);
    conf.allow_p2p = 1;                                                                      // Whether to allow peer-to-peer communication
    conf.allow_routing = 1;                                                                  // Whether to allow the edge to route packets to other edges
    snprintf((char *)conf.community_name, sizeof(conf.community_name), "%s", "mycommunity"); // Community to connect to
    conf.disable_pmtu_discovery = 1;                                                         // Whether to disable the path MTU discovery
    conf.drop_multicast = 0;                                                                 // Whether to disable multicast
    conf.dyn_ip_mode = 0;                                                                    // Whether the IP address is set dynamically (see IP mode; 0 if static, 1 if dynamic)
    conf.encrypt_key = "mysecret";                                                           // Secret to decrypt & encrypt with
    conf.local_port = 0;                                                                     // What port to use (0 = any port)
    conf.mgmt_port = N2N_EDGE_MGMT_PORT;                                                     // Edge management port (5644 by default)
    conf.register_interval = 1;                                                              // Interval for both UDP NAT hole punching and supernode registration
    conf.register_ttl = 1;                                                                   // Interval for UDP NAT hole punching through supernode
    // conf.sn_ip_array; // internal property; edge_conf_add_supernode handles this
    // conf.sn_num; // internal property; edge_conf_add_supernode handles this
    edge_conf_add_supernode(&conf, "localhost:1234"); // Supernode to connect to
    conf.tos = 16;                                    // Type of service for sent packets
    conf.transop_id = N2N_TRANSFORM_ID_TWOFISH;       // Use the twofish encryption

    if (edge_verify_conf(&conf) != 0)
    {
        return -1;
    }

    if (tuntap_open(&tuntap, ec.tuntap_dev_name, ec.ip_mode, ec.ip_addr, ec.netmask, ec.device_mac, ec.mtu) < 0)
    {
        return -1;
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