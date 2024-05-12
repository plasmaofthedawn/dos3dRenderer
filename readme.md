a 3d renderer I wrote for dos

works on hardware (the one laptop i have availble to test)

---
## running

requires [cwsdpmi](https://sandmann.dotster.com/cwsdpmi/) to run

movement is controled by wasd and qe, and rotation is controled by ijkl and ou
esc to quit

*might* require dos to be loaded high
i'm not too sure tbh but when i tested it on hardware that was needed

running a program is `view [file].pmf`

---
## wtf is a pmf file
it's a model file i made because i didn't feel like processing plaintext files in C

you can convert a .obj file to one using the provided python convert.py file
just
`convert.py [file].obj` and it'll spit out a beautiful pmf file

---
## build instructions

requires [djgpp](https://www.delorie.com/djgpp/) to build. i used the AUR package which works fine

then `make`

the flags in make specify a 486 architecture since the compiler was writing instructions too new for the hardware i was testing on

---
## credits

object models were taken from [here](https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html) and then coverted

---

## license

mit

