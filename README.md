# AIOSerial
How To use ACCES Serial cards in Linux

There are two types of configuration instructions for the ACCES I/O Products' line of Serial cards.  The first gets the ttyS devices created and configured correctly.  The second type of configuration selects between RS232, RS422 and RS485 (and is only applicable to PCI Express (PCIe) cards, including PCI Express Mini (mPCIe) Cards.

## Instructions for creating the devices (/dev/ttySn)

### Kernels 4.15 and newer
These kernels should successfully detect and install all ACCES PCI and PCIe serial cards, and should configure the created dev/ttySn correctly.  Please note that 1- and 2-port cards are creating 4 ttyS devices; please ignore the higher-numbered devices if your card doesn't include them.

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

### Kernels 3.13 through 4.13
These kernels will successfully detect and install all ACCES PCI and PCIe serial cards, but the fourth port of four-port PCIe cards will be installed at an incorrect base address.

For example, For example, an mPCIe-COM-4SM installs as follows...
```bash
dmesg | grep tty
...
> [    1.488105] 0000:02:00.0: ttyS4 at I/O 0xe000 (irq = 17, base_baud = 921600) is a ST16650
> [    1.490770] 0000:02:00.0: ttyS5 at I/O 0xe008 (irq = 17, base_baud = 921600) is a ST16650
> [    1.493416] 0000:02:00.0: ttyS6 at I/O 0xe010 (irq = 17, base_baud = 921600) is a ST16650
> [    1.496069] 0000:02:00.0: ttyS7 at I/O 0xe018 (irq = 17, base_baud = 921600) is a ST16650
```
...but the I/O address of ttyS7 (0xe018) should be 0xe038, instead, so ttyS7 will not work "out of the box" in these kernels

To get this fourth ttyS to work correctly you will need to issue a setserial command ...
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

Please note these kernels will *also* install four-ports even on 1- and 2-port cards.

### Kernels before 3.13
These older kernels may detect and install most ACCES PCI and PCIe serial cards, but won't configure the baud_base correctly, won't install the fourth port of four-port PCIe cards correctly, and may not detect relatively recent product models.

Resolve the fourth port issue as above, then scale up the baud_base frequency on the card by a factor of 8. This compensates for the UART chip that uses 8 times the typical, 115200, base baudrate frequency.

```bash
setserial /dev/ttyS4 baud_base 921600
setserial /dev/ttyS5 baud_base 921600
setserial /dev/ttyS6 baud_base 921600
setserial /dev/ttyS7 baud_base 921600
```

##Configuring the per-port protocol for RS232, RS422, or RS485 (PCIe only)

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