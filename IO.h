#pragma once

#include <SDL.h>

struct RGB {
	uint8_t r, g, b;
};

constexpr RGB Black = {0,0,0};
constexpr RGB White = {255,255,255};

struct IO {
	SDL_Window *win;
	SDL_Renderer *ren;
public:
	IO();
	~IO();

	void create();
	void destroy();
	void flip();
	void set_px(int x, int y, const RGB &color);
	void clear(const RGB &color = Black);
};
