#ifndef STATUS_H
#define STATUS_H

#define STATUS_FLAGS_DETACHED   1
#define STATUS_FLAGS_BUSY       2
#define STATUS_FLAGS_EVENTS     4
#define STATUS_FLAGS_PENDING    8
#define STATUS_FLAGS_NONE       0
#define STATUS_FLAGS_NO_MASK    16

#define STATUS_NO_CHANNEL       0xff
#define STATUS_NO_EVENTS        0

typedef struct {
  UBYTE  pending_channel;
  UBYTE  event_mask;
  UBYTE  flags;
  UBYTE  last_state;
  int    last_res;
} status_data_t;

extern void status_init(status_data_t *data);
extern int status_update(proto_handle_t *ph, status_data_t *data);

#endif
