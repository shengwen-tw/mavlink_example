#include "mavlink_parser.h"
#include "mavlink.h"

extern bool verbose;

void mav_heartbeat(mavlink_message_t *recvd_msg);

/* clang-format off */
enum ENUM_MAV_CMDS {
    ENUM_MAVLINK_HANDLER(mav_heartbeat),
    MAV_CMD_CNT
};
/* clang-format on */

struct mavlink_cmd cmd_list[] = {
    DEF_MAVLINK_CMD(mav_heartbeat, 0),
};

void parse_mavlink_msg(mavlink_message_t *msg)
{
    int i;
    for (i = 0; i < (signed int) CMD_LEN(cmd_list); i++) {
        if (msg->msgid == cmd_list[i].msg_id) {
            cmd_list[i].handler(msg);
            return;
        }
    }

    if (verbose)
        printf("[INFO] received undefined message #%d\n", msg->msgid);
}

void mav_heartbeat(mavlink_message_t *recvd_msg)
{
    if (verbose)
        printf("[INFO] received heartbeat message.\n");
}
