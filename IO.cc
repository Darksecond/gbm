#include "IO.h"

IO::IO() : win(nullptr), ren(nullptr), tex(nullptr) {
}

IO::~IO() {
	destroy();
}

void IO::create() {
	win = SDL_CreateWindow("GBM", 0, 0, 160*4, 144*4, SDL_WINDOW_SHOWN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160*4, 144*4);
}

void IO::destroy() {
	if(tex) {
		SDL_DestroyTexture(tex);
		tex = nullptr;
	}
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
	SDL_SetRenderDrawColor(ren, White.r, White.g, White.b, 255);
	SDL_RenderClear(ren);
	SDL_UpdateTexture(tex, NULL, framebuffer, 160*4*sizeof(RGBA));
	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);
}

void IO::set_px(int x, int y, const RGB &c) {
	if(!ren) return;
	framebuffer[y*160*4+x].r = c.r;
	framebuffer[y*160*4+x].g = c.g;
	framebuffer[y*160*4+x].b = c.b;
	framebuffer[y*160*4+x].a = 255;
}

void IO::clear(const RGB& c) {
	if(!ren) return;
	memset(framebuffer, 0, 160*4*144*4*sizeof(RGBA));
}
