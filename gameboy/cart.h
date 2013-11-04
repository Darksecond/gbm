#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstdio>

namespace GB {

	//TODO extract MBC's into their own struct
	struct Cart {
		uint8_t *rom; //Raw cartridge rom
		uint8_t *eram; //External cartridge ram
		struct {
			int mbc;
			bool ram;
			bool batt;
			bool rumble;
			bool timer;

			void parse(uint8_t raw) {
				mbc = 0;
				ram = false;
				batt = false;
				rumble = false;
				timer = false;
				switch(raw) {
					case 0x00:
						mbc = 0;
						break;
					case 0x01:
						mbc = 1;
						break;
					case 0x02:
						mbc = 1;
						ram = true;
						break;
					case 0x03:
						mbc = 1;
						ram = true;
						batt = true;
						break;
					case 0x05:
						mbc = 2;
						break;
					case 0x06:
						mbc = 2;
						batt = true;
						break;
					case 0x08:
						mbc = 0;
						ram = true;
						break;
					case 0x09:
						mbc = 0;
						ram = true;
						batt = true;
						break;
					//case 0x0B: //ROM+MMM01
					//case 0x0C: //ROM+MMM01+SRAM
					//case 0x0D: //ROM+MMM01+SRAM+BATT
					case 0x0F:
						mbc = 3;
						batt = true;
						timer = true;
						break;
					case 0x10:
						mbc = 3;
						ram = true;
						batt = true;
						timer = true;
						break;
					case 0x11:
						mbc = 3;
						break;
					case 0x12:
						mbc = 3;
						ram = true;
						break;
					case 0x13:
						mbc = 3;
						ram = true;
						batt = true;
						break;
					case 0x19:
						mbc = 5;
						break;
					case 0x1A:
						mbc = 5;
						ram = true;
						break;
					case 0x1B:
						mbc = 5;
						ram = true;
						batt = true;
						break;
					case 0x1C:
						mbc = 5;
						rumble = true;
						break;
					case 0x1D:
						mbc = 5;
						rumble = true;
						ram = true;
						break;
					case 0x1E:
						mbc = 5;
						rumble = true;
						ram = true;
						batt = true;
						break;
					//case 0x1F: //Pocket camera
					//case 0xFD: //Bandai TAMA5
					//case 0xFE: //Hudson HuC-3
					//case 0xFF: //Hudson HuC-1+RAM+BATTERY
				}
			}
		} mbc_type;

		size_t rom_offset;
		int rom_bank;
		size_t ram_offset;
		int mbc_mode;

		size_t eram_size() {
			assert(rom);
			//if(mbc_type.mbc == 2) return 512;
			switch(rom[0x0149]) {
				case 0: return 0;
				case 1: return 2*1024; //actually 2*1024, but 8 for overflow-safety
				case 2: return 8*1024;
				case 3: return 32*1024;
				case 4: return 128*1024;
				default: return 0; //failure status
			}
		}

		uint8_t eram_banks() {
			assert(rom);
			switch(rom[0x0149]) {
				case 0: return 0;
				case 1: return 1;
				case 2: return 1;
				case 3: return 4;
				case 4: return 16;
				default: return 0; //failure status
			}
		}

		uint8_t read_rom(uint16_t addr) {
			if(addr < 0x4000)
				return rom[addr]; //Rom, bank 0
			else if(addr < 0x8000) {
				return rom[addr-0x4000+rom_offset];
			}
			return 0; //failure state
		}

		uint8_t read_eram(uint16_t addr) {
			uint16_t real_addr = addr-0xA000+ram_offset;
			if(eram_size() > 0) {
				return eram[addr-0xA000+ram_offset];
			} else {
				return 0;
			}
		}

		void write_rom(uint16_t addr, uint8_t value) {
			switch(mbc_type.mbc) {
				case 1:
					if(addr >= 0x2000 && addr < 0x4000) {
						//Rom bank 1 switch
						rom_bank &= 0x60;
						value &= 0x1F;
						if(value == 0) value = 1;
						rom_bank |= value;
						rom_offset = rom_bank * 0x4000;
					}
					if(addr >= 0x4000 && addr < 0x6000) {
						//Ram bank switch
						if(mbc_mode == 1) {
							ram_offset = (value & 0x03) * 0x2000;
						} else {
							rom_bank &= 0x1F;
							rom_bank |= ((value & 0x03) << 5);
							rom_offset = rom_bank * 0x4000;
						}
					}
					if(addr >= 0x6000 && addr < 0x8000) {
						//mode switch
						mbc_mode = value & 0x01;
					}
					break;
				case 2:
					if(addr >= 0x2000 && addr < 0x4000) {
						//Rom bank 1 switch
						value &= 0x0F;
						if(value == 0) value = 1;
						rom_bank = value;
						rom_offset = rom_bank * 0x4000;
					}
					break;
				case 3: //TODO
					break;
				case 5: //TODO
					break;
				default:
					//failure state
					break;
			}
		}

		void write_eram(uint16_t addr, uint8_t value) {
			if(eram_size() == 0) return; //failure state
			switch(mbc_type.mbc) {
				case 2:
					eram[addr-0xA000+ram_offset] = value & 0x0F;
					break;
				default:
					eram[addr-0xA000+ram_offset] = value;
					break;
			}
		}
	public:
		Cart() : rom(nullptr), eram(nullptr) {
			unload();
		}

		void unload() {
			if(rom) delete [] rom;
			if(eram) delete [] eram;
		}

		void reset() {
			rom_offset = 0x4000;
			rom_bank = 1;
			ram_offset = 0x0;
			mbc_mode = 0; //16Mbit ROM/8KByte RAM
		}

		void load(const char* filename) {
			unload();
			FILE *fp = fopen(filename, "rb");
			//Determine file size
			fseek(fp, 0, SEEK_END);
			size_t size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			rom = new uint8_t[size];
			fread(rom, 1, size, fp);
			fclose(fp);

			mbc_type.parse(rom[0x0147]);

			eram = new uint8_t[eram_size()];

			printf("rom %s loaded (%zu bytes)\n", filename, size);
			printf("\t[title %.16s]\n", &rom[0x0134]);
			printf("\t[mbc %u] [ram %u] [batt %u] [timer %u] [rumble %u]\n",mbc_type.mbc, mbc_type.ram, mbc_type.batt, mbc_type.timer, mbc_type.rumble);
			printf("\t[eram %u bank(s) (%zu bytes)]\n", eram_banks(), eram_size());

			reset();
		}

		uint8_t read8(uint16_t addr) {
			if(!rom) return 0;
			if(addr < 0x8000) return read_rom(addr);
			if(addr >= 0xA000 && addr < 0xC000) return read_eram(addr);
			return 0; //failure state
		}

		void write8(uint16_t addr, uint8_t value) {
			if(!rom) return;
			if(addr < 0x8000) write_rom(addr, value);
			if(addr >= 0xA000 && addr < 0xC000) write_eram(addr, value);
		}
	};
}
