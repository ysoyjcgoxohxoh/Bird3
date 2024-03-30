> [!NOTE]
> This design is untested

## Fixes from Rev 1 ##
- Corrected slight inaccuracies in mounting hole locations
- Wired the LCD Connector in the correct direction
- Adjusted LCD mounting to add alignment holes and correct mounting cutout width.
- Moved components interfering with LCD mounting
- Replaced headlight circuit with one that steps down directly from 36V.
- Moved components interfering with LCD
- Increased headlight current setting

## Improvements from Rev 1 ##
- Added 22-pin connector (Credits to `Mental_Metalhead` for the pinout)
- Added EEPROM for durable storage ($0.19 BOM, > 4 million writes)
- Added Mosfet to programatically ground the `PWR_ENA` pin rather than tying it permanently to ground
- Added copper pour to reduce EMI

## Errata ##
- None yet

## Board Renderings ##
![PCB](Bird3Controller.png)
