#ifndef PROTO_DEV_H
#define PROTO_DEV_H

INLINE int proto_dev_is_cmd(u08 cmd)
{
  return ((cmd & 0xF0) == 0x10);
}

extern void proto_dev_init(void);
extern void proto_dev_handle(u08 cmd);

#endif
