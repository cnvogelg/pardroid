#ifndef ACTION_H
#define ACTION_H

typedef void (*action_func_t)(void);

#define ACTION_FLAG_NONE            0
#define ACTION_FLAG_NO_REPLY        1
#define ACTION_FLAG_END_BEFORE      2

struct action_table_entry {
  action_func_t  func;
  u08            flags;
};
typedef struct action_table_entry action_table_entry_t;

extern const u08 action_table_size ROM_ATTR;
extern const action_table_entry_t action_table[] ROM_ATTR;

#define ACTION_TABLE_SIZE           sizeof(action_table)/sizeof(action_table[0])

#define ACTION_TABLE_BEGIN          const action_table_entry_t action_table[] ROM_ATTR = {
#define ACTION_TABLE_END            }; const u08 action_table_size = ACTION_TABLE_SIZE;

#define ACTION_TABLE_FUNC(x)         { .func = x, .flags = ACTION_FLAG_NONE }
#define ACTION_TABLE_FUNC_FLAGS(x,f) { .func = x, .flags = f }

#define ACTION_PROTO_BOOTLOADER \
  ACTION_TABLE_FUNC_FLAGS(action_nop, ACTION_FLAG_NO_REPLY), \
  ACTION_TABLE_FUNC(action_ping), \
  ACTION_TABLE_FUNC_FLAGS(action_bootloader, ACTION_FLAG_NO_REPLY), \
  ACTION_TABLE_FUNC_FLAGS(action_reset, ACTION_FLAG_END_BEFORE),

extern void action_nop(void);
extern void action_ping(void);
extern void action_bootloader(void);
extern void action_reset(void);
extern void action_attach(void);
extern void action_detach(void);

extern void action_handle(u08 num);

#endif
