#ifndef PROTO_IOV_H
#define PROTO_IOV_H

struct proto_iov {
  ULONG       num_words;
  UBYTE      *data;
  struct proto_iov  *next;
};
typedef struct proto_iov proto_iov_t;

#endif
