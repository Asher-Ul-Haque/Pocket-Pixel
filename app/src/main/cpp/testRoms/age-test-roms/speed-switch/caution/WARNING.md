# WARNING

The roms in this folder prematurely terminate the HALT mode period that follows
STOP when switching the CPU speed.
Purpose of that HALT mode period is to allow for
[oscillation stabilization](https://archive.org/details/GameBoyProgManVer1.1/page/n256)
before returning control to the CPU.

**Nintendo's Game Boy Programming Manual
[explicitly warns about this](https://archive.org/details/GameBoyProgManVer1.1/page/n34).**

I have experienced my Game Boy Color E (CPU CGB E - CPU-CGB-06) to become
instable for a while when running these roms several times in a row.
Even a reset could not stabilize it,
but after some time everything seemed to be fine again.

My Game Boy Color B (CPU CGB B - CPU-CGB-02) on the other hand did not show any
stability issues.
