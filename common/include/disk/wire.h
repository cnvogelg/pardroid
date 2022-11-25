#ifndef DISK_WIRE_H
#define DISK_WIRE_H

#define DISK_WIRE_MAX_PACKET_SIZE   32

#define DISK_WIRE_ANY_UNIT          0xff

/* ----- command description for disk handler ----- */

/* mount a disk to a unit (use ANY_UNIT to pick a free unit)

   in:  unit (u08), namelen (u08), name ([u08])
   out: status (u08)
*/
#define DISK_WIRE_CMD_MOUNT             0

/* unmount a disk from a unit

   in:  unit (u08)
   out: status (u08)
*/
#define DISK_WIRE_CMD_UNMOUNT           1

/* get size of mounted disk

   in:  unit (u08)
   out: status (u08),
        num_blocks (u32),
        block_size (u16)
*/
#define DISK_WIRE_CMD_GET_SIZE          2

/* get geometry of mounted disk (if available)

  in:   unit (u08)
  out:  status (u08)
        cylinders (u16), heads (u16), sectors (u16)
*/
#define DISK_WIRE_CMD_GET_GEO           3

/* get max unit

   in:  -
   out: max_units (u08)
*/
#define DISK_WIRE_CMD_GET_MAX_UNITS     4

/* ----- status ----- */

#define DISK_WIRE_STATUS_OK               0
#define DISK_WIRE_STATUS_UNIT_BUSY        1
#define DISK_WIRE_STATUS_MOUNT_NOT_FOUND  2
#define DISK_WIRE_STATUS_UNIT_IDLE        3
#define DISK_WIRE_STATUS_NO_GEO_AVAIL     4

#endif
