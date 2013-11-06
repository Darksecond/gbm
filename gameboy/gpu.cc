#include "gpu.h"
#include <cstdio>
#include <cstring>

GB::GPU::GPU(IO &io) : io(io) {
	reset();
}

void GB::GPU::reset() {
	memset(vram, 0, 8192);
	memset(oam, 0, 160);
	memset(framebuffer, 255, 160*144*sizeof(RGB));

	current_line = 0;
	clock = 0;
	mode = 0;

	io.clear(White);
}

void GB::GPU::write_fb() {
	for(int y=0;y<144;++y) {
		for(int x=0;x<160;++x) {
			for(int iy = 0; iy < 4; iy++) {
				for(int ix = 0; ix < 4; ix++) {
					io.set_px(x*4+ix,y*4+iy,framebuffer[y*160+x]);
				}
			}
		}
	}
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
					io.clear(White);
					write_fb();
					io.flip();
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
				render_line();
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

	//TODO more registers
	//if(addr > 0xFF40) printf("[gpu write] [addr 0x%X] [val 0x%X]\n",addr,value);
}

void GB::GPU::render_line() {
	if(LCD_ON) {
		if(BG_ON) {
			const int tile_base = BG_TILE_BASE ? 0x0000 : 0x0800;
			const int map_base = BG_MAP_BASE ? 0x1C00 : 0x1800;

			uint8_t bg_y = (y_scrl + current_line) & 0xFF; //Roll over
			uint8_t tile_y = bg_y / 8;
			uint8_t offset_y = bg_y % 8;

			for(int x = x_scrl; x <= x_scrl + 160;) {
				uint8_t bg_x = x & 0xFF; //Roll over
				uint8_t tile_x = bg_x / 8;
				uint8_t offset_x = bg_x % 8;

				uint8_t map = vram[map_base + (tile_y*32) + tile_x];
				if(BG_TILE_BASE == 0) map ^= 0x80;

				uint8_t *tile = vram + tile_base + (map*16);
				uint8_t *tile_line = tile + (offset_y*2);
				for(int i = offset_x;i<8;++i) {
					uint8_t color = 0;
					color |= (tile_line[0] & (1 << i)) == (1 << i);
					color |= (tile_line[1] & (1 << i)) == (1 << i) ? 2 : 0;
					RGB rgb;
					if(color == 0) rgb = {200,200,200};
					if(color == 1) rgb = {127,127,127};
					if(color == 2) rgb = {96,96,96};
					if(color == 3) rgb = Black;
					//printf("[x %i] [y %i]\n",bg_x-x_scrl-i,current_line);
					framebuffer[(current_line*160)+bg_x-x_scrl-i] = rgb;
				}
				x += 8-offset_x;
			}
		} else {
			//!BG_ON
			for(int i=0;i<160;++i) {
				framebuffer[current_line*160+i] = {200,200,200};
			}
		}
	} else {
		//!LCD_ON
		if(clock >= 60000) {
			write_fb();
			clock -= 60000;
		}
		memset(framebuffer, 255, 160*144*sizeof(RGB));
		current_line = 0;
	}
}
