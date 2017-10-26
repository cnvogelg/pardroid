#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"
#include "proto.h"
#include "proto_shared.h"
#include "reg.h"
#include "offset.h"
#include "test.h"

static UWORD test_size;
static UWORD test_bias;
static UWORD test_add_size;
static UWORD test_sub_size;
static UBYTE test_channel;

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel)
{
  test_size = size;
  test_bias = bias;
  test_add_size = add_size;
  test_sub_size = sub_size;
  test_channel = channel;
}

int test_ping(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_action(pb->proto, PROTO_ACTION_PING);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "ping";
    return res;
  }
}

int test_reset(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_action(pb->proto, PROTO_ACTION_RESET);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }
}

int test_func_write(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v = 0x4711;

  int res = proto_function_write(pb->proto, PROTO_FUNC_REGADDR_SET, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }
  return 0;
}

int test_func_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v;

  int res = proto_function_read(pb->proto, PROTO_FUNC_REGADDR_GET, &v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }
  return 0;
}

int test_func_write_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v = (UWORD)p->iter + test_bias;

  /* write */
  int res = proto_function_write(pb->proto, PROTO_FUNC_REGADDR_SET, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_function_read(pb->proto, PROTO_FUNC_REGADDR_GET, &r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check */
  if(v != r) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", v, r);
    return 1;
  }

  return 0;
}

int test_offset_write_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v = (UWORD)p->iter + test_bias;
  v %= PROTO_MAX_CHANNEL;

  ULONG val = 0xdeadbeef + v;

  /* write offset */
  int res = offset_set(pb->proto, v, val);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "set";
    return res;
  }

  /* read offset */
  ULONG off;
  res = offset_get(pb->proto, v, &off);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get";
    return res;
  }

  /* compare */
  if(val != off) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%08lx, r=%08lx", val, off);
    return 1;
  }

  return 0;
}

int test_msg_empty(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_msg_write_single(pb->proto, test_channel, 0, 0);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 0;
  res = proto_msg_read_single(pb->proto, test_channel, 0, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 0) {
    p->error = "not empty";
    p->section = "compare";
    sprintf(p->extra, "%04x", size);
    return 1;
  }

  return 0;
}

int test_msg_tiny(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG data = 0xdeadbeef;
  int res = proto_msg_write_single(pb->proto, test_channel, (UBYTE *)&data, 2);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 2;
  res = proto_msg_read_single(pb->proto, test_channel, (UBYTE *)&data, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 2) {
    p->error = "not two words";
    p->section = "compare";
    sprintf(p->extra, "%04x", size);
    return 1;
  }

  if(data != 0xdeadbeef) {
    p->error = "invalid value";
    p->section = "compare";
    sprintf(p->extra, "%08lx", data);
    return 1;
  }

  return 0;
}

static ULONG get_default_size(void)
{
  /* get buffer size */
  ULONG size = 256;
  if(test_size != 0) {
    size = test_size;
    if(size < 2) {
      size = 2;
    }
    if(size & 1 == 1) {
      size++;
    }
  }
  return size;
}

static UBYTE get_start_byte(void)
{
  /* set start byte value */
  UBYTE start = 0;
  if(test_bias != 0) {
    start = (UBYTE)test_bias;
  }
  return start;
}

static ULONG get_size(ULONG size)
{
  if(test_add_size != 0) {
    size += test_add_size;
  }
  if(test_sub_size != 0) {
    size -= test_sub_size;
  }
  return size;
}

static void fill_buffer(ULONG size, UBYTE *mem)
{
  /* fill buffer */
  UBYTE data = get_start_byte();
  for(ULONG i=0;i<size;i++) {
    mem[i] = data++;
  }
}

static ULONG check_buffer(ULONG size, UBYTE *mem1, UBYTE *mem2)
{
  /* check buf */
  ULONG result = size;
  for(ULONG i=0;i<size;i++) {
    if(mem1[i] != mem2[i]) {
      result = i;
      break;
    }
  }
  return result;
}

int test_msg_size(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG size = get_default_size();

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  ULONG size_r = get_size(size);
  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    FreeVec(mem_w);
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  UWORD words = size>>1;

  /* send buffer */
  int res = proto_msg_write_single(pb->proto, test_channel, mem_w, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* receive buffer */
  UWORD got_words = size_r>>1;
  res = proto_msg_read_single(pb->proto, test_channel, mem_r, &got_words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf size */
  if(got_words != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", words, got_words);
    return 1;
  }

  /* check buf */
  ULONG pos = check_buffer(size, mem_w, mem_r);
  if(pos < size) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  FreeVec(mem_w);
  FreeVec(mem_r);
  return 0;
}

int test_msg_size_chunks(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG size = get_default_size();
  if(size < 4) {
    size = 4;
  }

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  ULONG size_r = get_size(size);
  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    FreeVec(mem_w);
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  ULONG words = size>>1;
  ULONG c1_words = words>>1;
  ULONG c2_words = words - c1_words;
  UBYTE *c1_buf = mem_w;
  UBYTE *c2_buf = mem_w + (c1_words << 1);

  /* send buffer */
  ULONG msgiov_w[] = {
    words,
    c1_words,
    (ULONG)c1_buf,
    c2_words,
    (ULONG)c2_buf,
    0
  };
  int res = proto_msg_write(pb->proto, test_channel, msgiov_w);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG words_r = size_r>>1;
  c1_buf = mem_r;
  c2_buf = mem_r + (c1_words << 1);
  c2_words = words_r - c1_words;

  /* receive buffer */
  ULONG msgiov_r[] = {
    words_r,
    c1_words,
    (ULONG)c1_buf,
    c2_words,
    (ULONG)c2_buf,
    0
  };
  res = proto_msg_read(pb->proto, test_channel, msgiov_r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }
  UWORD got_words = (UWORD)(msgiov_r[0] & 0xffff);

  /* check buf size */
  if(got_words != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", words, msgiov_r[0]);
    return 1;
  }

  /* check buf */
  ULONG pos = check_buffer(size, mem_w, mem_r);
  if(pos < size) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  FreeVec(mem_w);
  FreeVec(mem_r);
  return 0;
}

int test_msg_write(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG size = get_default_size();

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  UWORD words = size>>1;

  /* send buffer */
  int res = proto_msg_write_single(pb->proto, test_channel, mem_w, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  FreeVec(mem_w);
  return 0;
}

int test_msg_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG size = get_default_size();
  ULONG size_r = get_size(size);

  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  UWORD words = size>>1;

  /* receive buffer */
  UWORD got_words = size_r>>1;
  int res = proto_msg_read_single(pb->proto, test_channel, mem_r, &got_words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  FreeVec(mem_r);
  return 0;
}

#define REG_SIM_PENDING (PROTO_REG_USER + 5)
#define REG_SIM_ERROR   (PROTO_REG_USER + 6)

int test_status_read_pending(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;

  /* assume nothing is pending */
  UBYTE status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active";
    p->section = "pre";
    return 1;
  }

  /* write a message to see it works */
  ULONG data = 0xdeadbeef;
  int res = proto_msg_write_single(pb->proto, test_channel, (UBYTE *)&data, 2);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write msg";
    return res;
  }

  UWORD channel = (p->iter + test_bias) & 7;

  /* sim_pending */
  res = reg_set(pb->proto, REG_SIM_PENDING, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #1";
    return res;
  }

  /* assume pending is set */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == 0) {
    p->error = "pending inactive";
    p->section = "main";
    return 1;
  }

  /* check that channel is set */
  if((status & PROTO_STATUS_CHANNEL_MASK) != channel) {
    p->error = "wrong channel in status";
    p->section = "status";
    sprintf(p->extra, "got=%02x want=%02x", (UWORD)status, channel);
  }

  /* now message write must be aborted due to pending read */
  res = proto_msg_write_single(pb->proto, test_channel, (UBYTE *)&data, 2);
  if(res != PROTO_RET_WRITE_ABORT) {
    p->error = proto_perror(res);
    p->section = "write msg not aborted";
    return 1;
  }

  /* sim_pending with no channel (0xff) */
  res = reg_set(pb->proto, REG_SIM_PENDING, 0xff);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #2";
    return res;
  }

  /* assume pending is cleared again */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active";
    p->section = "post";
    return 1;
  }

  /* message write now works again */
  res = proto_msg_write_single(pb->proto, test_channel, (UBYTE *)&data, 2);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write msg2";
    return res;
  }

  return 0;
}

int test_status_ack_irq(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;

  /* alloc ack signal */
  BYTE ackSig = AllocSignal(-1);
  if(ackSig == -1) {
    p->error = "no signal";
    p->section = "init";
    return 1;
  }

  /* setup ack irq handler */
  struct Task *task = FindTask(NULL);
  int error = pario_setup_ack_irq(pb->pario, task, ackSig);
  if(error) {
    p->error = "setup ack irq";
    p->section = "init";
    return 1;
  }

  /* setup signal timer */
  error = timer_sig_init(pb->timer);
  if(error == -1) {
    p->error = "setup timer";
    p->section = "init";
    return 1;
  }

  /* sim pending */
  int res = reg_set(pb->proto, REG_SIM_PENDING, 0);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #1";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG tmask = timer_sig_get_mask(pb->timer);
  ULONG amask = 1 << ackSig;
  ULONG mask =  tmask | amask;
  timer_sig_start(pb->timer, 1, 0);
  ULONG got = Wait(mask);
  timer_sig_stop(pb->timer);

  /* sim pend_req_rem to restore state */
  res = reg_set(pb->proto, REG_SIM_PENDING, 0xff);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #2";
    return res;
  }

  /* timer cleanup */
  timer_sig_exit(pb->timer);

  /* cleanup ack irq */
  pario_cleanup_ack_irq(pb->pario);

  /* free signal */
  FreeSignal(ackSig);

  if(got & tmask) {
    p->error = "timer triggered";
    p->section = "main";
    return 1;
  }

  if((got & amask) == 0) {
    p->error = "no ack triggered";
    p->section = "main";
    return 1;
  }

  return 0;
}

int test_status_error(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;

  /* assume nothing is pending */
  UBYTE status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #1";
    p->section = "pre";
    return 1;
  }

  /* assume no error is set already */
  if((status & PROTO_STATUS_ERROR) == PROTO_STATUS_ERROR) {
    p->error = "already has error";
    p->section = "pre";
    return 1;
  }

  /* create an error */
  UBYTE error = (UBYTE)(p->iter + test_bias);
  if(error == 0) {
    error = 1;
  }

  /* simulate an error */
  int res = reg_set(pb->proto, REG_SIM_ERROR, error);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_error #1";
    return res;
  }

  /* assume status is set */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #2";
    p->section = "main";
    return 1;
  }
  if((status & PROTO_STATUS_ERROR) == 0) {
    p->error = "no error was set";
    p->section = "main";
    return 1;
  }

  /* get error (and clear it) */
  UWORD result;
  res = reg_get_error(pb->proto, &result);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get_error failed";
    return res;
  }

  /* check if correct error code was returned */
  if(result != error) {
    p->error = "wrong error returned";
    p->section = "main";
    sprintf(p->extra, "got=%02x want=%02x", result, (UWORD)error);
    return 1;
  }

  /* make sure error status is cleared now */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #3";
    p->section = "post";
    return 1;
  }

  /* assume nothing is pending */
  if((status & PROTO_STATUS_ERROR) == PROTO_STATUS_ERROR) {
    p->error = "error was not cleared";
    p->section = "post";
    return 1;
  }

  return 0;
}

int test_status_attach_detach(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;

  /* assume nothing is pending */
  UBYTE status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #1";
    p->section = "pre";
    return 1;
  }

  /* assume not attached */
  if((status & PROTO_STATUS_ATTACHED) == PROTO_STATUS_ATTACHED) {
    p->error = "already is attached";
    p->section = "pre";
    return 1;
  }

  /* attach device */
  int res = proto_action(pb->proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach failed";
    return res;
  }

  /* assume status is set */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #2";
    p->section = "main";
    return 1;
  }
  if((status & PROTO_STATUS_ATTACHED) == 0) {
    p->error = "not attached";
    p->section = "main";
    return 1;
  }

  /* detach device */
  res = proto_action(pb->proto, PROTO_ACTION_DETACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* make sure error status is cleared now */
  status = proto_get_status(pb->proto);
  if((status & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    p->error = "pending active #3";
    p->section = "post";
    return 1;
  }

  /* assume not attached */
  if((status & PROTO_STATUS_ATTACHED) == PROTO_STATUS_ATTACHED) {
    p->error = "still attached";
    p->section = "post";
    return 1;
  }

  return 0;
}
