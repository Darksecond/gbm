#include "gpu.h"
#include <cstdio>
#include <cstring>

GB::GPU::GPU(IO &io) : io(io) {
	reset();
}

void GB::GPU::reset() {
	memset(vram, 0, 8192);
	memset(oam, 0, 160);

	current_line = 0;
	clock = 0;
	mode = 0;
}

void GB::GPU::step(int cycles) {
	clock += cycles;

	switch(mode) {
		case 0: //HBlank
			if(clock >= 204) {
				clock = 0;
				++current_line;
				if(current_line == 143) {
					mode = 1;
					io.flip();
					//io.clear(White);
				} else {
					mode = 2;
				}
			}
			break;
		case 1: //Vblank
			if(clock >= 456) {
				clock = 0;
				++current_line;
				if(current_line > 153) {
					mode = 2;
					current_line = 0;
				}
			}
			break;
		case 2: //OAM Read mode, scanline active
			if(clock >= 80) {
				clock = 0;
				mode = 3;
			}
			break;
		case 3: //VRAM Read mode, scanline active. Treat end of mode 3 as end of scanline.
			if(clock >= 172) {
				clock = 0;
				mode = 0;
				//render_scan();
			}
			break;
	}
}

uint8_t GB::GPU::read8(uint16_t addr) {
	     if(addr >= 0x8000 && addr < 0xA000) return vram[addr & 0x1FFF]; //VRAM
	else if(addr >= 0xFE00 && addr < 0xFEA0) return oam[addr & 0xFF];    //OAM (Object Attribute Memory)

	//else if(addr >= 0xFF00 && addr < 0xFF80); //MMIO
	       if(addr == 0xFF40) {
		return lcdc;
	} else if(addr == 0xFF41) {
		return ((current_line==lyc)?4:0) | (mode & 3);
	} else if(addr == 0xFF42) {
		return y_scrl;
	} else if(addr == 0xFF43) {
		return x_scrl;
	} else if(addr == 0xFF44) {
		return current_line;
	} else if(addr == 0xFF45) {
		return lyc;
	}

	printf("[gpu read] [addr 0x%X]\n",addr);
	return 0;
}

void GB::GPU::write8(uint16_t addr, uint8_t value) {
	     if(addr >= 0x8000 && addr < 0xA000) vram[addr & 0x1FFF] = value; //VRAM
	else if(addr >= 0xFE00 && addr < 0xFEA0) oam[addr & 0xFF] = value;    //OAM (Object Attribute Memory)

	//else if(addr >= 0xFF00 && addr < 0xFF80); //MMIO

	//if(addr > 0xFEA0)
	//printf("[write] [addr 0x%X] [val 0x%X]\n",addr, value);

	       if(addr == 0xFF40) {
		lcdc = value;
	} else if(addr == 0xFF41) {
		//TODO interrupt flags, currently not used
	} else if(addr == 0xFF42) {
		y_scrl = value;
	} else if(addr == 0xFF43) {
		x_scrl = value;
	} else if(addr == 0xFF45) {
		lyc = value;
	}

	if(addr > 0xFF45) printf("[gpu write] [addr 0x%X] [val 0x%X]\n",addr,value);
}
