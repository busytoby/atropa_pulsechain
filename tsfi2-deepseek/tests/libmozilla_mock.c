#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint32_t type;
    int32_t param1;
    int32_t param2;
    int32_t param3;
} MozillaEventPacket;

void mozilla_web_render_push_event(const MozillaEventPacket *packet) {
    printf("[Mock Mozilla] Event Received: type=%u, param1=%d, param2=%d, param3=%d\n",
           packet->type, packet->param1, packet->param2, packet->param3);
}
