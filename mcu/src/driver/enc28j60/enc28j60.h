/*
 * enc28j60.h - pio_dev implementaton for Microchip ENC28J60
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of parbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef ENC28J60_H
#define ENC28J60_H

#define ENC28J60_FLAGS_NONE             0
#define ENC28J60_FLAGS_FULL_DUPLEX      1
#define ENC28J60_FLAGS_BROADCAST        2

#define ENC28J60_RESULT_OK              0
#define ENC28J60_RESULT_NOT_FOUND       1
#define ENC28J60_RESULT_RX_ERROR        2
#define ENC28J60_RESULT_RX_TOO_LARGE    3

extern u08 enc28j60_init(u08 *rev_ret);

extern u08 enc28j60_setup(const u08 macaddr[6], u08 flags);
extern void enc28j60_enable(void);
extern void enc28j60_disable(void);

extern void enc28j60_flow_control(u08 on);
extern u08 enc28j60_is_link_up(void);
extern u08 enc28j60_get_pending_packets(void);

extern void enc28j60_send(const u08 *data, u16 size);
extern void enc28j60_send_begin(void);
extern void enc28j60_send_data(const u08 *data, u16 size);
extern void enc28j60_send_seek(u16 abs_pos);
extern void enc28j60_send_end(u16 total_size);

extern u08 enc28j60_recv(u08 *data, u16 max_size, u16 *got_size);
extern u08 enc28j60_recv_begin(u16 *got_size);
extern void enc28j60_recv_data(u08 *data, u16 size);
extern void enc28j60_recv_seek(u16 abs_pos);
extern void enc28j60_recv_end(void);

extern void enc28j60_test_setup(void);
extern void enc28j60_test_begin_tx(void);
extern void enc28j60_test_end_tx(void);
extern void enc28j60_test_begin_rx(void);
extern void enc28j60_test_end_rx(void);

#endif
