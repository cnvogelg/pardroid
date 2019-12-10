###################
parbox pamela layer
###################

************
Introduction
************

The ``parbox`` project uses multiple abstraction layers to model the
transport of data between the Amiga computer and attached devices to the
microcontroller.

The lowest layer is called ``pamela`` which is short for *parallel messaging
layer* and describes the low-level protocol spoken on the parallel port
between the Amiga and the microcontroller. Next to the protocol description
also the API on both sides is discussed.

Features
========

* master/slave protocol with the Amiga being the master
* maximum transfer speed on parallel port (2E transfers)
* allows to multiplex up to 16 channels on the port
* supports protocol sub set for parallel bootloader
* each channel has fixed MTU and flow control with window size
* multi-tasking friendly to implement on Amiga side
* soft-reset, bootloader, and knok-on-start

********
Protocol
********

The **parbox** project defines a fast protocol for data exchange with
a MCU (microcontroller) directly attached to the Amiga parallel port.

This section describes the low-level details how the data transfer takes
places. The parallel signals and the steps necessary to exchange data are
given.

The parbox protocol is a **master/slave** protocol with the master being
the Amiga and the slave being the attached MCU. The Amiga master issues
commands and expects the slave to react on these commands. It is assumed
that the slave MCU is fast enough to keep pace with the transfers triggered
by the Amiga.


Transfer Speed
==============

The speed of the master is limited by the maximum clock rate the Amiga CPU
can access the CIA chips that handle both the data and control lines of the
parallel port. This so called **E Clock** has depending on PAL or NTSC
machines a frequency of ``F_ECLK = 716 KHz (NTSC) 709 KHz (PAL)`` or a cycle
length of ``t_ECLK = 1.40 us (NTSC) 1.41 us (PAL)``.

We implement a **2E transfer protocol**, i.e. a byte value is transferred
with two accesses to the CIA at E clock speed (one for the data port and
one for a clock line change). Thus we can achieve a maximum data transfer
rate of `358 KB/s (NTSC) 355 KB/s (PAL)`. See `pp_howfast`_ for more details
on the E transfers.

.. _pp_howfast: http://lallafa.de/blog/2015/09/amiga-parallel-port-how-fast-can-you-go/

Parallel Port Timings
---------------------

Here is a list of timing numbers relevant for the parallel port.

=============  ===========  =====  =====  =====
Description    Key          NTSC   PAL    Unit
=============  ===========  =====  =====  =====
E Clock Freq   ``F_ECLK``   716    709    kHz
E Clock Cycle  ``t_ECLK``   1.40   1.41   us
max Data Rate  ``r``        358    355    KB/s
=============  ===========  =====  =====  =====

If we assume a MCU clocked at 16 MHz we can perform these cycles per *E* clock
when running the protocol:

===========  =========  ===============  ====
Description  Key        Value            Unit
===========  =========  ===============  ====
CPU Freq     ``F_CPU``  16,000,000       Hz
CPU Cycle    ``t_CPU``  62.5             ns
Cycles/EClk  ``n``      1400/62.5=22.4   -
===========  =========  ===============  ====

So on an AVR RISC CPU we can execute about 22 instructions while the Amiga
executes a single E Clock access.


Basic Operation Principles
==========================

Message Exchange
----------------

The main operation in the parpox protocol is a *message exchange*. A message
is an arbitrary block of bytes with a given size (up to 128 KiB). The Amiga
either sends a message for the device to consume or the Amiga reads a message
and expects the device to deliver data.

In a *send* operation the Amiga knows the size of the message beforehand and
requests the device to accept the data. If size limits are met the device
will receive the full message. However, it might be possible that the device
rejects the transfer (e.g. due to missing RAM resources for the incoming
message). The protocol needs to handle this situation and report back an
error on Amiga side.

In a *receive* operation the actual message size is not known when the Amiga
initiates a transfer. It will be received first and then the message might
be empty, too large, or of correct size. The protocol needs to handle all
these cases correctly and emit errors if needed.

A message is always sent on a *channel* that allows to direct the message to
a specific *message handler* on the ``parbox`` device.

The maximum allowed size of a message is given by the *maximum transfer unit*
or short **MTU** that is defined per channel.

Since a message handler may receive data from an external source (e.g. via
Ethernet) at any time. The ``parbox`` device must be able to trigger a
*receive pending* interrupt request to the Amiga and notify which channels
have data ready to be transmitted.

Protocol Commands
-----------------

The message transfer is modeled as **commands** in the lowest protocol layer.
Next to message passing the protocol also allows to trigger **actions**  or
**functions**. While an action is only identified by its unique id an
function also either sends or receives a parameter word (16 or 32 bits).

Some actions and functions are already pre-defined in the protocol while
others a free to use for custom extensions.

The commands are designed to be small and minimal blocks of the protocol.
All have a fixed *data direction*: they either send data from the Amiga
or receive data but never change data direction during the command. This
is an important property since we have to switch the data direction of the
parallel port accordingly and this operation always need clean syncing
between both peers to avoid bus contention.

Actions being the simplest commands include a *ping* operation to see if the
device is able to talk the parbox protocol on a very low level, a *reset*
request to reboot the device or an action to *enter the bootloader* of the
device.

Functions are used to access a set of 16 bit (word) registers on the device
that represent internal state. Furthermore, you can read the last error
value with a function.

Pamela Register Model
---------------------

The protocol needs to transfer and store some own information for the whole
device or per channel. This data is not transferred by message exchange
but rather stored in a set of **registers** found in each parbox device.

The **register file** allows to access up 65536 registers (word) on the device
for both read and write access. Some registers store global information of
the device, e.g. the firmware version of per channel data, e.g. the channel's
MTU.

Some registers are read-only, i.e. a write operation will have no effect.

Error Modelling and Handling
----------------------------

The parbox code running on the MCU is designed in a such a way that every
error condition that may happen in the parbox protocol, e.g. waiting for
a signal line to become high or low is guarded by a *watch dog timer* with
a pre-defined timeout (e.g. 500 ms). If the protocol hangs then the device
automatically triggers a reset and re-initializes itself.

However, application level errors that only occurred on a single channel may
be reported back to the Amiga and handled appropriately.

Device Lifecycle
----------------

The ``parbox`` device has a clearly defined lifecylce that states which
operations are allowed in a specific situation.

*Power On*
     After powering on the device it either enters the bootloader if no
     firmware was found or enters *Knok* mode.

*Hardware Reset* or *Watchdog Reset*
     The device enters *Knok* mode.

*Knok Mode*
     Initially the devices stays in the so called *knok mode* and waits for
     a special *knok command* to proceed. This ensures that the main protocol
     loop is not entered before a driver on Amiga side has gained access to
     the parallel port. All other commands are simply rejected.

     This mode allows to support *bootstrapping* the drivers directly from
     the device: Here the device can interact with commands written via the
     ``PAR:`` handler from the Amiga Shell. See `Bootstrapping`_ for details.

*Protocol Loop*
     The main operation mode of the device: It waits for incoming protocol
     commands and processes them. Additionally all message handlers are
     processed to deal with incoming data via attached SPI/I2C devices.
     The time while waiting for a command is called *Idle Mode*.


Parallel Port Signals
=====================

Signal Names
------------

The parallel port offers the following signals and here is a mapping to
signals we will use in the protocol:

==============  ====  =========  ===   ============
Parallel Port   Pin   Signal     Dir   Description
==============  ====  =========  ===   ============
/Strobe          1    n/a        O     not used.
                                       since too slow for 2E transfers
Data 0-7        2-9   Dx         I/O   bi-direction data for
                                       commands and messages
Data Dir 0-7          DDRx       I/O   direction of data register
/Ack            10    ACK        I     used to trigger the ACK IRQ
                                       on falling edge
Busy            11    /RAK       I     slave confirms begin/end of command
                                       or signals device is busy
Paper Out       12    /CLK       O     Amiga clocks data/command
Select          13    PEND       I     pending receive from device
==============  ====  =========  ===   ============

Direction is given on Amiga side (O for output and I for input)

Signals in Off State
--------------------

If both the Amiga and/or the MCU is disabled then we assume **off state**.
In this state the data lines are high (0xff) and also the control lines.

The control signals in this protocol are designed in a such a way that
the default high state is always the disable state of the signal.

Signals in Knok Mode
--------------------

When the device first starts up then it enters *knok* mode. The signals of the
parallel port are set in a such a way to be compatible with a printer or the
``PAR:`` handler of AmigaDOS. This ensures that any other parallel port driver
does not cause any harm to the ``plipbox`` setup. Furthermore, the device
supports `Bootstrapping`_.

==========  ====  ======  ============
Signal      Dir   Value   Description
==========  ====  ======  ============
Paper Out   O     L       paper available
ACK         I     H       no ack pending
Busy        I     L       device is not busy
Select      I     H       device is online
==========  ====  ======  ============

In Knok mode it is essential to trigger an ACK automatically after each
incoming STROBE. Otherwise printer or ``PAR:`` handler will not work.

Signals in Idle Mode
--------------------

When *knok* mode is left then the protocol is setup in *idle mode*, i.e.
it waits for the first command from Amiga side.

The Amiga is configured as follows:

==========  ====  ======  ============
Signal      Dir   Value   Description
==========  ====  ======  ============
Dx          O     init    set the idle command: 0
DDRx              0xFF    output to set commmand
ACK         I     -       wait for incoming IRQ
RAK         I     -
CLK         O     H       clock high is default
PEND        I     -
==========  ====  ======  ============

The MCU is configured as follows:

==========  ====  ======  ============
Signal      Dir   Value   Description
==========  ====  ======  ============
Dx          I     -       set status nybble
DDRx        -     0x00    wait for command byte
ACK         O     H       no ack pulse yet
RAK         O     H       RAK is inactive, device is not busy
CLK         I     -
PEND        O     L       no pending receives
==========  ====  ======  ============


Protocol Commands
=================

A protocol command is written as a byte to the data port of the parallel port.

The following ranges of the byte are defined for the basic operations,
including actions, functions, and message transfer on a channel.

=============  ===  ============
Command Bits   Hex  Description
=============  ===  ============
0000xxxx       00   Reserved
0001xxxx       10   Action xxxx (0-15)
0010xxxx       20   Function Read Word at Register xxxx (0-15)
0011xxxx       30   Function Write Word at Register xxxx (0-15)
0100xxxx       40   Function Read Long at Register xxxx (0-15)
0101xxxx       50   Function Write Long at Register xxxx (0-15)
0110xxxx       60   Message Read from Channel xxx (0-15)
0111xxxx       70   Message Write to Channel xxx (0-15)
10xxxxxx       -    Reserved
11xxxxxx       -    Reserved
=============  ===  ============

Command Execution
=================

Each transfer with the protocol is performed with a command that the Amiga
sends to the slave. Then this command is processed (either ok or with error)
and then the next command is triggered by the Amiga. The slave device never
starts a command directly. It can only request a command by toggling ACK and
setting the pending flag.

Begin of a Command
------------------

A command always performs the following steps first:

* Amiga:
     * Ensure that RAK is H (device is not busy)
     * Set the command bits on the data port lines
     * Set CLK to L to request command begin
     * Wait for RAK to become L too or error if a timeout occurrs
* MCU:
     * Wait for CLK going to L
     * Read command bits from data port lines
     * Set RAK to L to signal we are ready to process the command

The Amiga operations::

     check_rak_hi
     set_cmd(command_bits)
     set_clk_lo
     wait_rak_lo

The operations are a short notation of operations performed on the parallel
port:

     * ``check_rak_hi``
          read ``RAK`` signal and if its not high then abort with error
     * ``set_cmd``
          set the data port to the given command
     * ``set_clk_lo``
          set the ``CLK`` signal to low
     * ``wait_rak_lo``
          wait for the ``RAK`` signal to become low and abort with error if
          a timeout was reached

Note that the device operations are always dual to the Amiga operations:

     * A ``set_clk_lo|hi`` operation is dualed by a ``wait_clk_lo|hi``
     * A ``wait_rak_lo|hi`` operation is dualed by a ``set_rak_lo|hi``

So the corresponding device operations are::

     wait_clk_lo
     set_rak_lo

The signals::

     -- Amiga --

     DDx:  <0x00><--COMMAND------------>
          _______________
     CLK:                |_______________


     -- MCU --
                   wait--^        v--set
          ________________________
     RAK:                         |______

After seting RAK to L both the master (Amiga) and the slave (MCU) are now
**in sync** for processing the command. I.e. the master now starts to clock
out signal changes with up to full E clock speed and the MCU is expected
to react to these signals in time.

The protocol is designed in such a way the the driving force is always the
master E clock and the slave MCU has to keep up with this. In the MCU code
you have to cope with that by using very few cycles for the protocol steps
(i.e. implement it in assembler) and also disable interrupt processing
during the command phase if necessary.

On the other hand the Amiga uses a multi-tasking OS and we do not require
disabling the tasking during the protocol operation. This may result in
delays during command processing and the CIA signal accesses are then not
strictly mapped to a series of E clocks. The protocol copes with these delays
by always enforcing a MCU visible CLK signal change if necessary.

Device is Busy
--------------

The initial check for RAK being inactive (H) before requesting a command
allows to check if the device is busy. In that case the command operation
immediately returns on Amiga side with an error that allows the driver to
repeat the command at a later time.

The device must ensure to enable toggle the RAK line to L for signalling
busy state only if the CLK line is not set (H). Otherwise a pending
command request may be wrongly answered with RAK going low.

Command End
-----------

Each commands ends in a similar way: The Amiga master returns its CLK signal
back to H and waits for the slave to return its RAK signal back to H, too.
Again a timeout on Amiga side ensures that the MCU reacts as expected.

* Amiga:
  * Set CLK back to H
  * Wait for RAK returning to H (with timeout)
  * Set cmmand bits to idle (0x00)
* MCU:
  * Wait for CLK H
  * Set RAK to H

The Amiga operations::

     set_clk_hi
     wait_rak_hi
     set_cmd(0x00)

The device operations::

     wait_clk_hi
     set_rak_hi

The signals::

     -- Amiga --

     DDx:  <XXXXXXXXXXXXXXXX><0x00>----
                         _____________
     CLK: _______________|


     -- MCU --
                   wait--^     v--set
                               ________
     RAK: _____________________|


Actions
=======

The following actions are defined:

=============  ============
Action Table   Description
=============  ============
0000 = 0       PING
0001 = 1       BOOTLOADER
0010 = 2       RESET
=============  ============

The Ping Action (``0x10``)
--------------------------

The simplest command in the protocol is the **ping** action (0000).
It essentially only consists of a command begin sequence directly followed
by an end sequence with no data exchange in between.

The command is very useful to test if the MCU is operable (and even enabled)
to handle commands.

Make sure that the Amiga side uses timeouts when waiting for RAK signal
changes. Otherwise the protocol will block if the MCU is not available or
hangs.

The sequence of Amiga operations::

     # cmd_begin(0x10):
     check_rak_hi
     set_cmd(0x10)
     clk_lo
     wait_rak_lo

     # cmd_end:
     clk_hi
     wait_rak_hi
     set_cmd(0x00)

Or in shorter notation::

     cmd_begin(0x00)
     cmd_end()

The signal diagram of the ping command looks as follows::

     -- Amiga --

     DDx: <0x00><0x10=PING-------------------><0x00>
          _______________                 _____________
     CLK:                |_______________|

     -- MCU --
                   wait--^      v--set   ^--wait v--set
          _____________________                ________
     RAK:                      |______________|

The Enter Bootloader Action (``0x11``)
--------------------------------------

The enter bootloader acion is handled in a special way to allow a bootloader
to stay in this mode. The action is started with the begin command sequence
and with this signals set up the device is reset.

The bootloader starting after the reset will see that the Amiga still sets the
enter bootloader command bits. The device stays in the bootloader and finishes
the command with a command end sequence to tell the Amiga the action is now
completed. Note that the bootloader does not check the CLK line as it might
alredy be gone high again from Amiga side.

Now the parbox protocol is spoken by the bootloader to handle the flash
update of the device (see `Pablo`_ for more informations). By issuing
a RESET command the device leaves the bootloader and re-enters the
application.

With this approach we can assure that the bootloader is ready when the
Amiga is returns from this command.

The signals::

     -- Amiga --

     DDx: <0x00><0x11=BOOTLOADER-------------------><0x00>
          _______________                           ___________
     CLK:                |_________________________|

     -- MCU --
                    reset device ---v       v--- bootloader checks DDx==BOOTLOADER
                                                  and completes command
          _____________________                        ________
     RAK:                      |_______  ...   _______|

The Amiga operations::

     cmd_begin(0x11)
     cmd_end()

A bootloader program must issue the Enter Bootloader action as a first command
to ensure the device can enter its bootloader.

The Reset Action (``0x12``)
---------------------------

The reset action works similar to the bootloader action as it triggers a
device reset after command start and completes the command after device
restart in the new protocol loop.

Since *knok mode* is entered after a reset the mode must be automatically
quit if the reset command byte (0x12) is found on the data lines.

The signals::

     -- Amiga --

     DDx: <0x00><0x12=RESET-------------------------><0x00>
          _______________                           ___________
     CLK:                |_________________________|

     -- MCU --
                    reset device ---v      v--- knok mode is skipped since
                                                DDx == 0x12

                                                      v--- complete command
          _____________________                        ________
     RAK:                      |_______  ...   _______|

The Amiga operations::

     cmd_begin(0x12)
     cmd_end()

An Amiga driver must issue the Reset action as a first operation in order
to initialise the device in the correct state.


Functions
=========

Functions either write or read a 16 bit word or 32 bit long value to/from
the device. Each value is associated with a register at the device numbered
from 0 to 15.

Note that 16 bit and 32 bit registers 0-15 denote different locations.

The following functions are available::

     func_write_word(reg, val)
     val = func_read_word(reg)
     func_write_long(reg, val)
     val = func_read_long(reg

Word Registers
--------------

The following word registers are defined:

=============  ==========  ===  ===============
Word Register  Name        Acc  Description
=============  ==========  ===  ===============
0              GLOB_ADDR   R/W  global parameter pointer
1              GLOB_VALUE  R/W  global parameter data
2              CHAN_ADDR   R/W  channel parameter pointer
3              CHAN_VALUE  R/W  channel parameter data
4              CHAN_STATE  R    channel status: receive ready
=============  ==========  ===  ===============

The first registers allow to access a `global parameter set` in the device
to store and retrieve parameters there. Similary, there is a
`channel parameter set` available for each channel that allows to store data
associated with this channel.

For details on the channel state see `Channel State`_.

Each parameter set has up to 65536 adresses where each address can store a
16 bit word value. The channel parameter pointer uses the highest nybble to
store the channel number and therefore a channel parameter set has only up
to 4096 parameters available.

To access a parameter value first set the pointer register to the desired
address and then either read or write a value to the corresponding value
register.

Access a global parameter::

     param_get_global(addr):
          func_write_word(GLOB_ADDR, addr)
          return func_read_word(GLOB_VALUE)

     param_set_global(addr, val):
          func_write_word(GLOB_ADDR, addr)
          func_write_word(GLOB_VALUE, val)

Access a channel parameter::

     param_get_channel(channel, addr):
          func_write_word(CHAN_ADDR, channel << 24 | addr)
          return func_read_word(CHAN_VALUE)

     param_set_channel(channel, addr, val):
          func_write_word(CHAN_ADDR, channel << 24 | addr)
          func_write_word(CHAN_VALUE, val)

Long Registers
--------------

Currently the long registers are assigned to the channels and represent the
stream offset there.


Write Word Register Function (``0x30+reg``)
-------------------------------------------

This command writes a 16 bit word to the device at the specified register
number 0-15.

The Amiga operations::

     cmd_begin(0x30+reg)

     # hi byte of word value
     set_data(value >> 8)
     set_clk_hi

     # lo byte of word value
     set_data(value & 0xff)
     set_clk_lo

     cmd_end()

With the operations:

     * ``set_data``
          sets the given value on the parallel port

Or in shorter notation::

     cmd_begin(0x30+reg)
     tx_data_hi(value >> 8)
     tx_data_lo(value & 0xff)
     cmd_end()

Read Word Register Function (``0x20+reg``)
------------------------------------------

This command reads a 16 bit word from the device at the specified register
number 0-15.

The Amiga operations::

     cmd_begin(0x30+reg)

     set_ddr_in
     set_clk_hi

     value_hi = get_data()
     set_clk_lo

     value_lo = get_data()
     set_clk_hi

     set_ddr_out
     set_clk_lo

     cmd_end()

     value = value_hi << 8 | value_lo

With the operations:

     * ``set_ddr_in``
          sets the data direction register of the parallel port to input for
          all 8 bits
     * ``set_ddr_out``
          sets the data direction register of the parallel port to output for
          all 8 bits

leading to a shorter notation::

     cmd_begin(0x30+reg)
     ddr_in()
     value_hi = rx_data_lo()
     value_lo = rx_data_hi()
     ddr_out()
     cmd_end()

     value = value_hi << 8 | value_lo

Write Long Register Function (``0x50+reg``)
-------------------------------------------

This command writes a 32 bit word to the device at the specified register
number 0-15.

The Amiga operations::

     cmd_begin(0x50+reg)
     tx_data_hi(value >> 24)
     tx_data_lo(value >> 16)
     tx_data_hi(value >> 8)
     tx_data_lo(value & 0xff)
     cmd_end()

Read Long Register Function (``0x40+reg``)
----------------------------------------------

This command reads a 32 bit word from the device at the specified register
number 0-15.

The Amiga operations::

     cmd_begin(0x40+reg)
     ddr_in()
     value_a = rx_data_lo()
     value_b = rx_data_hi()
     value_c = rx_data_lo()
     value_d = rx_data_hi()
     ddr_out()
     cmd_end()
     value = value_a << 24 | value_b << 16 | value_c << 8 | value_d

Message Transfer
================

The most used operations in the protocol transfer message data blocks of
a given but arbitrary size betweem the Amiga and the device. While sending
messages to the device is controlled by the Amiga, pending input messages
to receive are created asynchronously on the device and reported via a
*receive pending flag* (see `Channel State`_ for details).

Write a Message (``0x70+channel``)
----------------------------------

The Amiga operations::

     cmd_begin(0x70+channel)

     ; send crc
     set_data(crc_hi)
     set_clk_hi
     set_data(crc_lo)
     set_clk_lo

     ; send size
     set_data(size_hi)
     set_clk_hi
     set_data(size_lo)
     set_clk_lo

     ; did slave abort? (message too large)
     check_rak_lo -> goto cmd_end()

     data_loop (size words):
          set_data(odd_byte)
          set_clk_hi
          set_data(even_byte)
          set_clk_lo

     cmd_end()

If the message is too large for the device to handle. It will set its ``RAK``
signal to low directly after the size low byte was pulsed with clock low.

This indicates to the Amiga that the message transfer was aborted and it
terminates the command. Therefore a receiver error can be handled gracefully.

In normal operation this situation should not happen since both sides agreed
on a MTU (maximum transfer unit).

Read a Message (``0x60+channel``)
---------------------------------

The Amiga operations::

     cmd_begin(0x60+channel)

     set_ddr_in
     set_clk_hi

     ; read crc
     crc_hi = get_data()
     set_clk_lo
     crc_lo = get_data()
     set_clk_hi

     ; read size
     size_hi = get_data()
     set_clk_lo
     size_lo = get_data()
     set_clk_hi

     data_loop (size words):
          odd_byte = get_data()
          set_clk_lo
          even_byte = get_data()
          set_clk_hi

     set_ddr_out
     set_clk_lo

     cmd_end()

If a received message is larger than the Amiga size can handle then this
procotol can't report the problem back to the device. The Amiga either reads
the full message in a dummy block or read only up to the max size. By
returning a larger size than allowed the API can signal the receiption error.

In normal operation this situation should not happen since both sides agreed
on a MTU (maximum transfer unit).

Channel State
=============

Parameter Memory
================

Global Parameters
-----------------

Channel Parameters
------------------

*******************
Use of Pamela Layer
*******************

Bootstrapping
=============

Pablo
-----

Applications
============

plipbox
-------

paloma
------

*************************
The AmigaOS device driver
*************************

``pamela.device``
=================

channel parameter
*****************

status: open|attached|error
mtu: current mtu
offset: position in stream (both read/write)

channel properties
******************

CHANNEL_PROP_OFFSET
CHANNEL_PROP_VAR_SIZE

* use offset otherwise stream
* use RX_PENDING otherwise direct read
* use custom size otherwise fixed MTU size

stream I/O
**********

* no position/offset information
* continous data flow
* typically messages of arbitrary size (2..mtu)
* example: network packets, spi data, i2c data

random access I/O
*****************

* position (offset) required
* random access
* typically messages of fixed MTU size (block I/O)
* example: disk i/o

open channel
************

* adjust mtu (only allowed before)
* select channel
WRITE_CHN_INDEX
* get properties and mtu
READ_CHN_MTU
READ_CHN_PROPERTIES
* (optional) adjust MTU and read back
WRITE_CHN_MTU
READ_CHN_MTU
* open
ACTION_CHN_OPEN
   error_code = handler->open(chn)
* check status
READ_CHN_ERROR_CODE

close channel
*************

* select channel
WRITE_CHN_INDEX
* close
ACTION_CHN_CLOSE
   handler->close()
* check status
READ_CHN_ERROR_CODE

write operation
***************

if(CHANNEL_PROP_OFFSET && offset_changed)
     PROTO_CMD_CHN_SET_TX_OFFSET
if(CHANNEL_PROP_VAR_SIZE && size_changed)
     PROTO_CMD_CHN_SET_TX_SIZE
PROTO_CMD_CHN_WRITE_DATA
     handler->write_begin(chn, size, offset);
+ extra WRITE_DATA if size > MTU
     handler->write_next(chn, size)
(may be interrupted by other channels)
     handler->write_end(chn, size)

* sequence is not interrupted!

read operation
**************

if(CHANNEL_PROP_OFFSET && offset_changed)
     PROTO_CMD_CHN_SET_RX_OFFSET
if(CHANNEL_FLAG_RX_SIZE)
     PROTO_CMD_CHN_SET_RX_SIZE
PROTO_CMD_CHN_REQUEST_RX
     handler->read_request(chn, size, offset);

... wait for RX_PENDING event
... other commands allowed even write on channel!
... but no other RX

if(CHANNEL_FLAG_RX_SIZE && size_changed)
     PROTO_CMD_CHN_GET_SIZE
PROTO_CMD_CHN_READ_DATA
     handler->read_begin(chn, size, total_size, offset)
+ extra READ_DATA is size > MTU
     handler->read_next(chn, size)
(may be interrupted by other channels)
     handler->read_end(chn)



--------------------------------

?? READ_REQUEST ()     -> status=RX_REQUEST
                    -> handle->read_request(id, offset)
.
. (might be busy)
. waiting
. in work:
. handler calls channel_send(id, size, buf)   -> status=RX_PENDING
    buf locked
( write may interleave here )
( or other reads )
.
. waiting for master to start reading

+ READ_SIZE         -> read_begin(id, size, buf)
+ READ_DATA
. read multiple data if size > MTU
+ READ_DATA
. done              -> read_done(id, size, buf)
    buf free

+ = atomic block

* via offset (disk i/o)

READ_OFFSET (offset) -> status=RX_REQUEST
                        rx_offset=
                        read_request(id, offset)
.
. (might be busy) 
. waiting
. handler->send()    -> 


+ READ_SIZE          -> read_begin(id, size) -> buf
+ READ_DATA
...
+ READ_DATA           -> read_done(id, size, buf)


writing
*******

* direct (without offset) (network)

+WRITE_SIZE        -> write_begin(size,0) -> buf
+WRITE_DATA
... if size > MTU
+WRITE_DATA        -> write_done()

* with offset (disk i/o)

+WRITE_OFFSET
+WRITE_SIZE        -> write_begin(size,offset)
+WRITE_DATA
... if size > MTU
+WRITE_DATA        -> write_done()
<perform write operation>
(may be busy)
