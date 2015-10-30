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

		uint8_t current_line;
		int clock;
		int mode;
		MMU &mmu;
		uint8_t x_scrl, y_scrl;
		uint8_t wnd_x, wnd_y;
		uint8_t lyc;
		bool frame_done;

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
	public:
		void write_fb(IO &io);
		GPU(MMU &mmu);

		void reset();
		void step(int cycles);
		uint8_t read8(uint16_t addr);
		void write8(uint16_t addr, uint8_t value);

		bool is_frame_done();
	};
}
