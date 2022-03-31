# A flux dispenser for woz files

## Theory

 * The device plugs in on USB and you can upload at least one .woz file to its internal storage
 * A track of flux is stored in memory, and 'scanned out' using the PIO peripheral and DMA
 * When incoming step signals are seen (also from a PIO peripheral as pin-change-interrupt), a different track is read and fluxed out
 * Later, a UI will allow selection of the woz file
 * Much later, or never, using an SD card might be added
 * Much, Much later, or never, writing might be added

## FAQ

### Q: What microcontroller(s) are targeted?

RP2040

### Q: Does this work yet?

A: No

### Q: Is there hardware yet?

A: No

### Q: Why is flux data stored one byte per bit?

A: PIO peripheral can't act on units smaller than 1 byte, and there's enough RAM to be extravagant


