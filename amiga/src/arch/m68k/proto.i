; error/return codes
RET_OK                  equ     0
RET_RAK_INVALID         equ     1
RET_TIMEOUT             equ     2
RET_SLAVE_ERROR         equ     3

CMD_IDLE                equ     0
CMD_PING                equ     $10
CMD_TEST_READ           equ     $11
CMD_TEST_WRITE          equ     $12
