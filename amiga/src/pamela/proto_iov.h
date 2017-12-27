#ifndef PROTO_IOV_H
#define PROTO_IOV_H

struct proto_iov_node {
  ULONG       num_words;
  UBYTE      *data;
  struct proto_iov_node  *next;
};
typedef struct proto_iov_node proto_iov_node_t;

/* io vector for vectorized message transfer */
struct proto_iov {
  UWORD       total_words;
  UWORD       extra;
  struct proto_iov_node  first;
};
typedef struct proto_iov proto_iov_t;

#endif
