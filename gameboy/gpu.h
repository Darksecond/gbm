#pragma once

#include "../IO.h"
#include "../util.h"
#include <cstdint>

namespace GB {

	struct MMU;
	struct GPU {
		uint8_t vram[8192]; //Video ram
		uint8_t oam[160];   //Object Attribute Memory
		RGB framebuffer[160*144];

		int current_line;
		int clock;
		int mode;
		IO &io;
		MMU &mmu;
		int x_scrl, y_scrl;
		int lyc;

		union {
			RegBit<7> LCD_ON;
			RegBit<6> WND_MAP_BASE;
			RegBit<5> WND_ON;
			RegBit<4> BG_TILE_BASE;
			RegBit<3> BG_MAP_BASE;
			RegBit<2> OBJ_SIZE;
			RegBit<1> OBJ_ON;
			RegBit<0> BG_ON;
			uint8_t lcdc;
		};

		void render_line();
		void write_fb();
	public:
		GPU(IO &io, MMU &mmu);

		void reset();
		void step(int cycles);
		uint8_t read8(uint16_t addr);
		void write8(uint16_t addr, uint8_t value);
	};
}
