# VirtualPet_Prototype

**Game Concept**

This is the source code for a small Tamagotchi-like game prototype I wanted to do in a few hours. 

One of two random pets will hatch. It will age and go hungry over time, and can be fed to stop it from starving. It will die from either starvation or old age, and let you start again once it has passed away.

The other options (play & heal) are only there as placeholders for the prototype to be expanded upon. There are a few other elements that are unused but that could be used : some data is there to prepare some form of "randomized" movement, and the sprites for the walking animations are already done.

**Files Contents**

The Sprites and Backgrounds folders contain the C data for the graphics, and their .GBR / .GBM files (for the Tile Designer and Map Builder tools). All of the game code is in VirtualPet.c, compil.bat launches the compilation, and VirtualPet.gb is the Game Boy rom it produces.

**Credits**

The code is based on the GBDK Library. You will need it to compile the game. Find it here :
http://gbdk.sourceforge.net/

GBTD ans GBMB tools by Harry Mulder :
http://www.devrs.com/gb/hmgd/intro.html

I'm really bad at anything audio-related. What little audio I have here is based on what I understood from : 
https://gbdev.gg8.se/wiki/articles/Sound_tutorial

All main coding and graphical aspects are done by me, Piers Bishop. 
