#include "tsfi_parc_routing.h"

void tsfi_parc_routing_init(tsfi_parc_routing_table_t *rt) {
    if (!rt) return;
    rt->route_count = 0;
}

int tsfi_parc_routing_add(tsfi_parc_routing_table_t *rt, uint8_t net, uint8_t gateway, uint16_t metric) {
    if (!rt) return -1;

    // Check for existing route and update it
    for (int i = 0; i < rt->route_count; i++) {
        if (rt->routes[i].dest_net == net) {
            rt->routes[i].next_gateway = gateway;
            rt->routes[i].metric = metric;
            return 0;
        }
    }

    if (rt->route_count >= 16) return -2;

    int idx = rt->route_count++;
    rt->routes[idx].dest_net = net;
    rt->routes[idx].next_gateway = gateway;
    rt->routes[idx].metric = metric;
    return 0;
}

int tsfi_parc_routing_lookup(const tsfi_parc_routing_table_t *rt, uint8_t net, uint8_t *gateway_out) {
    if (!rt || !gateway_out) return -1;

    for (int i = 0; i < rt->route_count; i++) {
        if (rt->routes[i].dest_net == net) {
            *gateway_out = rt->routes[i].next_gateway;
            return (int)rt->routes[i].metric;
        }
    }

    return -2; // route not found
}
