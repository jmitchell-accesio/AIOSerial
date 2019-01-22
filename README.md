# AIOSerial
How To use ACCES Serial cards in Linux

There are two types of configuration instructions for the ACCES I/O Products' line of Serial cards.  The first gets the ttyS devices created and configured correctly.  The second type of configuration selects between RS232, RS422 and RS485 (and is only applicable to PCI Express cards, including PCI Express Mini (mPCIe) Cards.




# Instructions for configuring the serial port

On my system I run


```bash
dmesg | grep tty
> [    1.488105] 0000:02:00.0: ttyS4 at I/O 0xe000 (irq = 17, base_baud = 921600) is a ST16650
> [    1.490770] 0000:02:00.0: ttyS5 at I/O 0xe008 (irq = 17, base_baud = 921600) is a ST16650
> [    1.493416] 0000:02:00.0: ttyS6 at I/O 0xe010 (irq = 17, base_baud = 921600) is a ST16650
> [    1.496069] 0000:02:00.0: ttyS7 at I/O 0xe018 (irq = 17, base_baud = 921600) is a ST16650
```

Hence, my first ttyS4 corresponds to Port A, ttyS5 corresponds to Port B ...etc.


To get this card to work correctly you will need to do two things.

The first is to scale up the baud_base frequency on the card by a factor of 8. This compensates for the internal Pericom chip that uses 8 times the 115200 frequency that these types of UART chips typically use.

```bash
sudo  setserial /dev/ttyS4 baud_base 921600
sudo  setserial /dev/ttyS5 baud_base 921600
sudo  setserial /dev/ttyS6 baud_base 921600
sudo  setserial /dev/ttyS7 baud_base 921600
```

Second, you will need to configure your card based on the modes you want to use. A mode would be either RS232, RS485 or RS422.  By default on boot up, every port will be in RS232 mode, so you can at least quickly check that connections work for you.


For our example I will be creating a configuration that is the following:

PortA = RS-485
PortB = RS-485
PortC = RS-422
PortD = RS-422

You will need to set your PCIe-COM-4SM  using the setpci command and verify it with the lspci.

Before you run this you will need to find out which Domain and Bus your PCIe-COM-4SM is located at

```
lspci -n | grep "494f:"

> 02:00.0 0700: 494f:10d9  # <- this is a com board for vendor id 0x494f ( ACCES I/O )
```


Now you can just run

```
sudo setpci -v -s 02:00.0 b4.B=ff,11
```

To verify that you have set this , run the lspci as follows

```
sudo lspci -xxx -v  -s 02:00.0      
>
> 02:00.0 Serial controller: ACCES I/O Products, Inc. Device 10d9 (prog-if 02 [16550])
>         Flags: bus master, fast devsel, latency 0, IRQ 17
>         I/O ports at e000 [size=64]
>         Memory at f7d00000 (32-bit, non-prefetchable) [size=4K]
>         Capabilities: [80] Power Management version 3
>         Capabilities: [8c] MSI: Enable- Count=1/1 Maskable- 64bit+
>         Capabilities: [9c] Vital Product Data
>         Capabilities: [a4] Vendor Specific Information: Len=28 <?>
>         Capabilities: [e0] Express Legacy Endpoint, MSI 00
>         Capabilities: [100] Advanced Error Reporting
>         Kernel driver in use: serial
> 00: 4f 49 d9 10 07 00 10 00 00 02 00 07 10 00 00 00
> 10: 01 e0 00 00 00 00 d0 f7 00 00 00 00 00 00 00 00
> 20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
> 30: 00 00 00 00 80 00 00 00 00 00 00 00 0b 01 00 00
> 40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
> 50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
> 60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
> 70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
> 80: 01 8c c3 47 08 00 00 00 00 00 00 00 05 9c 80 00
> 90: 00 00 00 00 00 00 00 00 00 00 00 00 03 a4 00 00
> a0: 00 00 00 00 09 e0 28 00 60 10 00 04 71 02 00 04
> b0: 00 00 00 00 ff 11 00 00 04 00 01 00 00 00 00 00
> c0: 00 00 00 00 08 00 00 bf 00 02 00 00 00 00 00 00
> d0: 00 00 00 00 00 00 00 00 84 00 00 00 18 00 00 00
> e0: 10 00 11 00 00 80 90 05 00 00 10 00 11 3c 00 00
> f0: 00 00 11 00 00 00 00 00 00 00 00 00 00 00 00 00
```

The values ff and 11 represent the 485 and 422 configurations as based off of the Pericom data sheet ( see page 32 ) : http://datasheet.octopart.com/PI7C9X7958ANBE-Pericom-datasheet-11898032.pdf


# Example

For cards that match the following table of cards, reference the instructions just below at [Matching Devices](#MatchingDevices)

| 4 port           |  8 port             | 
| -------------    | ------------------- | 
| PCIe-COM-4SM     | PCIe-COM-8SM      | 
| PCIe-COM-4SMRJ   | PCIe-COM-8SMRJ    | 
| PCIe-COM-4SMDB   | PCIe-COM-8SMDB    | 
| PCIe-COM485-4    | PCIe-COM485-8     | 
| PCIe-COM422-4    | PCIe-COM422-8     | 
| PCIe-COM232-4    | PCIe-COM232-8     | 
| PCIe-COM232-4RJ  | PCIe-COM232-8RJ   | 
| PCIe-COM232-4DB  | PCIe-COM232-8DB   | 
| 104-COM232-4     | 104-COM232-8      | 
| 104-COM-4S       | 104-COM-8S        | 
| 104-COM-4SM      | 104-COM-8SM       | 
| 104I-COM232-4    | 104I-COM232-8     | 
| 104I-COM-4S      | 104I-COM-8S       | 
| 104I-COM-4SM     | 104I-COM-8SM      | 
| LPCI-COM232-4    | LPCI-COM232-8     | 
| LPCI-COM-4SM     | LPCI-COM-8SM      |


## <a name="MatchingDevices"></a>Matching Devices
Starting with a 4port device, PCIe-com-4SM, we want to have the following configuration

Port A = 422
Port B = 485
Port C = 485
Port D = 422


## First we will start with the ports A and D.  In our example we have
found , when grepping throug the dmesg, that our TTY's for Port A
and D are /dev/ttyS4 and /dev/ttyS7 respectively. Hence we want to put
a "1" in the PCI configuration register for these two ports.

Due to the nature of the ports being backwards ( according to sane
people ), you have to reverse the order as in the following snippet.

### Setting the Serial ports for Matching Serial Devices
```bash
RS422=1
RS485=f

A=$RS422 
B=$RS485
C=$RS485
D=$RS422

sudo setpci -v -s 02:00.0 b4.B=$B$A,$D$C
0000:02:00.0 @b4 f1 1f
```

This would configure PortA and PortD to be 422 and PortB and PortC to
be 485. 


### Setting the Serial ports for Non-matching Serial devices


```bash
RS422=1
RS485=f

A=$RS422 
B=$RS485
C=$RS485
D=$RS422

sudo setpci -v -s 02:00.0 b4.B=$B$A,0$C,00,$D0
0000:02:00.0 @b4 f1 1f
```


# Pinout diagrams

Please consult 

- [Pin out image for 485 mode](Pics/RS485_PortA_to_PortB.jpg )

- [Pin out image for 422 mode](Pics/RS422_FullDuplex.jpg)
