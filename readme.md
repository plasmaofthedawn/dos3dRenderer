a 3d renderer I wrote for dos

works on hardware (the one laptop i have availble to test)

---
## running

requires [cwsdpmi](https://sandmann.dotster.com/cwsdpmi/) to run

movement is controled by wasd and qe, and rotation is controled by ijkl and ou
esc to quit

*might* require dos to be loaded high
i'm not too sure tbh but when i tested it on hardware that was needed

---
## build instructions

requires [djgpp](https://www.delorie.com/djgpp/) to build. i used the AUR package which works fine

then `make`

the flags in make specify a 486 architecture since the compiler was writing instructions too new for the hardware i was testing on

---
## credits

object models were taken from here [website](https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html)

---

## license

mit

