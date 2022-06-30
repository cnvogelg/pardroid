#ifndef PALOMA_TYPES_H
#define PALOMA_TYPES_H

// types
#define PALOMA_TYPE_UBYTE     0
#define PALOMA_TYPE_UWORD     1
#define PALOMA_TYPE_ULONG     2
#define PALOMA_TYPE_BUFFER    3

// sizes of types
#define PALOMA_TYPE_SIZE_UBYTE    1
#define PALOMA_TYPE_SIZE_UWORD    2
#define PALOMA_TYPE_SIZE_ULONG    4

// hint for buffers
#define PALOMA_TYPE_HINT_NONE     0
#define PALOMA_TYPE_HINT_STRING   1
#define PALOMA_TYPE_HINT_IP_ADDR  2
#define PALOMA_TYPE_HINT_MAC_ADDR 3

// limits
#define PALOMA_TYPE_MAX_NAME_SIZE 16
#define PALOMA_TYPE_MAX_BUFFER_SIZE 60

struct paloma_param_info {
  u08   id; /* unique ID of parameter */
  u08   type; /* type of parameter */
  u08   type_hint; /* display hint of type */
  u08   min_bytes; /* max byte size of type */
  u08   max_bytes; /* max byte size of type */
  u08   name[PALOMA_TYPE_MAX_NAME_SIZE];
};
typedef struct paloma_param_info paloma_param_info_t;

struct paloma_param_buffer {
  u08   length;
  u08   data[PALOMA_TYPE_MAX_BUFFER_SIZE];
};
typedef struct paloma_param_buffer paloma_param_buffer_t;

#endif
