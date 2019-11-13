/*
VIRTUAL PET (PROTOTYPE)

This is the source code for a small Tamagotchi-like game prototype I wanted to do in a few hours.

One of two random pets will hatch. It will age and go hungry over time, and can be fed to stop it from starving. 
It will die from either starvation or old age, and let you start again once it has passed away.

The other options (play & heal) are only there as placeholders for the prototype to be expanded upon. 
There are a few other elements that are unused but that could be used : some data is there to prepare 
some form of "randomized" movement, and the sprites for the walking animations are already done.

The code is based on the GBDK Library. You will need it to compile the game. Find it here : 
http://gbdk.sourceforge.net/

GBTD ans GBMB tools by Harry Mulder : 
http://www.devrs.com/gb/hmgd/intro.html

I'm really bad at anything audio-related. What little audio I have here is based on what I understood from : 
https://gbdev.gg8.se/wiki/articles/Sound_tutorial

All main coding and graphical aspects are done by me, Piers Bishop.
https://github.com/PiersBishop/VirtualPet_Prototype
https://piersbishop.itch.io/
*/

// ---------------------------------------
//
//     INCLUDES
//
// ---------------------------------------
// ---
// Game Boy Library
// ---
#include <gb/gb.h>

// ---
// Sprite & BG tiles
// ---
#include "Sprites/TamaSprites.c"
#include "Sprites/CursorSprites.c"
#include "Background/BackTiles.c"

// ---
// BG info & text boxes
// ---
#include "Background/BackMap.c"
#include "Background/BackTopIcons.c"
#include "Background/BackTextNone.c"
#include "Background/BackTextHatch.c"
#include "Background/BackTextFeed.c"
#include "Background/BackTextPlay.c"
#include "Background/BackTextHeal.c"
#include "Background/BackTextTryAgain.c"

// ---------------------------------------
//
//     DEFINES
//
// ---------------------------------------
// ---
// sprite indexes
// ---
#define EggIndex 0
#define HeadIndex 4
#define LegsIndex 8
#define FaceIndex 11
#define HeartIndex 15
#define GraveIndex 16
#define CursorIndex 250

// ---
// animation values
// ---
#define wait 0
#define ready 1
#define walk 1
#define smile 0
#define happy 1
#define sad 2
#define sick 3
#define smoothHead 0
#define earsHead 2

// ---
// sound indexes
// ---
#define happySound 0
#define hatchSound 1
#define deathSound 2
#define beepSound 3

// ---
// cursor values
// ---
#define CursorHeight 24
#define CursorDelay 200
#define feed 32
#define play 80
#define heal 128
#define directionLeft -1
#define directionRight 1

// ---
// Pet info values
// ---
#define EggLoops 5
#define feedingValue -50
#define oldAge 500
#define deathAge 550

// ---------------------------------------
//
//     ADDITIONNAL DATA
//
// ---------------------------------------
const UINT8 rHeads[] = { // used to randomise pet's head
	smoothHead, earsHead, smoothHead, smoothHead, 
	earsHead, earsHead, smoothHead, earsHead
};

const UINT8 rLengths1[] = { // values intended for "random" walking around, unused
	5,2,7,3,
	8,1,4,6
};

const UINT8 rLengths2[] = { // values intended for "random" waiting, unused
	3,5,0,4,
	1,0,2,3
};

// ---------------------------------------
//
//     CALCULATIONS
//
// ---------------------------------------
UINT8 incLoop (UINT8 val, UINT8 max) { // add 1 and loop around
	val++;
	if (val == max) val = 0;
	return (val);
}

UINT8 incClamp (UINT8 val, UINT8 max) { // add 1 and clamp at maximum
	if (val != max) val++;
	return (val);
}

// ---------------------------------------
//
//     DISPLAY / ANIMATION FUNCTIONS
//
// ---------------------------------------
// ---
// Display Egg & Animate
// ---
void DisplayEgg(UINT8 frame, UINT8 isReady) {
	set_sprite_tile(0,EggIndex+1 + (frame*2));
	set_sprite_tile(1,EggIndex + (frame*2));
	move_sprite(0,80,80);
	move_sprite(1,80,88);
	
	set_sprite_tile(2,EggIndex+1 + (frame*2));
	set_sprite_tile(3,EggIndex + (frame*2));
	set_sprite_prop(2,0x20); // flip sprites for right side
	set_sprite_prop(3,0x20);
	move_sprite(2,88,80);
	move_sprite(3,88,88);
	
	set_sprite_tile(4,HeartIndex);
	move_sprite(4,84,75 - frame - (1-isReady)*70);
}

// ---
// Display Pet & Animate
// ---
void DisplayPet(UINT8 frame, UINT8 headType, UINT8 state, UINT8 mood) {
	set_sprite_tile(4,HeadIndex+headType + frame);
	set_sprite_tile(5,LegsIndex+state+(state*frame));
	set_sprite_prop(4,0x00); // specify that isn't flipped
	set_sprite_prop(5,0x00);
	move_sprite(4,80,80);
	move_sprite(5,80,88);
	
	set_sprite_tile(2,HeadIndex+headType + frame);
	set_sprite_tile(3,LegsIndex+state+(state*frame));
	set_sprite_prop(2,0x20); // flip sprites for right side
	set_sprite_prop(3,0x20);
	move_sprite(2,88,80);
	move_sprite(3,88,88);
	
	set_sprite_tile(0,FaceIndex+mood); // face has to be first sprites
	set_sprite_tile(1,FaceIndex+mood);
	set_sprite_prop(0,0x00);
	set_sprite_prop(1,0x20);
	move_sprite(0,80,83 + frame);
	move_sprite(1,88,83 + frame);
	
}

// ---
// Display Gravestone
// ---
void DisplayGrave() {
	set_sprite_tile(0,GraveIndex);
	set_sprite_tile(1,GraveIndex+1);
	move_sprite(0,80,80);
	move_sprite(1,80,88);
	set_sprite_prop(0,0x00);
	set_sprite_prop(1,0x00);
	
	set_sprite_tile(2,GraveIndex+2);
	set_sprite_tile(3,GraveIndex+3);
	move_sprite(2,88,80);
	move_sprite(3,88,88);
	set_sprite_prop(2,0x00);
	set_sprite_prop(3,0x00);
	
	move_sprite(4,0,0); // remove extra pet sprites
	move_sprite(5,0,0);
	
	move_sprite(36,0,0); // remove cursor sprites
	move_sprite(37,0,0);
	move_sprite(38,0,0);
	move_sprite(39,0,0);
}

// ---
// Display Cursor & Animate
// ---
void DisplayCursor(UINT8 frame, UINT8 selection){
	// setting sprites 36-39
	// in case I need more sprites 
	// for the pets later
	set_sprite_tile(36,CursorIndex + frame); // UL
	set_sprite_tile(37,CursorIndex + frame); // DL
	set_sprite_tile(38,CursorIndex + frame); // UR
	set_sprite_tile(39,CursorIndex + frame); // DR
	set_sprite_prop(37,0x40);
	set_sprite_prop(38,0x20);
	set_sprite_prop(39,0x60);
	move_sprite(36,selection,CursorHeight);
	move_sprite(37,selection,CursorHeight+8);
	move_sprite(38,selection+8,CursorHeight);
	move_sprite(39,selection+8,CursorHeight+8);
}

// ---
// Display Cursor Action in background
// ---
void DisplayCursorAction (UINT8 selection){
	switch (selection) {
		case feed :
		set_bkg_tiles(0,15,20,2,BackTextFeed);
		break;
		case play :
		set_bkg_tiles(0,15,20,2,BackTextPlay);
		break;
		case heal :
		set_bkg_tiles(0,15,20,2,BackTextHeal);
		break;
	}
}

// ---------------------------------------
//
//     EXTRA FUNCTIONS
//
// ---------------------------------------
UINT8 CursorPress (UINT8 currentState, INT8 direction) { // changes the cursor's state value
	currentState = 
		direction == directionLeft ?
			(currentState == heal ? play : feed)
		: 
			(currentState == feed ? play : heal);
	return (currentState);
}

UINT8 HungerAdd (UINT8 hgr, INT8 val, UINT8 max){ // used to increase or decrease Pet's hunger stat
	INT16 ret = hgr+val;
	if (ret < 0) return (0);
	if (ret > max) return (max);
	return (ret);
}

// ---------------------------------------
//
//     AUDIO
//
// ---------------------------------------
void PlaySound(UINT8 soundID, UINT8 creatureType) { 
// I'm really bad at anything audio-related, I based this on what I understood from : https://gbdev.gg8.se/wiki/articles/Sound_tutorial
	switch (soundID) {
		case hatchSound:
		NR10_REG = 0x1E; // edit last charcater, 6 if increase in freq, E if decrease in freq
		NR11_REG = 0x10; // don't change ?
		NR12_REG = 0xF2; // edit last charcater, values 1-7 = "echo length", 0 is a default value not <1
		NR13_REG = 0x00; // don't use ?
		NR14_REG = 0x84; // edit last charcater, values 0-7 = frequency
		break;
		case happySound:
		NR10_REG = 0x16; 
		NR11_REG = 0x10; 
		NR12_REG = creatureType==smoothHead? 0xF7 : 0xF4; // different cries for different creatures
		NR13_REG = 0x00;
		NR14_REG = creatureType==smoothHead? 0x86 : 0x85; // different cries for different creatures
		break;
		case deathSound:
		NR10_REG = 0x1E;
		NR11_REG = 0x10;
		NR12_REG = 0xF6;
		NR13_REG = 0x00;
		NR14_REG = 0x83;
		break;
		case beepSound:
		NR10_REG = 0x16;
		NR11_REG = 0x10;
		NR12_REG = 0xF1;
		NR13_REG = 0x00;
		NR14_REG = 0x83;
		break;
	}
}

// ---------------------------------------
//
//     MAIN FUNCTION
//
// ---------------------------------------
void main() {
	// ---
	// variables have to be at beginning of script
	// ---
	UINT8 frm = 0; // frame counter (0-1)
	UINT8 idx = 0; // index value used for loops
	UINT8 idx2 = 0; // index value used for loops, unused
	UINT8 rdm = 0; // counter value used for fake randomness
	INT8 posX = 80; // coordinates value, unused
	INT8 posY = 88; // coordinates value, unused
	UINT8 myHead = 0; // to store head type (random)
	UINT8 PET_hunger = 0; // Pet info : hunger
	UINT16 PET_age = 0; // Pet info : age
	UINT8 PET_state = smile; // Pet info : state (face)
	UINT8 cursorState = feed; // cursor value and default state
	UINT8 cursorTimer = CursorDelay; // input cooldown timer for the cursor
	
	// ---
	// sprites data setup
	// ---
	SPRITES_8x8;
	set_sprite_data(0,20,TamaSprites);
	set_sprite_data(CursorIndex,2,CursorSprites);
	SHOW_SPRITES;
	
	// ---
	// background data setup + display
	// ---
	set_bkg_data(0,35,BackTiles);
	set_bkg_tiles(0,0,20,18,BackMap);
	SHOW_BKG;
	
	// ---
	// turn audio on (it is off by default to save the Game Boy's batteries)
	// ---
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;
	
	// ---
	// display button prompt to start game
	// ---
	set_bkg_tiles(0,15,20,2,BackTextPlay); // display button text
	waitpad(J_A); // wait for input
	PlaySound(beepSound,0); // play audio feedback
	set_bkg_tiles(0,15,20,2,BackTextNone); // hide button text
	
	// ---
	// main game loop
	// ---
	while(1) {
		// ------------------------------------------
		// 
		//     FIRST EGG WAITING CYCLE
		// 
		// ------------------------------------------
		frm = 0; // reset frame counter
		for(idx=0; idx < EggLoops*2; idx++){ // loop to animate egg, wait X animation loops before erady to hatch
			DisplayEgg(frm,wait);
			frm = 1-frm; // alternate between frm=0 and frm=1
			delay(500);
		}
		
		// ------------------------------------------
		// 
		//     EGG IS READY, WAITING FOR INPUT
		// 
		// ------------------------------------------
		frm = 0; 
		set_bkg_tiles(0,15,20,2,BackTextHatch); // display button prompt
		while (1) { // animation loop
			DisplayEgg(frm,ready);
			for(idx=0; idx < 250; idx++){
				delay(2);
				if(joypad()==J_A){ // waiting for button press
					goto hatchEgg;
				}
			}
			rdm = incLoop(rdm,8); // "random" value loops between 0-7 very fast until button is pressed
			frm = 1-frm;
		}
		hatchEgg:
		
		// ------------------------------------------
		// 
		//     EGG HATCHING ANIM
		// 
		// ------------------------------------------
		DisplayEgg(0);
		set_bkg_tiles(0,15,20,2,BackTextNone);
		move_sprite(4,0,0); // remove heart
		// split egg progressively :
		move_sprite(0,79,79);
		PlaySound(hatchSound,0);
		delay (500);
		move_sprite(1,79,89);
		PlaySound(hatchSound,0);
		delay (500);
		move_sprite(2,89,79);
		PlaySound(hatchSound,0);
		delay (500);
		move_sprite(3,89,89);
		PlaySound(hatchSound,0);
		delay (500);
		// loop this animation a bit
		for (idx=0; idx<3; idx++){
			move_sprite(0,80,80);
			move_sprite(1,80,88);
			move_sprite(2,88,80);
			move_sprite(3,88,88);
			delay (200);
			move_sprite(0,79,79);
			move_sprite(1,79,89);
			move_sprite(2,89,79);
			move_sprite(3,89,89);
			PlaySound(hatchSound,0);
			delay (200);
		}
		
		// ------------------------------------------
		// 
		//     PREPARING CREATURE & UI
		// 
		// ------------------------------------------
		frm = 0; // reset frame counter
		set_bkg_tiles(0,1,20,2,BackTopIcons); // display icons up top
		set_bkg_tiles(0,15,20,2,BackTextFeed); // display bottom text
		myHead = rHeads[rdm]; // figuring out which head was "randomly" selected
		PET_hunger = 20; // initial hunger value
		cursorTimer=CursorDelay; // reset cursor delay
		DisplayPet(0,myHead, wait, smile); // display Pet creature
		PlaySound(happySound, myHead); // it's happy to see you !
		
		// ------------------------------------------
		// 
		//     PET GAMEPLAY LOOP
		// 
		// ------------------------------------------
		while(1){
			// ---
			// (loop) input detection + frame delay
			// ---
			// this loop checks for input every frame and also calls "delay()"
			// This is done 250 times before displaying, to give a normal
			// 500-length delay between animation frames
			// ---
			for(idx=0; idx < 250; idx++){ 
				cursorTimer = incClamp(cursorTimer, CursorDelay);
				delay(2);
				// ---
				// if I press A (select an action)
				// ---
				if (joypad()==J_A && cursorTimer==CursorDelay) {
					if(cursorState == feed) {
						PET_hunger = HungerAdd(PET_hunger, feedingValue);
						PET_state = happy;
						PlaySound(happySound, myHead);
						DisplayPet(frm,myHead, wait, PET_state);
					} else if(cursorState == play) {
						// nothing here yet
						// random content to avoir warnings
						PET_state = PET_state;
						PlaySound(beepSound,0);					
					} else if(cursorState == heal) {
						// nothing here yet
						// random content to avoir warnings
						PET_state = PET_state;
						PlaySound(beepSound,0);	
					}
					cursorTimer = 0;
				// ---
				// if I press left or right (move the cursor)
				// ---
				} else if (joypad()==J_LEFT && cursorTimer==CursorDelay) {
					cursorState = CursorPress(cursorState,directionLeft);
					DisplayCursor(frm,cursorState);
					DisplayCursorAction(cursorState);
					cursorTimer = 0;
					PlaySound(beepSound,0);
				} else if (joypad()==J_RIGHT && cursorTimer==CursorDelay) {
					cursorState = CursorPress(cursorState,directionRight);
					DisplayCursor(frm,cursorState);
					DisplayCursorAction(cursorState);
					cursorTimer = 0;
					PlaySound(beepSound,0);
				} else if (joypad() == 0x00U){ // cancels timer if no buttons are pressed
					cursorTimer=CursorDelay;
				}
			}
			// ---
			// Updating values & checking for Death
			// ---
			frm = 1-frm;
			PET_hunger=incClamp(PET_hunger,100);
			PET_age++;
			PET_state = PET_hunger > 80 ? sad : smile; // changes state depending on hunger and age values
			PET_state = PET_age > oldAge ? sad : PET_state;
			// dies if too hungry or old
			if (PET_hunger == 100) {
				goto Death;
			}
			if (PET_age == deathAge) {
				goto Death;
			}
			// ---
			// Display & animation
			// ---
			DisplayPet(frm,myHead, wait, PET_state);
			DisplayCursor(frm,cursorState);
		}
		Death:
		
		// ------------------------------------------
		// 
		//     DYING ANIM
		// 
		// ------------------------------------------
		DisplayPet(0,myHead, wait, sick); // sick face + frame 0 anim state
		set_bkg_tiles(0,15,20,2,BackTextNone); //remove texts
		set_bkg_tiles(0,1,20,2,BackTextNone);
		// remove cursor :
		move_sprite(36,0,0); 
		move_sprite(37,0,0); 
		move_sprite(38,0,0); 
		move_sprite(39,0,0);
		// same anim as egg hatching + face
		for (idx=0; idx<3; idx++){
			move_sprite(4,80,80);
			move_sprite(5,80,88);
			move_sprite(2,88,80);
			move_sprite(3,88,88);
			move_sprite(0,80,83);
			move_sprite(1,88,83);
			delay (200);
			move_sprite(4,79,79);
			move_sprite(5,79,89);
			move_sprite(2,89,79);
			move_sprite(3,89,89);
			move_sprite(0,79,83);
			move_sprite(1,89,83);
			PlaySound(deathSound,0);
			delay (200);
		}
		
		// ------------------------------------------
		// 
		//     DEATH AND RESET
		// 
		// ------------------------------------------
		set_bkg_tiles(0,15,20,2,BackTextTryAgain); // display text
		DisplayGrave(); // display gravestone
		PlaySound(deathSound,0); // play dead sound
		while (1) { // wait for final input
			if (joypad()==J_A) {
				PlaySound(beepSound,0);
				delay(10);
				break;
			}
			delay(10);
		}
		// Getting here will loop back to beginning and spawna new egg. 
		// Clearing top and bottom windows again just in case.
		set_bkg_tiles(0,15,20,2,BackTextNone);
		set_bkg_tiles(0,1,20,2,BackTextNone);
	}
	
	// ------------------------------------------
	// This area is the end of the main function.
	// There should be nothing here, everything 
	// goes in the main while loop, or before it.
	// Nothing should ever bring you out of the 
	// main game while loop.
	// ------------------------------------------
}