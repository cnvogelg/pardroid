cmake_minimum_required(VERSION 3.12)

include(scripts/pico_sdk_import.cmake)

project(amiparbox C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

set(PB_PICO_LIBS 
  hardware_spi
  hardware_i2c
  hardware_watchdog
  hardware_gpio
  hardware_uart
  pico_stdlib
)

# arch dirs for pico
set(ARCH_BASE_DIR src/arch)
set(ARCH_DIR src/arch/rp2)
set(ARCH_MACH_DIR src/arch/rp2/mach-pico)
set(ARCH_COMMON_DIR src/arch/common)
set(BASE_DIR src/base)
set(TEST_DIR src/test)
set(COMMON_DIR ../common/src)
set(PROTO_DIR src/proto)

# source lists
set(BASE_SRCS
        ${ARCH_DIR}/hw_led.c
        ${ARCH_DIR}/hw_uart.c
        ${ARCH_DIR}/hw_timer.h
        ${ARCH_DIR}/hw_spi.c
        ${ARCH_DIR}/hw_i2c.c
        ${ARCH_DIR}/hw_system.c

        ${ARCH_DIR}/pablo.c
        ${ARCH_DIR}/strobe.c

        ${ARCH_COMMON_DIR}/crc.c

        ${BASE_DIR}/uartutil.c
        ${BASE_DIR}/rominfo.c
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


add_compile_definitions(
        MACHTAG=${MACHTAG}
        VERSION_TAG=${VERSION_TAG}
)

include_directories(
        ${ARCH_BASE_DIR}
        ${ARCH_DIR}
        ${ARCH_MACH_DIR}
        ${ARCH_COMMON_DIR}
        ${BASE_DIR}
        ${PROTO_DIR}
        ${COMMON_DIR}
        ${CMAKE_BINARY_DIR}
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
