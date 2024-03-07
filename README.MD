# Bird 3 Controller #

## Introduction ##
The Bird 3 scooter is a scooter designed by Bird specifically for the rental market. Unfortunately for them, they are sometimes seized and sold at auction or abandoned when ceasing operations in a market.

Note: This document was developed by observing a Bird 3 scooter using the `EA00004` controller and another scooter with the `EA00022` controller. The `EA00003` controller is also used for older Bird 3 scooters with 22-pin connectors, but the communication protocol remains compatible, at least as far as this document is concerned.

Compared to the `EA00004`, the `EA00022` controller has an updated circuit board that touts sidewalk detection. Pinout and messages remain the same.

## Fundamentals ##
The Bird 3 scooter uses a nominally 36 volt battery in the 10S6P configuration. It has three circuit boards:
 - Controller/Bird Brain - Contains the GPS, cellular radio, LCD
   - Reads the control inputs such as throttle, brakes, and button.
   - Located in the handlebars
 - BMS - Battery Management System
   - Handles charging and cell balancing
   - Soldered to the battery
 - ESC - Brushless DC Motor Controller aka Electronic Speed Control
   - Controls the motor and the rear light
   - Located behind the battery

These modules communicate using a CAN bus, similar to how the computers in most automobiles communicate. Each controller sends many messages, some hundreds of times per second. The messages can be roughly separated into two categories: status and command.

The CAN bus on the Bird 3 operates at 500kbps and at 2.5v.

## Messages to Receive ##
These messages provide important information about the scooter's components. You only need to receive one of them to operate the scooter.

| Message ID        | Interval | Data Length | Source | Description                            |
|-------------------|----------|-------------|--------|----------------------------------------|
| `0x60C`           | 1000 ms  | 4 bytes     | BMS    | BMS ID of some sort used for keepalive |
| `0x152`           | 20 ms    | 7 bytes     | ESC    | Motor Speed                            |
| `0x701` - `0x70A` | 1000 ms  | 3 bytes     | BMS    | Cell Voltage in millivolts             |

### `0x60C` - BMS ID ###
This message contains four bytes that must be sent in the `0x742` message to keep the BMS awake. Otherwise, it will go to sleep after a minute or two.

More details are in the documentation for `0x742`.

### `0x152` - Motor Speed ###
Bytes 3 and 4 of this message contain the motor's current speed as a signed 16-bit integer representing the current RPM of the rear wheel.

At full speed, the highest number I've seen is 516. With a nominally 10" wheel, this works out to 24.7KM/h or 15.35mph.

### `0x701` - `0x70A` - Cell Voltage ###
The 10s6p battery configuration means there are 10 cells. The BMS monitors the voltage of each of these and reports it with these messages.

Bytes 1 and 2 of each of these messages contain the cell voltage in millivolts as a 16-bit integer.


## Operational Messages ##
Unlocking the BMS, operating the motor, and enabling the rear light takes only four messages.

| Message ID | Interval | Data Length | Description                  |
|------------|----------|-------------|------------------------------|
| `0x140`    |  200 ms  | 8 bytes     | BMS Output Enable            |
| `0x153`    |    2 ms  | 4 bytes     | Motor Control                |
| `0x500`    |   50 ms  | 3 bytes     | Rear Light Enable            |
| `0x742`    | 5000 ms  | 8 bytes     | Keepalive                    |

### `0x140` - BMS Output Enable ###
This message is eight bytes long and is sent every 200 ms. The second byte determines whether the BMS should output voltage to the ESC. The other bytes have unknown purposes, and are set to 0x00 in all observed messages.
Here are the two known messages:

| Description    | Message Contents                                 |
|----------------|--------------------------------------------------|
| Disable Output | `0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00` |
| Enable Output  | `0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00` |


### `0x153` - Motor Control ###
This message is four bytes long and is sent every 2 ms. This must be sent continuously in order for the ESC to continue powering the motor.

The first two bytes determine the motor output. Rather than setting the speed, this appears to set the torque or current. Large values cause the motor to reach its max speed very quickly, while smaller values still cause it to reach its max speed, albeit more slowly.

The valid values are from 0x0000 to 0x1027, according to [this discord post](https://discord.com/channels/550154171433615364/747890701328711771/1039920555308101724). This is also a [little-endian value](https://www.geeksforgeeks.org/little-and-big-endian-mystery), meaning that the least significant byte comes first.

The last byte controls the dynamic braking feature that you may have experienced when moving an irresponsibly parked scooter.

Examples:
| Description           | Message Contents         |
|-----------------------|--------------------------|
| 0% power              | `0x00, 0x00, 0x00, 0x00` |
| 0% power with braking | `0x00, 0x00, 0x00, 0x01` |
| 33% power             | `0x1d, 0x0e, 0x00, 0x00` |
| 100% power            | `0x27, 0x10, 0x00, 0x00` |

In my experience, even low values such as `0x0036` have been ample to get the motor to its max speed on the stand, although the torque is significantly less than with higher values.

### `0x500` - Rear Light Control ###
This message is three bytes long and has two known values. It is unknown if the light can be dimmed.

| Description   | Message Contents   |
|---------------|--------------------|
| Disable Light | `0x00, 0x00, 0x00` |
| Enable Light  | `0x01, 0x00, 0x00` |

### `0x742` - Keepalive ###
This message is eight bytes long, and is made up of two parts. The first four bytes are a 32-bit counter that increment once per second. This count restarts whenever the controller is plugged into the 24-pin connector. The last four bytes contain the bytes received from the BMS via the `0x60C` message from the BMS. This set of messages must be sent continually in order to keep the BMS and ESC awake.

The OEM Bird Brain sends `0x742` once per second: most messages contain seemingly random in positions 2, 4, 5, 6, and 7, but every fifth message contains the contents of the `0x60C` message in positions 4, 5, 6, and 7 (the last four bytes). The four messages containing random data seem to be unnecessary.

As an example, suppose a BMS sends the following message:
```
0x60C: 0x2C, 0x00, 0x41, 0x00
```
We need to respond with something like the following, sent every five seconds:

```
0x742: 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x0A, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x0F, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x14, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x19, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x1E, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
0x742: 0x23, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0x00
```
