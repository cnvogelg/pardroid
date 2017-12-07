#ifndef WIZ_IO_H
#define WIZ_IO_H

extern void wiz_io_base_reg_write(u16 addr, u08 value);
extern void wiz_io_base_reg_write_buf(u16 addr, const u08 *buf, u16 len);
extern u08  wiz_io_base_reg_read(u16 addr);
extern void wiz_io_base_reg_read_buf(u16 addr, u08 *buf, u16 len);

extern void wiz_io_socket_reg_write(u08 sock, u16 addr, u08 value);
extern void wiz_io_socket_reg_write_word(u08 sock, u16 addr, u16 value);
extern void wiz_io_socket_reg_write_buf(u08 sock, u16 addr, const u08 *buf, u16 len);
extern u08  wiz_io_socket_reg_read(u08 sock, u16 addr);
extern u16  wiz_io_socket_reg_read_word(u08 sock, u16 addr);
extern void wiz_io_socket_reg_read_buf(u08 sock, u16 addr, u08 *buf, u16 len);

extern u16  wiz_io_get_rx_size(u08 sock);
extern u16  wiz_io_get_tx_free(u08 sock);
extern void wiz_io_tx_buffer_write(u08 sock, u16 offset, const u08 *buf, u16 len);
extern void wiz_io_tx_buffer_confirm(u08 sock, u16 len);
extern void wiz_io_rx_buffer_read(u08 sock, u16 offset, u08 *buf, u16 len);
extern void wiz_io_rx_buffer_confirm(u08 sock, u16 len);

#endif
