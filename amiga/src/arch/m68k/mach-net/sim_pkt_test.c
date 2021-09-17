#include <proto/exec.h>
#include <proto/dos.h>
#include <libraries/bsdsocket.h>

#include "sim_pkt.h"

#define MY_PORT 1234
#define BUF_SIZE 2048

static struct sim_pkt_handle hnd;
static struct sim_pkt  sim_pkt;
static struct sim_pkt  cmd_pkt;
static UBYTE send_buf[BUF_SIZE];
static UBYTE recv_buf[BUF_SIZE];

int dosmain(void)
{
  PutStr("sim_pkt_test\n");
  int res = sim_pkt_init(&hnd, BUF_SIZE, MY_PORT);
  if(res < 0) {
    Printf("sim_pkt_init failed: %ld!\n", (LONG)res);
    return 10;
  }
  Printf("port=%ld, buf_size=%ld\n", (LONG)MY_PORT, (LONG)BUF_SIZE);

  while(1) {
    /* receive packet */
    sim_pkt.buf_ptr = recv_buf;
    sim_pkt.buf_size = BUF_SIZE;
    ULONG sig_mask = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
    res = sim_pkt_recv(&hnd, &sim_pkt, &sig_mask);
    Printf("recv: ret=%ld\n", (LONG)res);
    if(res == SIM_PKT_SIGNAL) {
      if(sig_mask & SIGBREAKF_CTRL_C) {
        break;
      } else {
        Printf("SEND\n");
        cmd_pkt.buf_ptr = send_buf;
        cmd_pkt.buf_size = 10;
        cmd_pkt.type = SIM_PKT_TYPE_CMD;
        cmd_pkt.value = 42;
        res = sim_pkt_send(&hnd, &cmd_pkt);
        Printf("send: %ld\n", (LONG)res);
      }
    }
    else if(res == SIM_PKT_OK) {
      if(sim_pkt.type == SIM_PKT_TYPE_STATUS) {
        Printf("STATUS value=%ld\n", (ULONG)sim_pkt.value);
      }
      else if(sim_pkt.type == SIM_PKT_TYPE_CMD) {
        Printf("CMD=%ld size=%ld\n", (ULONG)sim_pkt.value, (ULONG)sim_pkt.buf_size);
      }
      else {
        Printf("??? type=%ld\n", (ULONG)sim_pkt.type);
      }
    }
    else if(res < 0) {
      Printf("ERROR: %ld\n", (ULONG)res);
      break;
    }
  }

  PutStr("exit\n");
  sim_pkt_exit(&hnd);
  PutStr("done\n");

  return 0;
}
