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
/Ack          | 10  | ACK      | I   | used to trigger the ACK IRQ on falling edge
Busy          | 11  | /RAK     | I   | slave confirms begin/end of command
Paper Out     | 12  | /CLK     | O   | Amiga clocks data/command
Select        | 13  | /PEND    | I   | signal if transfers are pending

Direction is given on Amiga side (O for output and I for input)

### Signals in Off State

If both the Amiga and/or the MCU is disabled then we assume **off state**.
In this state the data lines are high (0xff) and also the control lines.

The control signals in this protocol are designed in a such a way that
the default high state is always the disable state of the signal.

### Initial Signal Setup

The Amiga is configured as follows on protocol setup:

Signal    | Dir | Value | Description
----------|-----|-------|---------------------------
Dx        | O   | 0x00  | set the IDLE command
ACK       | I   | -     | wait for incoming IRQ
RAK       | I   | -     |
CLK       | O   | H     | clock high is default
PEND      | I   | -     |

The MCU is configured as follows:

Signal    | Dir | Value | Description
----------|-----|-------|---------------------------
Dx        | I   | -     | wait for command
ACK       | O   | H     | no ack pulse yet
RAK       | O   | H     | RAK is inactive
CLK       | I   | -     |
PEND      | I   | H     | no pending transfer

## Commands

### Basic Steps

Each transfer with the protocol is performed with a command that the Amiga
sends to the slave. Then this command is processed (either ok or with error)
and then the next command is triggered by the Amiga. The slave never starts
a command directly. He can only request a command by toggling ACK and setting
PEND.

### Command Begin

A command always performs the following steps first:

* Amiga:
  * Set the command number (a byte) on the data port
  * Set CLK to L to signal command
  * Wait for RAK to become L too or if a timeout occurrs
* MCU:
  * Wait for CLK going to L
  * Read command number from data port
  * Set RAK to L to signal we are ready for the command

```
-- Amiga --

Dx:  <XXXXXX><--COMMAND------------>
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
  * Set data port to IDLE command (0x00)
* MCU:
  * Wait for CLK H
  * Set RAK to H

```
-- Amiga --

Dx:  <XXXXXXXXXXXXXXXXXXXXXXX><-00->
                     _______________
CLK: _______________|


-- MCU --
              wait--^     v--set
                           _________
RAK: _____________________|
```

### The Ping Command

The simplest command in the protocol is the **ping** command (0x10).
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

Dx:  <-00-><-10---------------------------><-00-->
                     _______________
CLK: _______________|               |_____________


-- MCU --
                           ______________
RAK: _____________________|              |________
```
