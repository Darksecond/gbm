#include "IO.h"

IO::IO() : win(nullptr), ren(nullptr) {
}

IO::~IO() {
	destroy();
}

void IO::create() {
	win = SDL_CreateWindow("GBM", 0, 0, 160, 144, SDL_WINDOW_SHOWN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void IO::destroy() {
	if(ren) {
		SDL_DestroyRenderer(ren);
		ren = nullptr;
	}
	if(win) {
		SDL_DestroyWindow(win);
		win = nullptr;
	}
}

void IO::flip() {
	if(!ren) return;
	SDL_RenderPresent(ren);
}

void IO::set_px(int x, int y, const RGB &c) {
	if(!ren) return;
	SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
	SDL_RenderDrawPoint(ren, x, y);
}

void IO::clear() {
	if(!ren) return;
	SDL_RenderClear(ren);
}
