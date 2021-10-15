#include <proto/exec.h>
#include <proto/dos.h>
#include <libraries/bsdsocket.h>

#include "sim_msg.h"
#include "sim_pkt.h"

#define MY_PORT 1234
#define BUF_SIZE 2048

static sim_pkt_handle_t pkt_hnd;
static sim_msg_handle_t msg_hnd;
static UBYTE rx_buf[BUF_SIZE];
static sim_msg_t *pending_status;
static sim_msg_t *pending_cmd;
static UBYTE status;

static void handle_pkt(sim_pkt_t *pkt)
{
  if(pkt->type == SIM_PKT_TYPE_STATUS) {
    Printf("pkt: STATUS value=%ld\n", (ULONG)pkt->value);
    // update global status
    status = pkt->value;
    // reply pending status request
    if(pending_status != NULL) {
      pending_status->value = status;
      sim_msg_reply(pending_status);
      pending_status = NULL;
      Printf("msg: reply pending status\n");
    }
  }
  else if(pkt->type == SIM_PKT_TYPE_CMD) {
    Printf("pkt: CMD=%ld size=%ld\n", (ULONG)pkt->value, (ULONG)pkt->buf_size);
    // reply pending cmd
    if(pending_cmd != NULL) {
      // check command
      if(pkt->value != pending_cmd->value) {
        Printf("ERROR: wrong command: pending=%ld\n", pending_cmd->value);
      } else {
        if(pkt->buf_size != pending_cmd->rx_size) {
          Printf("ERROR: wrong size: pending=%ld\n", pending_cmd->rx_size);
        } else if(pkt->buf_size > 0) {
          // copy incoming buffer
          memcpy(pending_cmd->data_ptr, pkt->buf_ptr, pkt->buf_size);
        }
        // reply command
        sim_msg_reply(pending_cmd);
        pending_cmd = NULL;
        Printf("msg: reply pending cmd\n");
      }
    } else {
      Printf("ERROR: no command pending!\n");
    }
  }
  else {
    Printf("pkt: ??? type=%ld\n", (ULONG)pkt->type);
  }
}

static void handle_msg(sim_msg_t *msg)
{
  if(msg->type == SM_TYPE_STATUS) {
    Printf("msg: STATUS value=%ld\n", (LONG)msg->value);
    if(pending_status != NULL) {
      Printf("ERROR: status already pending?!\n");
      return;
    }
    UBYTE sender_status = msg->value;
    // if sender_status is not our state then respond now
    if(sender_status != status) {
      msg->value = status;
      sim_msg_reply(msg);
      Printf("msg: immediate status=%ld\n", (LONG)status);
    }
    // pending request until state changes
    else {
      pending_status = msg;
      Printf("msg: pending status\n");
    }
  }
  else if(msg->type = SM_TYPE_CMD) {
    Printf("msg: CMD=%ld tx_size=%ld rx_size=%ld",
           (ULONG)msg->value, (ULONG)msg->tx_size, (ULONG)msg->rx_size);
    if(pending_cmd != NULL) {
      Printf("ERROR: command already pending?!\n");
      return;
    }
    pending_cmd = msg;
    // send cmd pkt
    sim_pkt_t pkt;
    pkt.type = SIM_PKT_TYPE_CMD;
    pkt.value = msg->value;
    pkt.buf_ptr = msg->data_ptr;
    pkt.buf_size = msg->tx_size;
    int res = sim_pkt_send(&pkt_hnd, &pkt);
    Printf("pkt: send cmd: res=%ld\n", (LONG)res);
  }
}

static void main_loop(void)
{
  ULONG msg_sigmask = msg_hnd.sig_mask;
  ULONG break_sigmask = SIGBREAKF_CTRL_C;
  sim_pkt_t pkt;

  // init state
  pending_cmd = NULL;
  pending_status = NULL;
  status = 0;

  while(1) {
    /* handle pkt */
    pkt.buf_ptr = (APTR)rx_buf;
    pkt.buf_size = BUF_SIZE;
    ULONG sig_mask = msg_sigmask | break_sigmask;
    int res = sim_pkt_recv(&pkt_hnd, &pkt, &sig_mask);
    if(res == SIM_PKT_SIGNAL) {
      if(sig_mask & break_sigmask) {
        PutStr("Break**\n");
        break;
      }
      if(sig_mask & msg_sigmask) {
        while(1) {
          sim_msg_t *msg = sim_msg_get(&msg_hnd);
          if(msg == NULL) {
            break;
          }
          handle_msg(msg);
        }
      }
    }
    else if(res == SIM_PKT_OK) {
      handle_pkt(&pkt);
    }
    else if(res < 0) {
      Printf("ERROR: %ld\n", (ULONG)res);
      break;
    }
  }
}

int dosmain(void)
{
  UBYTE *buf = NULL;

  PutStr("Welcome to pario_sim\n");

  /* init sim_pkt */
  int res = sim_pkt_init(&pkt_hnd, BUF_SIZE, MY_PORT);
  if(res == 0) {

    /* init sim msg */
    res = sim_msg_server_init(&msg_hnd);
    if(res == 0) {

      /* main loop */
      main_loop();

      sim_msg_server_exit(&msg_hnd);
    }
    sim_pkt_exit(&pkt_hnd);
  } else {
    Printf("sim_pkt_init failed: %ld!\n", (LONG)res);
    return 10;
  }

  PutStr("Done.\n");
  return 0;
}