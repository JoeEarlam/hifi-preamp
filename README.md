# hifi preamp
A small, digitally controlled hifi preamp, with remote input and volume selection. Uses a PGA2311 for volume control and an NJM2753 for input switching.

Bipolar power is required, this can be from an AC transformer (preferred) or a switch mode supply. The prototype uses a 3VA transformer with two 15V windings. Higher voltage transformers are not recommended as the 15V model already produces >21V DC post regulator and the hottest vReg sits at around 45DegC.

Audio IO is as specified in the PGA2311 datasheet. TL;DR, this is 31.5dB maximum gain, with clipping happening around 4.5V peak. I have performed zero measurements, but everything sounds great which is good enough for me.

IR uses NEC codes produced by a Chromecast remote. These can be customised depending on your remote. Control can also be performed using the buttons on the PCB.
