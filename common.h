#ifndef __COMMON_H__
#define __COMMON_H__

uint8_t get_sys_id(void);
void mavlink_send_msg(mavlink_message_t *msg);

#endif
