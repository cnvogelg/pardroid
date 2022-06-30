#ifndef PALOMA_WIRE_H
#define PALOMA_WIRE_H

/* wire format description of paloma protocol

   client sends request:

   U08 cmd, u08 zero, u08 len_data,
   ... command request data (in) (payload)

   server responds:

   U08 cmd, U08 status, U08 len_data,
   ... command reply data (out) (payload)

*/

/* protocol parameters */
#define PALOMA_WIRE_MAX_PACKET_SIZE          64
#define PALOMA_WIRE_MAX_PAYLOAD_SIZE         60
#define PALOMA_WIRE_HEADER_SIZE              4

#define PALOMA_WIRE_INVALID_SLOT             0xff

/* --- status --- */
#define PALOMA_WIRE_STATUS_OK                0
#define PALOMA_WIRE_STATUS_NO_CMD            1
#define PALOMA_WIRE_STATUS_DATA_WRONG_SIZE   2
#define PALOMA_WIRE_STATUS_WRONG_SIZE        3
#define PALOMA_WIRE_STATUS_WRONG_SLOT        4
#define PALOMA_WIRE_STATUS_WRONG_TYPE        5

/* --- command list --- */

#define PALOMA_WIRE_CMD_INVALID               0

/* actions
   in:  -
   out: -
 */
#define PALOMA_WIRE_CMD_PARAM_ALL_RESET       1
#define PALOMA_WIRE_CMD_PARAM_ALL_LOAD        2
#define PALOMA_WIRE_CMD_PARAM_ALL_SAVE        3

/* GET_TOTAL
   in:  -
   out: U08 num_slots
*/
#define PALOMA_WIRE_CMD_PARAM_GET_TOTAL_SLOTS 4

/* GET_INFO
   in:  U08 slot
   out: U08 id, U08 type, U08 max_bytes, U08 max_varlen
        U08[] name
*/
#define PALOMA_WIRE_CMD_PARAM_GET_INFO        5

/* FIND_SLOT
   in:  U08 id
   out: U08 slot
*/
#define PALOMA_WIRE_CMD_PARAM_FIND_SLOT       6

/* GET_VALUE
   in:  U08 slot
   out: U08 type, U08 len, U08 data[len]
*/
#define PALOMA_WIRE_CMD_PARAM_GET_VALUE       7

/* SET_VALUE
   in:  U08 slot, U08 type, U08 len, U08 data[len]
   out: U08 got_type
*/
#define PALOMA_WIRE_CMD_PARAM_SET_VALUE       8

/* GET_DEFAULT
   in:  U08 slot
   out: U08 len, U08 data[len]
*/
#define PALOMA_WIRE_CMD_PARAM_GET_DEFAULT     9

/* RESET
   in:  U08 slot
   out: -
*/
#define PALOMA_WIRE_CMD_PARAM_RESET           10

#endif
