> [!NOTE]
> This board does drive the scooter, but it has some issues. This revision is not recommended.

## Errata ##
- Physical
  - The mounting holes are in slightly incorrect locations.
- LCD
  - LCD connector is wired in reverse.
  - The LCD mounting cutouts are not quite big enough, causing a very tight fit when clipping on the LCD.
  - The LCD mounting is missing alignment holes.
  - The data lines to the LCD span two registers, necessitating jumper wires for compatibility with the `TFT_eSPI` library.
- Power
  - There are two 3.3v nets, causing half the board to be unpowered.
  - 3.3v is insufficient for the headlight driver.
- Headlight
  - The headlight driver's current is set too low.
  - The headlight driver's inductor physically interferes with LCD.


## Board Renderings ##
![Top of PCB](Bird3Controller-Top.png)
![Bottom of PCB](Bird3Controller-Bottom.png)
