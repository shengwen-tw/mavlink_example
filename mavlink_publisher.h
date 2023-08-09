#ifndef __MAVLINK_PUBLISHER_H__
#define __MAVLINK_PUBLISHER_H__

#include "mavlink.h"

void mavlink_send_msg(mavlink_message_t *msg);

void mavlink_send_heartbeat(void);
void mavlink_send_play_tune(void);

#endif
