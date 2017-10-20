# The parbox Protocol

## Introduction

The **parbox** project defines a fast protocol for data exchange with
a MCU (microcontroller) directly attached to the Amiga parallel port.

This document describes the low-level details how the data transfer takes
places. The parallel signals and the steps necessary to exchange data are
given.

The parbox protocol is a **master/slave** protocol with the master being
the Amiga and the slave being the attached MCU. The Amiga master issues
commands and expects the slave to react on these commands. It is assumed
that the slave MCU is fast enough to keep pace with the transfers triggered
by the Amiga.

The speed of the master is limited by the maximum clock rate the Amiga CPU
can access the CIA chips that handle both the data and control lines of the
parallel port. This so called **E Clock** has depending on PAL or NTSC
machines a frequency of `F_ECLK = 716 KHz (NTSC) 709 KHz (PAL)` or a cycle
length of `t_ECLK = 1.40 us (NTSC) 1.41 us (PAL).

We implement a **2E transfer protocol**, i.e. a byte value is transferred
with two accesses to the CIA at E clock speed (one for the data port and
one for a clock line change). Thus we can achieve a maximum data transfer
rate of `358 KB/s (NTSC) 355 KB/s (PAL)`. See [pp_howfast] for more details
on the E transfers.

[pp_howfast]: http://lallafa.de/blog/2015/09/amiga-parallel-port-how-fast-can-you-go/


## Basic Operation Principles

### Message Passing

The main operation in the parpox protocol is message exchange. A message is
an arbitrary block of bytes with a given size (up to 128 KiB). The Amiga
either sends a message for the device to consume or the Amiga reads a message
and expects the device to deliver data.

Since message data may arrive at arbitrary points in time at the device the
protocol allows to signal the master via the slave with an interrupt request.

### Message Multiplexing on 8 Channels

The parbox protocol allows to use 8 distinct channels to send or receive
your messages. This is realized by attaching a channel id to every message
exchange. The MCU then may dispatch the messages to different channel
handlers. E.g. one may send them via Ethernet, the other one can write the
data to an SD card.

### Protocol Actions and Functions

Next to message passing the protocol also allows to trigger **actions**  or
**functions**. While an action is only identified by its unique id (0-7) an
function also either sends or receives a parameter word (16 bits).

Some actions and functions are already pre-defined in the protocol while
others a free to use for custom extensions.

Actions include a ping operation to see if the device is able to talk the
parbox protocol on a very low level, a reset request to reboot the device
or an action to enter the bootloader of the device.

Functions are used to access a set of 16 bit (word) registers on the device
that represent internal state. Furthermore, you can read the last error
value with a function.

### Parbox Register Model

The parbox protocol allows to access up 65536 registers (word) on the device
for both read and write access. Some registers are pre-defined by the
protocol while the others are free for application use.

Some registers are read-only, i.e. a write operation will have no effect.

Parbox registers include a firmware version field, a machine tag to identify
the MCU board and a firmware id.

### Error Modelling and Handling

The parbox code running on the MCU is designed in a such a way that every
error condition that may happen in the parbox protocol, e.g. waiting for
a signal line to become high or low is guarded by a watch dog timer with
a pre-defined timeout (e.g. 500 ms). If the protocol hangs the device
automatically triggers a reset and re-initializes itself.

However, application level errors that only occurred on a single channel may
be reported back to the Amiga and handled appropriately.

### Device Attach/Detach

Since the device may reset unexpectedly or is even powered off during protocol
operation from the Amiga side, the device manages an attach flag that denotes
wether the Amiga driver is using the device or not. During a reset this flag
is also reset and the Amiga driver can detect this by reading this flag in
regular intervals.


## Timings

Here is a list of timing numbers relevant for the protocol.

Description   | Key    | NTSC | PAL | Unit |
--------------|--------|------|-----|------|
E Clock Freq  | F_ECLK | 716 | 709 | kHz   |
E Clock Cycle | t_ECLK | 1.40 | 1.41 | us  |
max Data Rate | r      | 358  | 355 | KB/s |

If we assume a MCU clocked at 16 MHz we can perform these cycles per E clock
when running the protocol:

Description  | Key     | Value      | Unit |
-------------|---------|------------|------|
CPU Freq     | F_CPU   | 16,000,000 | Hz   |
CPU Cycle    | t_CPU   | 62.5       | ns   |
Cycles/EClk  | n       | 1400/62.5=22.4 | - |

So on an AVR RISC CPU we can execute about 22 instructions while the Amiga
executes a single E Clock access.


## Signals

### Definition

The parallel port offers the following signals and here is a mapping to
signals we will use in the protocol:

Parallel Port | Pin | Signal   | Dir | Description
--------------|-----|----------|-----|---------------------------------------
/Strobe       |  1  | n/a      | O   | not used. since too slow for 2E transfers
Data 0-7      | 2-9 | Dx       | I/O | bi-direction data for commands and messages
Data Dir 0-7  |     | DDRx     | I/O | direction of data register
/Ack          | 10  | ACK      | I   | used to trigger the ACK IRQ on falling edge
Busy          | 11  | /RAK     | I   | slave confirms begin/end of command
Paper Out     | 12  | /CLK     | O   | Amiga clocks data/command
Select        | 13  | CFLAG    | O   | extra bit for command

Direction is given on Amiga side (O for output and I for input)

### Signals in Off State

If both the Amiga and/or the MCU is disabled then we assume **off state**.
In this state the data lines are high (0xff) and also the control lines.

The control signals in this protocol are designed in a such a way that
the default high state is always the disable state of the signal.


## Amiga and Device Setup

### Initial Signal Setup

The Amiga is configured as follows on protocol setup:

Signal    | Dir | Value | Description
----------|-----|-------|---------------------------
Dx        | IO  | init  | set the idle command nybble: 0
DDRx      |     | 0x0f  | command nybble: out, status nybble: in
ACK       | I   | -     | wait for incoming IRQ
RAK       | I   | -     |
CLK       | O   | H     | clock high is default
CFLAG     | O   | H     | highest bit of command

The MCU is configured as follows:

Signal    | Dir | Value | Description
----------|-----|-------|---------------------------
Dx        | OI  | -     | set status nybble
DDRx      |     | 0xf0  | command nybble: in, status nybble:
ACK       | O   | H     | no ack pulse yet
RAK       | O   | H     | RAK is inactive
CLK       | I   | -     |
CFLAG     | I   | -     |

Note: The data bits are split in **idle mode**: the lower nibble (bits 0-3)
is called **command nibble** and set by the Amiga to indicate the next
command for the slave device. The high nibble (bits 4-7) is called the
**device status nibble** and set by the MCU to give quick status access
for the Amiga.

### Command Nibble (Bits 0-3) + CFLAG Signal ###

The following (low-level) commands are defined in the parbox protocol:

Both the **command nybble** plus the bit stored in the **CFLAG** signal define
the current command for the device. CFLAG is typically OR'ed in as Bit 4 to
define the **command bits** (short **CB**):

Command Bits (Bits 4-0) | Description
---------------|--------|------------------------------------
00xxx                   | Execute Action xxx (0-7)
01xxx                   | Execute Function xxx (0-7)
10xxx                   | Message Write to Channel xxx (0-7)
11xxx                   | Message Read from Channel xxx (0-7)

Action Table | Description
-------------|------------------------
000 = 0      | IDLE
001 = 1      | PING
010 = 2      | Enter BOOTLOADER
011 = 3      | RESET
100 = 4      | Attach Driver to Device
101 = 5      | Detach Driver from Device
110 = 6      | (Free) User Action #1
111 = 7      | (Free) User Action #2

Function Table | Description
---------------|--------------------
000 = 0        | REGADDR_GET (read)
001 = 1        | REGADDR_SET (write)
010 = 2        | REG_READ (read)
011 = 3        | REG_WRITE (write)
100 = 4        | GET_ERROR (read)
101 = 5        | (Free) User Function #1
110 = 6        | (Free) User Function #2
111 = 7        | (Free) User Function #3

### Device Status Nibble (Bits 4-7) ###

The status nibble is maintained by the MCU and reflects the internal status
of the device.

If Bit 7 is set (read pending) then the lower bits (Bit 4-6) denote the
channel where a message is ready to be read by the Amiga (Short **SB**)

If Bit 7 is cleared (no read pending) then the lower bits (Bit 4-6) are
status bits.

Status Bits (7-4) | Description
------------------|---------------------------------------
1xxx              | Read is Pending for channel xxx (0-7)
0xx0              | Regular device firmware running
0xx1              | Bootloader is running
0x0x              | Driver is not attached to device
0x1x              | Drive is attached  to device
00xx              | No Error is reported by device
01xx              | An Error was reported by device

If an error is reported then no pending requests are signalled until the
error is acknowleged. You confirm an error by calling the GET_ERROR function.
The returned word contains the error code. The device clears the error
code after reading this value. If another read is pending then this signal
is restored.

## Protocol Operation

Each transfer with the protocol is performed with a command that the Amiga
sends to the slave. Then this command is processed (either ok or with error)
and then the next command is triggered by the Amiga. The slave device never
starts a command directly. It can only request a command by toggling ACK and
setting the pending flag.

### Idle Mode

The parbox is considered to be in idle mode if no command is currently started
or running. In this mode the data port is split into **command nibble** (CB)
and **status nibble** (CB). The command bits are set to IDLE command (00000).

The device updates its status nibble whenever its internal state changes, e.g.
on a device reset it clears the attach flag. If an error happens the error
bit will be set.

### Read the Device Status

Reading the device status can be performed at any time by the Amiga driver.
However, since the device may update the state any time asynchronously some
care has to be taken to ensure that the read operation is atomic.

In parbox protocol the read access to the data port for the 4 status bits is
guarded by toggling the CFLAG line from Hi to Lo and back again. This signals
the device not to alter the status while CFLAG is low.

```
-- Amiga: Read Status --
SB:    ---------<READ STATUS>------------
       _______                  _________
CFLAG:        |________________|

```

```
-- MCU: Update Status --

CFLAG: wait for HI ---v
SB:    ----------------<WRITE STATUS>----
```

### Begin a Command

A command always performs the following steps first:

* Amiga:
  * Set the command bits on the data port/cflag line
  * Set CLK to L to signal command
  * Wait for RAK to become L too or if a timeout occurrs
* MCU:
  * Wait for CLK going to L
  * Read command bits from data port/cflag line
  * Set RAK to L to signal we are ready for the command

```
-- Amiga --

CB:  <10000><--COMMAND------------>
     _______________
CLK:                |_______________


-- MCU --
              wait--^        v--set
     ________________________
RAK:                         |______
```

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

### Command End

Each commands ends in a similar way: The Amiga master returns its CLK signal
back to H and waits for the slave to return is RAK back to H, too. Again
a timeout on Amiga side ensures that the MCU reacts as expected.

* Amiga:
  * Set CLK back to H
  * Wait for RAK returning to H (with timeout)
  * Set cmmand bits to (10000)
* MCU:
  * Wait for CLK H
  * Set RAK to H

```
-- Amiga --

CB:  <XXXXXXXXXXXXXXXX><10000>----
                     _____________
CLK: _______________|


-- MCU --
              wait--^     v--set
                           ________
RAK: _____________________|
```

## Actions

### The Ping Action

The simplest command in the protocol is the **ping** action (CB=00001).
It essentially only consists of a command begin sequence directly followed
by an end sequence with no data exchange in between.

The command is very useful to test if the MCU is operable (and even enabled)
to handle commands.

Make sure that the Amiga side uses timeouts when waiting for RAK signal
changes. Otherwise the protocol will block if the MCU is not available or
hangs.

The signal diagram of the ping command looks as follows:

```
-- Amiga --

CB:  <10000><00001=PING-------------------><10000>
     _______________                 _____________
CLK:                |_______________|

-- MCU --
              wait--^      v--set
     _____________________                ________
RAK:                      |______________|
```

### The Enter Bootloader Action

The enter bootloader acion is handled in a special way to allow a bootloader
to stay in this mode. The action is started with the begin command sequence
and with this signals set up the device is reset.

The bootloader starting after the reset will see that the Amiga still sets the
enter bootloader command bits. The device stays in the bootloader and finishes
the command with a command end sequence to tell the Amiga the action is now
completed. Note that the bootloader does not check the CLK line as it might
alredy be gone high again from Amiga side.

Now the parbox protocol is spoken by the bootloader to handle the flash
update of the device (see pablo protocol for more informations). By issuing
a RESET command the device leaves the bootloader and re-enters the
application.

```
-- Amiga --

CB:  <10000><00010=BOOTLOADER-------------------><10000>
     _______________                           ___________
CLK:                |_________________________|

-- MCU --
                  reset device ---v       v--- bootloader checks CB==BOOTLOADER
                                               and completes command
     _____________________                        ________
RAK:                      |_______  ...   _______|
```

## Functions

## Message Exchange

###

