#pragma once

#include "../IO.h"
#include <cstdint>

namespace GB {

	struct GPU {
		uint8_t vram[8192]; //Video ram
		uint8_t oam[160];   //Object Attribute Memory

		int current_line;
		int clock;
		int mode;
		IO &io;
	public:
		GPU(IO &io);

		void reset();
		void step(int cycles);
		uint8_t read8(uint16_t addr);
		void write8(uint16_t addr, uint8_t value);
	};
}
