#include "mmu.h"
#include <cstring>

GB::MMU::MMU(Cart& cart, GPU& gpu) : cart(cart), gpu(gpu) {
	reset();
}

void GB::MMU::reset() {
	memset(wram, 0, 8192);
	memset(zram, 0, 128);
}

uint8_t GB::MMU::read8(uint16_t addr) {
	//TODO More memory things

	     if(addr >= 0x0000 && addr < 0x4000) return cart.read8(addr);    //Rom, bank 0
	else if(addr >= 0x4000 && addr < 0x8000) return cart.read8(addr);    //Rom, bank 1
	else if(addr >= 0x8000 && addr < 0xA000) return  gpu.read8(addr);    //VRAM
	else if(addr >= 0xA000 && addr < 0xC000) return cart.read8(addr);    //External cartridge ram
	else if(addr >= 0xC000 && addr < 0xE000) return wram[addr & 0x1FFF]; //working ram
	else if(addr >= 0xE000 && addr < 0xFE00) return wram[addr & 0x1FFF]; //shadow working ram
	else if(addr >= 0xFE00 && addr < 0xFEA0) return gpu.read8(addr);     //OAM (Object Attribute Memory)
	else if(addr >= 0xFEA0 && addr < 0xFF00) return 0;                   //Unusable
	
	//else if(addr >= 0xFF00 && addr < 0xFF80); //MMIO (TODO)
	//START VIDEO REGS
	else if(addr >= 0xFF40 && addr < 0xFF56) return gpu.read8(addr); //GPU register(s)
	//END VIDEO REGS

	else if(addr >= 0xFF80 && addr <= 0xFFFF) return zram[addr & 0x7F];   //Zero ram

	//printf("[mmu read] [addr 0x%X]\n",addr);
	return 0; //failure state
}

void GB::MMU::write8(uint16_t addr, uint8_t value) {
	//TODO More memory things
	
	     if(addr >= 0x0000 && addr < 0x4000) cart.write8(addr, value);    //Rom, bank 0
	else if(addr >= 0x4000 && addr < 0x8000) cart.write8(addr, value);    //Rom, bank 1
	else if(addr >= 0x8000 && addr < 0xA000)  gpu.write8(addr, value);    //VRAM
	else if(addr >= 0xA000 && addr < 0xC000) cart.write8(addr, value);    //External cartridge ram
	else if(addr >= 0xC000 && addr < 0xE000) wram[addr & 0x1FFF] = value; //working ram
	else if(addr >= 0xE000 && addr < 0xFE00) wram[addr & 0x1FFF] = value; //shadow working ram
	else if(addr >= 0xFE00 && addr < 0xFEA0)  gpu.write8(addr, value);    //OAM (Object Attribute Memory)
	//else if(addr >= 0xFEA0 && addr < 0xFF00); //Unusable

	//else if(addr >= 0xFF00 && addr < 0xFF80) printf("[mmu write] [addr 0x%X] [val 0x%X]\n",addr,value); //MMIO
	//START VIDEO REGS
	else if(addr >= 0xFF40 && addr < 0xFF56) gpu.write8(addr, value); //GPU register(s)
	//END VIDEO REGS
	else if(addr >= 0xFF80 && addr <= 0xFFFF) zram[addr & 0x7F] = value;   //Zero ram
	
	//failure state
}

uint16_t GB::MMU::read16(uint16_t addr) {
	return (read8(addr+1) << 8) + read8(addr);
}

void GB::MMU::write16(uint16_t addr, uint16_t value) {
	write8(addr, value & 0xFF);
	write8(addr+1, value >> 8);
}
