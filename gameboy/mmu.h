#pragma once
#include <cstdint>

namespace GB {

	struct Cart;
	struct GPU;
	struct Input;
	struct MMU {
		uint8_t wram[8192]; //Working ram
		uint8_t zram[128];  //Zero (fast) ram
		uint8_t IF;
		Cart& cart;
		GPU& gpu;
		Input& input;
	public:
		MMU(Cart& cart, GPU& gpu, Input& input);

		void reset();
		uint8_t read8(uint16_t addr);
		void write8(uint16_t addr, uint8_t value);
		uint16_t read16(uint16_t addr);
		void write16(uint16_t addr, uint16_t value);
	};
}
