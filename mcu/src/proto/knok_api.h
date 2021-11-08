#ifndef KNOK_API_H
#define KNOK_API_H

typedef u08 (*knok_api_upload_byte_func_t)(void);

extern knok_api_upload_byte_func_t knok_api_upload_boot(u16 *size);
extern knok_api_upload_byte_func_t knok_api_upload_rexx(u16 *size);
extern knok_api_upload_byte_func_t knok_api_upload_rxbt(u16 *size);

#endif