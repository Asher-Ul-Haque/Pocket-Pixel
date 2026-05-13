# AGE test roms

These Game Boy test roms were written to complement
[other test suites](https://github.com/c-sp/gameboy-test-roms).
I use them to test
[my Game Boy emulator AGE](https://github.com/c-sp/AGE)
for regressions.

## Test naming

All tests are named to include the devices they have been verified
to be compatible to.

| Test name | compatible devices |
|-----------|--------------------|
| `foo-cgbBE.gb` | `CPU-CGB-B` & `CPU-CGB-E`
| `foo-dmgC-cgbB.gb` | `CPU-DMG-C` & `CPU-CGB-B`
| `foo-ncmBE.gb` | `CPU-CGB-B (non-CGB mode)` & `CPU-CGB-E (non-CGB mode)`

## Screenshot colors

To be able to verify the result of screenshot based tests
your emulator should calculate RGB values as follows:
- the four DMG shades are `#000000`, `#555555`, `#AAAAAA` and `#FFFFFF`
- each CGB 5 bit RGB channel is converted to 8 bits
  using the formula `(X << 3) | (X >> 2)`
- the shades for non-CGB mode are:
  - background: `#000000`, `#0063C6`, `#7BFF31` and `#FFFFFF`
  - objects: `#000000`, `#943939`, `#FF8484` and `#FFFFFF`

## Font

AGE test roms use the
[Cellphone Font](https://opengameart.org/content/ascii-bitmap-font-cellphone)
created by
[domsson](https://opengameart.org/users/domsson).

## Build

You need [RGBDS](https://rgbds.gbdev.io) to `make` these roms.

## Development tools

In my experience [Visual Studio Code](https://code.visualstudio.com)
with the [RGBDS GBZ80 plugin](https://github.com/DonaldHays/rgbds-vscode)
installed is your best choice.
