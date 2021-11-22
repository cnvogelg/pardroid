#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "timer.h"
#include "pario.h"
#include "proto_atom.h"
#include "proto_env.h"
#include "proto_atom_test_shared.h"
#include "proto-env-testsuite.h"
#include "test-buffer.h"

#define WAIT_S 0UL
#define WAIT_US 50000UL

static int assert_timer_mask(test_param_t *p, const char *section, proto_env_handle_t *pb, ULONG got)
{
  ULONG timer_mask = proto_env_get_timer_sigmask(pb);

  if (got != timer_mask)
  {
    p->error = "no timer triggered";
    p->section = section;
    sprintf(p->extra, "got=%08lx want=%08lx", got, timer_mask);
    return 1;
  }

  return 0;
}

static int assert_trigger_mask(test_param_t *p, const char *section,
                               proto_env_handle_t *pb, ULONG got)
{
  ULONG trigger_mask = proto_env_get_trigger_sigmask(pb);

  if (got != trigger_mask)
  {
    p->error = "no trigger found";
    p->section = section;
    sprintf(p->extra, "got=%08lx want=%08lx", got, trigger_mask);
    return 1;
  }

  return 0;
}

static int assert_num_triggers(test_param_t *p, const char *section,
                               proto_env_handle_t *pb,
                               UWORD num_triggers, UWORD num_signals)
{
  UWORD got_triggers = proto_env_get_num_triggers(pb);
  UWORD got_signals = proto_env_get_num_trigger_signals(pb);

  /* check number of trigger aka irqs */
  if (num_triggers != got_triggers)
  {
    p->error = "num triggers (ack irqs) mismatch";
    p->section = section;
    sprintf(p->extra, "got=%u want=%u", got_triggers, num_triggers);
    return 1;
  }

  /* check number of signals */
  if (num_signals != got_signals)
  {
    p->error = "num signals mismatch";
    p->section = section;
    sprintf(p->extra, "got=%u want=%u", got_signals, num_signals);
    return 1;
  }

  return 0;
}

static int run_with_events(test_t *t, test_param_t *p, test_func_t func)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  proto_env_handle_t *penv = proto_atom_get_env(proto);

  /* init events */
  int res = proto_env_init_events(penv);
  if (res != PROTO_RET_OK)
  {
    p->error = proto_env_perror(res);
    p->section = "proto_env_init_events";
    return 1;
  }

  /* call test func */
  res = func(t, p);

  /* cleanup events */
  proto_env_exit_events(penv);

  return res;
}

static int run_timer_sig(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  proto_env_handle_t *penv = proto_atom_get_env(proto);

  /* wait for either timeout or ack */
  ULONG got = proto_env_wait_event(penv, WAIT_S, WAIT_US, 0);

  int res = assert_timer_mask(p, "main", penv, got);
  if (res != 0)
  {
    return 1;
  }

  return assert_num_triggers(p, "main", penv, 0, 0);
}

// TEST: timer signal (internal)
int test_timer_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_timer_sig);
}

static int run_event_sig(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  proto_env_handle_t *penv = proto_atom_get_env(proto);

  /* trigger signal */
  int res = proto_atom_action(proto, TEST_PULSE_IRQ);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "action to trigger signal";
    return 1;
  }

  /* wait for either timeout or trigger signal */
  ULONG got = proto_env_wait_event(penv, WAIT_S, WAIT_US, 0);

  res = assert_trigger_mask(p, "main", penv, got);
  if (res != 0)
  {
    return 1;
  }

  return assert_num_triggers(p, "main", penv, 1, 1);
}

// TEST: wait for signal event
int test_event_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_event_sig);
}

static int run_event_sig2(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  proto_env_handle_t *penv = proto_atom_get_env(proto);

  /* trigger signal */
  int res = proto_atom_action(proto, TEST_PULSE_IRQ);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "action to trigger signal";
    return 1;
  }

  /* trigger signal 2 */
  res = proto_atom_action(proto, TEST_PULSE_IRQ);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "action to trigger signal 2";
    return 1;
  }

  /* wait for either timeout or trigger signal */
  ULONG got = proto_env_wait_event(penv, WAIT_S, WAIT_US, 0);

  res = assert_trigger_mask(p, "main", penv, got);
  if (res != 0)
  {
    return 1;
  }

  return assert_num_triggers(p, "main", penv, 2, 1);
}

// TEST: wait for two signal events
int test_event_sig2(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_event_sig2);
}
