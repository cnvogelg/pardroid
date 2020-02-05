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
#include "test.h"
#include "test-buffer.h"

ULONG test_buffer_get_default_size(test_buffer_param_t *param)
{
  /* get buffer size */
  ULONG size = 1024;
  if (param->size != 0)
  {
    size = param->size;
    if (size < 2)
    {
      size = 2;
    }
    if (size & 1 == 1)
    {
      size++;
    }
  }
  return size;
}

ULONG test_buffer_get_adjusted_size(ULONG size, test_buffer_param_t *param)
{
  if (param->add_size != 0)
  {
    size += param->add_size;
  }
  if (param->sub_size != 0)
  {
    size -= param->sub_size;
  }
  return size;
}

static UBYTE get_start_byte(UBYTE bias, UBYTE offset)
{
  /* set start byte value */
  UBYTE start = 0;
  if (bias != 0)
  {
    start = bias;
  }
  start += offset;
  return start;
}

void test_buffer_fill(ULONG size, UBYTE *mem, test_buffer_param_t *bp, test_param_t *p)
{
  /* fill buffer */
  UBYTE data = get_start_byte(bp->bias, (UBYTE)p->iter);
  for (ULONG i = 0; i < size; i++)
  {
    mem[i] = data++;
  }
}

int test_buffer_validate_(ULONG size, UBYTE *mem, test_buffer_param_t *bp, test_param_t *p)
{
  UBYTE data = get_start_byte(bp->bias, (UBYTE)p->iter);
  for (ULONG i = 0; i < size; i++)
  {
    if (mem[i] != data)
    {
      return 1;
    }
    data++;
  }
  return 0;
}

ULONG test_buffer_check(ULONG size, UBYTE *mem1, UBYTE *mem2)
{
  /* check buf */
  ULONG result = size;
  for (ULONG i = 0; i < size; i++)
  {
    if (mem1[i] != mem2[i])
    {
      result = i;
      break;
    }
  }
  return result;
}

UBYTE *test_buffer_alloc(ULONG size, test_param_t *p)
{
  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if (buf == NULL)
  {
    p->error = "out of mem";
    p->section = "buffer alloc";
    return NULL;
  }
  return buf;
}

void test_buffer_free(UBYTE *buf)
{
  FreeVec(buf);
}
