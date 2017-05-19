#ifndef WORKER_H
#define WORKER_H

struct worker_def {
  const char *task_name;
  void *user_data;
  BOOL (*startup)(void *user_data, ULONG *user_sig_mask);
  void (*shutdown)(void *user_data);
  BOOL (*handle_msg)(struct Message *msg, void *user_data);
  void (*handle_sig)(ULONG sig_mask, void *user_data);
  /* filled in by worker */
  ULONG user_sig_mask;
  struct Library *sys_base;
  struct MsgPort *port;
  struct Task *main_task;
  struct Task *worker_task;
  ULONG term_sig_mask;
  ULONG ack_sig_mask;
  BYTE term_signal;
  BYTE ack_signal;
};
typedef struct worker_def worker_def_t;

extern BOOL worker_start(worker_def_t *def);
extern void worker_stop(worker_def_t *def);

#endif
