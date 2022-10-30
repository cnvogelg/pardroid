cmake_minimum_required(VERSION 3.12)

include(scripts/pico_sdk_import.cmake)

project(amiparbox C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

string(TOUPPER ${CMAKE_BUILD_TYPE} FLAVOR)

# Initialize the SDK
pico_sdk_init()

set(PB_PICO_LIBS 
  hardware_spi
  hardware_i2c
  hardware_watchdog
  hardware_gpio
  hardware_uart
  hardware_sync
  pico_stdlib
)

# arch dirs for pico
set(ARCH_BASE_DIR src/arch)
set(ARCH_DIR src/arch/rp2)
set(ARCH_MACH_DIR src/arch/rp2/mach-pico)
set(ARCH_COMMON_DIR src/arch/common)
set(BASE_DIR src/base)
set(TEST_DIR src/test)
set(COMMON_SRC_DIR ../common/src)
set(COMMON_INC_DIR ../common/include)
set(PROTO_DIR src/proto)
set(PAMELA_DIR src/pamela)
set(PAMELA_REQ_DIR src/pamela_req)
set(SDCARD_DIR src/driver/sdcard)
set(FATFS_DIR src/driver/fatfs)
set(FATFS_CONTRIB_DIR src/contrib/fatfs)
set(ENC28J60_DIR src/driver/enc28j60)
set(WIZNET_DIR src/driver/wiznet)
set(DISPLAY_DIR src/driver/display)
set(PALOMA_DIR src/paloma)

# source lists
set(BASE_SRCS
        ${ARCH_DIR}/hw_led.c
        ${ARCH_DIR}/hw_uart.c
        ${ARCH_DIR}/hw_timer.h
        ${ARCH_DIR}/hw_spi.c
        ${ARCH_DIR}/hw_i2c.c
        ${ARCH_DIR}/hw_system.c
        ${ARCH_DIR}/hw_dev.c

        ${ARCH_DIR}/pablo.c
        ${ARCH_DIR}/strobe.c

        ${ARCH_COMMON_DIR}/crc.c
        ${ARCH_COMMON_DIR}/hw_spi_common.c

        ${BASE_DIR}/uartutil.c
        ${BASE_DIR}/rominfo.c
        ${BASE_DIR}/util.c
#        ${BASE_DIR}/mem.c
)

set(PROTO_ATOM_SRCS
        ${ARCH_COMMON_DIR}/proto_low.c
        ${PROTO_DIR}/proto_atom.c
)

set(PROTO_DEV_SRCS
        ${PROTO_ATOM_SRCS}
        ${PROTO_DIR}/proto_dev.c
        ${PROTO_DIR}/knok.c
)

set(PROTO_IO_SRCS
        ${PROTO_DEV_SRCS}
        ${PROTO_DIR}/proto_io.c
)

set(PROTO_BOOT_SRCS
        ${PROTO_ATOM_SRCS}
        ${PROTO_DIR}/proto_dev.c
        ${PROTO_DIR}/proto_boot.c
)

set(PAMELA_SRCS
        ${PROTO_IO_SRCS}
        ${PAMELA_DIR}/pamela.c
        ${PAMELA_DIR}/pamela_io.c
)

set(PAMELA_REQ_SRCS
        ${PROTO_IO_SRCS}
        ${PAMELA_SRCS}
        ${PAMELA_REQ_DIR}/pamela_req.c
)

set(SDCARD_SRCS
      ${SDCARD_DIR}/sdcard.c
      ${SDCARD_DIR}/sdcard.h
      ${SDCARD_DIR}/sdcard_defs.h
)

set(FATFS_SRCS
      ${FATFS_DIR}/fatfs_diskio.c
      ${FATFS_DIR}/ffconf.h
      ${FATFS_CONTRIB_DIR}/ff.c
      ${FATFS_CONTRIB_DIR}/ff.h
      ${FATFS_CONTRIB_DIR}/ffunicode.c
)

set(ENC28J60_SRCS
      ${ENC28J60_DIR}/enc28j60.c
      ${ENC28J60_DIR}/enc28j60.h
      ${ENC28J60_DIR}/enc28j60_regs.h
)

set(WIZNET_SRCS
      ${WIZNET_DIR}/wiz_defs.h
      ${WIZNET_DIR}/wiz_io.c
      ${WIZNET_DIR}/wiz_io.h
      ${WIZNET_DIR}/wiznet.c
      ${WIZNET_DIR}/wiznet.h
)

set(DISPLAY_SRCS
      ${DISPLAY_DIR}/ssd1306/ssd1306.c
      ${DISPLAY_DIR}/ssd1306/ssd1306.h
      ${DISPLAY_DIR}/display.c
      ${DISPLAY_DIR}/display.h
      ${DISPLAY_DIR}/font6x8.c
      ${DISPLAY_DIR}/font6x8.h
)

set(PALOMA_SRCS
      ${PAMELA_REQ_SRCS}
      ${PALOMA_DIR}/paloma.h
      ${PALOMA_DIR}/paloma.c
      ${PALOMA_DIR}/paloma_cmd.h
      ${PALOMA_DIR}/paloma_cmd.c
      ${PALOMA_DIR}/paloma_api.h
)

add_compile_definitions(
        MACHTAG=${MACHTAG}
        VERSION_TAG=${VERSION_TAG}
        FLAVOR_${FLAVOR}=1
)

include_directories(
        ${ARCH_BASE_DIR}
        ${ARCH_DIR}
        ${ARCH_MACH_DIR}
        ${ARCH_COMMON_DIR}
        ${BASE_DIR}
        ${PROTO_DIR}
        ${COMMON_INC_DIR}
        ${PAMELA_DIR}
        ${PAMELA_REQ_DIR}
        ${CMAKE_BINARY_DIR}
        ${SDCARD_DIR}
        ${FATFS_DIR}
        ${FATFS_CONTRIB_DIR}
        ${ENC28J60_DIR}
        ${WIZNET_DIR}
        ${DISPLAY_DIR}
        ${DISPLAY_DIR}/ssd1306
        ${PALOMA_DIR}
)

macro(make_fw name)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} ${PB_PICO_LIBS})
#        pico_enable_stdio_usb(${name} 0)
#        pico_enable_stdio_uart(${name} 1)
        pico_add_extra_outputs(${name})
endmacro()

# ----- firmwares -----
make_fw(test-base ${BASE_SRCS} ${TEST_DIR}/test-base.c)
make_fw(test-proto-atom ${BASE_SRCS} ${PROTO_ATOM_SRCS} ${TEST_DIR}/test-proto-atom.c)
make_fw(test-proto-dev ${BASE_SRCS} ${PROTO_DEV_SRCS} ${TEST_DIR}/test-proto-dev.c)
make_fw(test-proto-io ${BASE_SRCS} ${PROTO_IO_SRCS} ${TEST_DIR}/test-proto-io.c)
make_fw(test-proto-boot ${BASE_SRCS} ${PROTO_BOOT_SRCS} ${TEST_DIR}/test-proto-boot.c)
make_fw(test-pamela ${BASE_SRCS} ${PAMELA_SRCS} ${TEST_DIR}/test-pamela.c)
make_fw(test-pamela-req ${BASE_SRCS} ${PAMELA_REQ_SRCS} ${TEST_DIR}/test-pamela-req.c)
make_fw(test-sdcard ${BASE_SRCS} ${SDCARD_SRCS} ${TEST_DIR}/test-sdcard.c)
make_fw(test-fatfs ${BASE_SRCS} ${SDCARD_SRCS} ${FATFS_SRCS} ${TEST_DIR}/test-fatfs.c)
make_fw(test-enc28j60 ${BASE_SRCS} ${ENC28J60_SRCS} ${TEST_DIR}/test-enc28j60.c)
make_fw(test-wiznet ${BASE_SRCS} ${WIZNET_SRCS} ${TEST_DIR}/test-wiznet.c)
make_fw(test-display ${BASE_SRCS} ${DISPLAY_SRCS} ${TEST_DIR}/test-display.c)
make_fw(test-paloma ${BASE_SRCS} ${PALOMA_SRCS} ${TEST_DIR}/test-paloma.c)
