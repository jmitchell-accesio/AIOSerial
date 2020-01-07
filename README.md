# AIOSerial
How To use ACCES Serial cards in Linux



## Higher baud rate support

Support for baudrates higher than 921600 for ACCES PCI COM cards was added in kernel 5.3. If you require higher baud rate support and are unable to move to kernel 5.3 please contact ACCES for support options.

## Correct number of ports

On some kernel versions more ports would be created than exist on the system. Depending on your distribution the patch to address this may have been backported to your kernel. The additional ports that are created can be ignored. If your design requires the correct number of ports please contact ACCES support.

For example, an mPCIe-COM-2S installs as follows...
```bash
dmesg | grep tty
...
> [    1.488105] 0000:02:00.0: ttyS4 at I/O 0xe000 (irq = 17, base_baud = 921600) is a ST16650
> [    1.490770] 0000:02:00.0: ttyS5 at I/O 0xe008 (irq = 17, base_baud = 921600) is a ST16650
> [    1.493416] 0000:02:00.0: ttyS6 at I/O 0xe010 (irq = 17, base_baud = 921600) is a ST16650
> [    1.496069] 0000:02:00.0: ttyS7 at I/O 0xe018 (irq = 17, base_baud = 921600) is a ST16650
```
... so ttyS6 and ttyS7 should be ignored.

The ttyS devices are created in order, so ttyS4 is Port A and ttyS5 is Port B.

## Fourth port at wrong address.
### This issue only affects four port cards

On some kernel versions the fourth port of a four port ACCES PCI COM card will be set to the wrong address. Depending on your distribution the patch to address this may have been backported to your kernel.

For example, For example, an mPCIe-COM-4SM installs as follows...
```bash
dmesg | grep tty
...
> [    1.488105] 0000:02:00.0: ttyS4 at I/O 0xe000 (irq = 17, base_baud = 921600) is a ST16650
> [    1.490770] 0000:02:00.0: ttyS5 at I/O 0xe008 (irq = 17, base_baud = 921600) is a ST16650
> [    1.493416] 0000:02:00.0: ttyS6 at I/O 0xe010 (irq = 17, base_baud = 921600) is a ST16650
> [    1.496069] 0000:02:00.0: ttyS7 at I/O 0xe018 (irq = 17, base_baud = 921600) is a ST16650
```
...but the I/O address of ttyS7 is 0xe038, so ttyS7 will not work "out of the box" in these kernels

To get the fourth ttyS to work correctly you will need to issue a setserial command ...
```bash
setserial /dev/ttyS7 port 0xe038
```
... where "0xe038" is the address of the first (lowest I/O addressed) port plus 0x0038.  This will result in...
```bash
dmesg | grep tty
...
> [    1.488105] 0000:02:00.0: ttyS4 at I/O 0xe000 (irq = 17, base_baud = 921600) is a ST16650
> [    1.490770] 0000:02:00.0: ttyS5 at I/O 0xe008 (irq = 17, base_baud = 921600) is a ST16650
> [    1.493416] 0000:02:00.0: ttyS6 at I/O 0xe010 (irq = 17, base_baud = 921600) is a ST16650
> [    1.496069] 0000:02:00.0: ttyS7 at I/O 0xe038 (irq = 17, base_baud = 921600) is a ST16650
```
...which is correct.

## Configuring the per-port protocol for RS232, RS422, or RS485 (PCIe only)

Second, you will need to configure your card based on the modes you want to use. A mode would be either RS232, RS485 or RS422.
The configuration of per-port serial protocol is set in the PCI Configuration registers of the UART on the card, and we can read/write this configuration using _setpci_.  

The PCIe UARTs on all ACCES PCIe devices (including mPCIe) have an internal assumption of 8-ports, but provide 1, 2, 4 or 8 actual UARTs to the outside world.  ACCES names these ports by letter, the first UART being "A", the second "B", the fourth "D", and the eigth named "H".

The UART uses a 32-bit register at PCI Configuration address 0xB4 to hold a four-bit (one nybble) protocol configuration setting for each of the ports:
```
|protocol|binary| hex|
|:-------|-----:|---:|
| RS232  | 0000 |  0 |
| RS422  | 0001 |  1 |
| RS485  | 1111 |  F |
```
###Configuring an 8-port card's per-port protocol
The setpci instruction to set all 8 ports to RS232...
```bash
setpci -s 02:00.0 B4.L=00000000
```
...this writes the Long value "0" to register 0xB4 in the PCI configuration space of the device at 02:00.0 (as found in the `dmesg | grep tty` output above).  Substitute the appropriate device identifier in your usage.

The 32-bit register holds 8 nybbles, one per port, with A being the least-significant nybble and H being the most significant.  Looking at each character in the Long value and replacing each nybble with the corresponding port name yields: `HGFEDCBA`, so...
```bash
setpci -s 02:00.0 B4.L=01F01F01
```
...would configure H, E, and B for RS232; G, D, and A would be RS422, and the ports F and C would be RS485.


###Configuring a 4-port card's per-port protocol
Configuration is much the same as for an 8-port card but for one quirk: the fourth port's register nybble is located in the location of the 8th-port, above.  Thus, for four-port PCIe serial cards the 32-bit configuration register can be thought of like `DxxxxCBA`, so...
```bash
setpci -s 02:00.0 B4.L=f00001f0
```
...would configure port D and B for RS485, Port C for RS422, and Port A for RS232.

Note that the configuration nybbles for unused ports should be cleared to "0"

###Configuring a 1- or 2-port card's per-port protocol
Proceed exactly as shown for 4- or 8-port cards, but understand that the configuration nybbles for ports C through H should be cleared to "0".  That is, the 32-bit configuration register can be thought of as `xxxxxxBA` or `xxxxxxxA`.

http://datasheet.octopart.com/PI7C9X7958ANBE-Pericom-datasheet-11898032.pdf