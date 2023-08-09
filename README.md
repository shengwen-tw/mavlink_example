## Build

```
make all
```

## Usage

Connect via serial:

```
./mavlink [-v] -s serial-name [-b baudrate]
```

Connect via TCP/IP:

```
./mavlink [-v] -n -i ip-address -p port-number
```

* `-v`: verbose, print received MAVLink messages.
* `-n`: connect via TCP/IP, otherwise via serial.
* `serial-name`: path to the serial device.
* `baudrate`: baudrate of the serial port.
* `ip-address`: IP address to a net server using MAVLink as communication protocol.
* `port-number`: Port number of the TCP/IP for MAVLink connection.
