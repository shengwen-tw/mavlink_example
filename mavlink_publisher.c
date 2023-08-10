#include "mavlink_publisher.h"
#include "common.h"
#include "mavlink.h"

void mavlink_send_heartbeat(void)
{
    uint8_t sys_id = 1;
    uint8_t component_id = 191;
    uint8_t type = MAV_TYPE_GENERIC;
    uint8_t autopilot = MAV_AUTOPILOT_INVALID;
    uint8_t base_mode = 0;
    uint32_t custom_mode = 0;
    uint8_t sys_status = 0;

    mavlink_message_t msg;
    mavlink_msg_heartbeat_pack(sys_id, component_id, &msg, type, autopilot,
                               base_mode, custom_mode, sys_status);
    mavlink_send_msg(&msg);

    printf("[INFO] send heartbeat message.\n");
}

void mavlink_send_play_tune(void)
{
    uint8_t sys_id = 1;
    uint8_t component_id = 191;
    uint8_t target_system = 0;
    uint8_t target_component = 0;
    uint8_t format = 1;
    const char *tune = "T200 L16 O5 A C O6 E O5 A C O6 E O5 A C O6 E";

    mavlink_message_t msg;
    mavlink_msg_play_tune_v2_pack(sys_id, component_id, &msg, target_system,
                                  target_component, format, tune);
    mavlink_send_msg(&msg);

    printf("[INFO] send play_tune_v2 message.\n");
}
