#pragma once
#include <cstdint>

namespace GB {

	struct Input {
		uint8_t P1;
	public:
		Input();
		void step();
		uint8_t read8(uint16_t addr);
		void write8(uint16_t addr, uint8_t value);
	};
}
