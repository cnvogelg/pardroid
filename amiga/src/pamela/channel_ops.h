#ifndef CHANNEL_OPS_H
#define CHANNEL_OPS_H

extern int channel_op_open(channel_handle_t *ch);
extern int channel_op_close(channel_handle_t *ch);
extern int channel_op_reset(channel_handle_t *ch);
extern int channel_op_transfer(channel_handle_t *ch);
extern int channel_op_cancel(channel_handle_t *ch);
extern int channel_op_poll(channel_handle_t *ch);

#endif
