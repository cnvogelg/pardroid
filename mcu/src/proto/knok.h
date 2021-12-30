#ifndef KNOK_H
#define KNOK_H

#define KNOK_KEY_BOOT       0x626f6f74  // 'boot'
#define KNOK_KEY_REXX       0x72657878  // 'rexx'
#define KNOK_KEY_RXBT       0x52584254  // 'RXBT'
#define KNOK_KEY_HELO       0x68656c6f  // 'helo'
#define KNOK_KEY_INFO       0x696e666f  // 'info'

extern void knok_main(void);

// ----- API -----

typedef u08 (*knok_api_upload_byte_func_t)(void);

extern knok_api_upload_byte_func_t knok_api_upload_boot(u16 *size);
extern knok_api_upload_byte_func_t knok_api_upload_rexx(u16 *size);
extern knok_api_upload_byte_func_t knok_api_upload_rxbt(u16 *size);

#endif
