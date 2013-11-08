#include "input.h"
#include <SDL.h>

void GB::Input::step() {
	P1 = 0x0F;
}

uint8_t GB::Input::read8(uint16_t addr) {
	if(addr != 0xFF00) return 0;

	const uint8_t *keys = SDL_GetKeyboardState(NULL);
	P1 |= 0x0F;

	if((P1 & 0x10) == 0) { //P14
		if(keys[SDL_SCANCODE_RIGHT])
			P1 &= ~0x01;
		if(keys[SDL_SCANCODE_LEFT])
			P1 &= ~0x02;
		if(keys[SDL_SCANCODE_UP])
			P1 &= ~0x04;
		if(keys[SDL_SCANCODE_DOWN])
			P1 &= ~0x08;
	}
	if((P1 & 0x20) == 0) { //P15
		if(keys[SDL_SCANCODE_Z])
			P1 &= ~0x01;
		if(keys[SDL_SCANCODE_X])
			P1 &= ~0x02;
		if(keys[SDL_SCANCODE_RETURN])
			P1 &= ~0x04;
		if(keys[SDL_SCANCODE_SPACE])
			P1 &= ~0x08;
	}

	return P1;
}

void GB::Input::write8(uint16_t addr, uint8_t value) {
	if(addr == 0xFF00)
		P1 = value;
}
