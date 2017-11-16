#ifndef STATUS_H
#define STATUS_H

#define STATUS_FLAGS_ATTACHED   1
#define STATUS_FLAGS_BOOTLOADER 2
#define STATUS_FLAGS_ERROR      4
#define STATUS_FLAGS_PENDING    8
#define STATUS_FLAGS_INIT       0

#define STATUS_NO_CHANNEL       0xff
#define STATUS_NO_ERROR         0

typedef struct {
  UBYTE  pending_channel;
  UBYTE  error;
  UBYTE  flags;
  UBYTE  pad;
} status_data_t;

extern void status_init(status_data_t *data);
extern int status_update(proto_handle_t *ph, status_data_t *data);

#endif
