#include "processor.h"
#include <cstdio>

GB::Processor::Processor(MMU& mmu) : mmu(mmu) {
	reset();
}

//TODO Move this to a BIOS
void GB::Processor::reset() {

	//TODO A register changes depending on hardware
	regs.AF = 0x01B0;
	regs.BC = 0x0013;
	regs.DE = 0x00D8;
	regs.HL = 0x014D;

	regs.PC = 0x100;
	regs.SP = 0xFFFE;

	ime = false;
	halt = false;

	//Write IO
	mmu.write8(0xFF05, 0x00); //TIMA
	mmu.write8(0xFF06, 0x00); //TMA
	mmu.write8(0xFF07, 0x00); //TAC
	mmu.write8(0xFF10, 0x80); //NR10
	mmu.write8(0xFF11, 0xBF); //NR11
	mmu.write8(0xFF12, 0xF3); //NR12
	mmu.write8(0xFF14, 0xBF); //NR14
	mmu.write8(0xFF16, 0x3F); //NR21
	mmu.write8(0xFF17, 0x00); //NR22
	mmu.write8(0xFF19, 0xBF); //NR24
	mmu.write8(0xFF1A, 0x7F); //NR30
	mmu.write8(0xFF1B, 0xFF); //NR31
	mmu.write8(0xFF1C, 0x9F); //NR32
	mmu.write8(0xFF1E, 0xBF); //NR33
	mmu.write8(0xFF20, 0xFF); //NR41
	mmu.write8(0xFF21, 0x00); //NR42
	mmu.write8(0xFF22, 0x00); //NR43
	mmu.write8(0xFF23, 0xBF); //NR30
	mmu.write8(0xFF24, 0x77); //NR50
	mmu.write8(0xFF25, 0xF3); //NR51
	mmu.write8(0xFF26, 0xF1); //NR52 (0xF0 on SGB)
	mmu.write8(0xFF40, 0x91); //LCDC
	mmu.write8(0xFF42, 0x00); //SCY
	mmu.write8(0xFF43, 0x00); //SCX
	mmu.write8(0xFF45, 0x00); //LYC
	mmu.write8(0xFF47, 0xFC); //BGP
	mmu.write8(0xFF48, 0xFF); //OBP0
	mmu.write8(0xFF49, 0xFF); //OBP1
	mmu.write8(0xFF4A, 0x00); //WY
	mmu.write8(0xFF4B, 0x00); //WX
	mmu.write8(0xFFFF, 0x00); //IE
}

void GB::Processor::print() {
	printf("State:\n");
	printf("AF %02X%02X\n",regs.A,regs.F.raw);
	printf("BC %02X%02X\n",regs.B,regs.C);
	printf("DE %02X%02X\n",regs.D,regs.E);
	printf("HL %02X%02X\n",regs.H,regs.L);
	printf("SP %04X\n",regs.SP);
	printf("PC %04X\n",regs.PC);

	printf("Flags: ");
	if(regs.F.Z) printf("Z"); else printf("-");
	if(regs.F.N) printf("N"); else printf("-");
	if(regs.F.H) printf("H"); else printf("-");
	if(regs.F.C) printf("C"); else printf("-");
	printf("\n");
	
	printf("IME %i\n",ime);
	printf("HALT %i\n",halt);
	printf("next op 0x%X\n",mmu.read8(regs.PC));
}

int GB::Processor::step() {

	handle_interrupts();

	if(halt == true) { //TODO Halt bug?
		return 20;
	}

	return decode();
}

void GB::Processor::handle_interrupts() {
	const uint8_t IE = mmu.read8(0xFFFF);
	const uint8_t IF = mmu.read8(0xFF0F);
	handle_interrupt(0x01, 0x40, IE, IF);
	handle_interrupt(0x02, 0x48, IE, IF);
	handle_interrupt(0x04, 0x50, IE, IF);
	handle_interrupt(0x08, 0x58, IE, IF);
	handle_interrupt(0x10, 0x60, IE, IF);
}

void GB::Processor::handle_interrupt(uint8_t interrupt, uint16_t vector, uint8_t IE, uint8_t IF) {
	if((IF & interrupt) && (IE & interrupt)) {
		if(ime) {
			mmu.write8(0xFF0F, IF & ~(interrupt));
			ime = false;
			push(regs.PC);
			regs.PC = vector;
			//printf("INT 0x%X\n",interrupt);
		}
		halt = false;
	}
}

int GB::Processor::decode() {
	uint8_t opcode = mmu.read8(regs.PC++);
	int cycles = 0;
	switch(opcode) {
		case 0x00: //NOP
			cycles = 4;
			break;
		case 0x01: //LD BC, nn
			regs.BC = mmu.read16(regs.PC);
			regs.PC += 2;
			cycles = 12;
			break;
		case 0x03: //INC BC
			++regs.BC;
			cycles = 8;
			break;
		case 0x05: //DEC B
			regs.B = dec(regs.B);
			cycles = 4;
			break;
		case 0x06: //LD B, n
			regs.B = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x09: //ADD HL, BC
			regs.HL = ADD16(regs.HL, regs.BC);
			cycles = 8;
			break;
		case 0x0A: //LD A, (BC)
			regs.A = mmu.read8(regs.BC);
			cycles = 8;
			break;
		case 0x0B: //DEC BC
			--regs.BC;
			cycles = 8;
			break;
		case 0x0C: //INC C
			regs.C = inc(regs.C);
			cycles = 4;
			break;
		case 0x0D: //DEC C
			regs.C = dec(regs.C);
			cycles = 4;
			break;
		case 0x0E: //LD C, n
			regs.C = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x11: //LD DE, nn
			regs.DE = mmu.read16(regs.PC);
			regs.PC += 2;
			cycles = 12;
			break;
		case 0x12: //LD (DE), A
			mmu.write8(regs.DE, regs.A);
			cycles = 8;
			break;
		case 0x13: //INC DE
			++regs.DE;
			cycles = 8;
			break;
		case 0x14: //INC D
			regs.D = inc(regs.D);
			cycles = 4;
			break;
		case 0x15: //DEC D
			regs.D = dec(regs.D);
			cycles = 4;
			break;
		case 0x16: //LD D, n
			regs.D = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x18: //JR n
			jr(mmu.read8(regs.PC));
			++regs.PC;
			cycles = 12;
			break;
		case 0x19: //ADD HL, DE
			regs.HL = ADD16(regs.HL, regs.DE);
			cycles = 8;
			break;
		case 0x1A: //LD A, (DE)
			regs.A = mmu.read8(regs.DE);
			cycles = 8;
			break;
		case 0x1B: //DEC DE
			--regs.DE;
			cycles = 8;
			break;
		case 0x1C: //INC E
			regs.E = inc(regs.E);
			cycles = 4;
			break;
		case 0x1D: //DEC E
			regs.E = dec(regs.E);
			cycles = 4;
			break;
		case 0x1E: //LD E, n
			regs.E = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x20: //JR NZ, n
			if(regs.F.Z == 0) {
				jr(mmu.read8(regs.PC));
				++regs.PC;
				cycles = 12;
			} else {
				++regs.PC;
				cycles = 8;
			}
			break;
		case 0x21: //LD HL, nn
			regs.HL = mmu.read16(regs.PC);
			regs.PC += 2;
			cycles = 12;
			break;
		case 0x22: //LDI (HL), A
			mmu.write8(regs.HL++, regs.A);
			cycles = 8;
			break;
		case 0x23: //INC HL
			++regs.HL;
			cycles = 8;
			break;
		case 0x24: //INC H
			regs.H = inc(regs.H);
			cycles = 4;
			break;
		case 0x25: //DEC H
			regs.H = dec(regs.H);
			cycles = 4;
			break;
		case 0x26: //LD H, n
			regs.H = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x28: //JR Z, n
			if(regs.F.Z != 0) {
				jr(mmu.read8(regs.PC));
				++regs.PC;
				cycles = 12;
			} else {
				++regs.PC;
				cycles = 8;
			}
			break;
		case 0x2A: //LDI A, (HL)
			regs.A = mmu.read8(regs.HL++);
			cycles = 8;
			break;
		case 0x2B: //DEC HL
			--regs.HL;
			cycles = 8;
			break;
		case 0x2C: //INC L
			regs.L = inc(regs.L);
			cycles = 4;
			break;
		case 0x2F: //CPL
			regs.A = ~regs.A;
			regs.F.N = 1;
			regs.F.H = 1;
			cycles = 4;
			break;
		case 0x30: //JR NC, n
			if(regs.F.C == 0) {
				jr(mmu.read8(regs.PC));
				++regs.PC;
				cycles = 12;
			} else {
				++regs.PC;
				cycles = 8;
			}
			break;
		case 0x31: //LD SP, nn
			regs.SP = mmu.read16(regs.PC);
			regs.PC += 2;
			cycles = 12;
			break;
		case 0x32: //LDD (HL), A
			mmu.write8(regs.HL--, regs.A);
			cycles = 8;
			break;
		case 0x34: //INC (HL)
			mmu.write8(regs.HL, inc(mmu.read8(regs.HL)));
			cycles = 12;
			break;
		case 0x35: //DEC (HL)
			mmu.write8(regs.HL, dec(mmu.read8(regs.HL)));
			cycles = 12;
			break;
		case 0x36: //LD (HL), n
			mmu.write8(regs.HL, mmu.read8(regs.PC++));
			cycles = 12;
			break;
		case 0x38: //JR C, n
			if(regs.F.C != 0) {
				jr(mmu.read8(regs.PC));
				++regs.PC;
				cycles = 12;
			} else {
				++regs.PC;
				cycles = 8;
			}
			break;
		case 0x3C: //INC A
			regs.A = inc(regs.A);
			cycles = 4;
			break;
		case 0x3D: //DEC A
			regs.A = dec(regs.A);
			cycles = 4;
			break;
		case 0x44: //LD B, H
			regs.B = regs.H;
			cycles = 4;
			break;
		case 0x47: //LD B, A
			regs.B = regs.A;
			cycles = 4;
			break;
		case 0x4C: //LD C, H
			regs.C = regs.H;
			cycles = 4;
			break;
		case 0x4D: //LD C, L
			regs.C = regs.L;
			cycles = 4;
			break;
		case 0x4F: //LD C, A
			regs.C = regs.A;
			cycles = 4;
			break;
		case 0x53: //LD D, E
			regs.D = regs.E;
			cycles = 4;
			break;
		case 0x54: //LD D, H
			regs.D = regs.H;
			cycles = 4;
			break;
		case 0x56: //LD D, (HL)
			regs.D = mmu.read8(regs.HL);
			cycles = 8;
			break;
		case 0x57: //LD D, A
			regs.D = regs.A;
			cycles = 4;
			break;
		case 0x58: //LD E, B
			regs.E = regs.B;
			cycles = 4;
			break;
		case 0x5D: //LD E, L
			regs.E = regs.L;
			cycles = 4;
			break;
		case 0x5F: //LD E, A
			regs.E = regs.A;
			cycles = 4;
			break;
		case 0x3E: //LD A, n
			regs.A = mmu.read8(regs.PC++);
			cycles = 8;
			break;
		case 0x5E: //LD E, (HL)
			regs.E = mmu.read8(regs.HL);
			cycles = 8;
			break;
		case 0x61: //LD H, C
			regs.H = regs.C;
			cycles = 4;
			break;
		case 0x62: //LD H, D
			regs.H = regs.D;
			cycles = 4;
			break;
		case 0x67: //LD H, A
			regs.H = regs.A;
			cycles = 4;
			break;
		case 0x6B: //LD L, E
			regs.L = regs.E;
			cycles = 4;
			break;
		case 0x6F: //LD L, A
			regs.L = regs.A;
			cycles = 4;
			break;
		case 0x70: //LD (HL), B
			mmu.write8(regs.HL, regs.B);
			cycles = 8;
			break;
		case 0x71: //LD (HL), C
			mmu.write8(regs.HL, regs.C);
			cycles = 8;
			break;
		case 0x76: //HALT
			//TODO Halt bug
			if(ime)
				halt = 1;
			cycles = 4;
			break;
		case 0x77: //LD (HL), A
			mmu.write8(regs.HL, regs.A);
			cycles = 8;
			break;
		case 0x78: //LD A, B
			regs.A = regs.B;
			cycles = 4;
			break;
		case 0x79: //LD A, C
			regs.A = regs.C;
			cycles = 4;
			break;
		case 0x7A: //LD A, D
			regs.A = regs.D;
			cycles = 4;
			break;
		case 0x7B: //LD A, E
			regs.A = regs.E;
			cycles = 4;
			break;
		case 0x7C: //LD A, H
			regs.A = regs.H;
			cycles = 4;
			break;
		case 0x7D: //LD A, L
			regs.A = regs.L;
			cycles = 4;
			break;
		case 0x7E: //LD A, (HL)
			regs.A = mmu.read8(regs.HL);
			cycles = 8;
			break;
		case 0x80: //ADD A, B
			regs.A = ADD(regs.A, regs.B);
			cycles = 4;
			break;
		case 0x83: //ADD A, E
			regs.A = ADD(regs.A, regs.E);
			cycles = 4;
			break;
		case 0x86: //ADD A, (HL)
			regs.A = ADD(regs.A, mmu.read8(regs.HL));
			cycles = 8;
			break;
		case 0x87: //ADD A, A
			regs.A = ADD(regs.A, regs.A);
			cycles = 4;
			break;
		case 0x90: //SUB A, B
			regs.A = sub(regs.A, regs.B);
			cycles = 4;
			break;
		case 0x91: //SUB A, C
			regs.A = sub(regs.A, regs.C);
			cycles = 4;
			break;
		case 0x92: //SUB A, D
			regs.A = sub(regs.A, regs.D);
			cycles = 4;
			break;
		case 0x97: //SUB A, A
			regs.A = sub(regs.A, regs.A);
			cycles = 4;
			break;
		case 0xA0: //AND B
			regs.A = AND(regs.A, regs.B);
			cycles = 4;
			break;
		case 0xA1: //AND C
			regs.A = AND(regs.A, regs.C);
			cycles = 4;
			break;
		case 0xA7: //AND A
			regs.A = AND(regs.A, regs.A);
			cycles = 4;
			break;
		case 0xA9: //XOR C
			regs.A = XOR(regs.A, regs.C);
			cycles = 4;
			break;
		case 0xAF: //XOR A
			regs.A = XOR(regs.A, regs.A);
			cycles = 4;
			break;
		case 0xB0: //OR B
			regs.A = OR(regs.A, regs.B);
			cycles = 4;
			break;
		case 0xB1: //OR C
			regs.A = OR(regs.A, regs.C);
			cycles = 4;
			break;
		case 0xB2: //OR D
			regs.A = OR(regs.A, regs.D);
			cycles = 4;
			break;
		case 0xB3: //OR E
			regs.A = OR(regs.A, regs.E);
			cycles = 4;
			break;
		case 0xB6: //OR (HL)
			regs.A = OR(regs.A, mmu.read8(regs.HL));
			cycles = 8;
			break;
		case 0xB9: //CP C
			sub(regs.A, regs.C);
			cycles = 4;
			break;
		case 0xC0: //RET NZ
			if(regs.F.Z == 0) {
				ret();
				cycles = 20;
			} else {
				cycles = 8;
			}
			break;
		case 0xC1: //POP BC
			regs.BC = pop();
			cycles = 12;
			break;
		case 0xC2: //JP NZ, nn
			if(regs.F.Z == 0) {
				regs.PC = mmu.read16(regs.PC);
				cycles = 16;
			} else {
				regs.PC += 2;
				cycles = 12;
			}
			break;
		case 0xC3: //JP nn
			regs.PC = mmu.read16(regs.PC);
			cycles = 12;
			break;
		case 0xC5: //PUSH BC
			push(regs.BC);
			cycles = 16;
			break;
		case 0xC6: //ADD A, n
			regs.A = ADD(regs.A, mmu.read8(regs.PC++));
			cycles = 8;
			break;
		case 0xC7: //RST 0x00
			rst(0x00);
			cycles = 16;
			break;
		case 0xC8: //RET Z
			if(regs.F.Z != 0) {
				ret();
				cycles = 20;
			} else {
				cycles = 8;
			}
			break;
		case 0xC9: //RET
			ret();
			cycles = 16;
			break;
		case 0xCA: //JP Z, nn
			if(regs.F.Z != 0) {
				regs.PC = mmu.read16(regs.PC);
				cycles = 16;
			} else {
				regs.PC += 2;
				cycles = 12;
			}
			break;
		case 0xCB: //extended instruction set
			{
				uint8_t opcode2 = mmu.read8(regs.PC++);
				switch(opcode2) {
					case 0x12: //RL D
						regs.D = rl(regs.D);
						cycles = 8;
						break;
					case 0x1C: //RR H
						regs.H = rr(regs.H);
						cycles = 8;
						break;
					case 0x1D: //RR L
						regs.L = rr(regs.L);
						cycles = 8;
						break;
					case 0x23: //SLA H
						regs.H = sla(regs.H);
						cycles = 8;
						break;
					case 0x27: //SLA A
						regs.A = sla(regs.A);
						cycles = 8;
						break;
					case 0x37: //SWAP A
						regs.A = swap(regs.A);
						cycles = 8;
						break;
					case 0x3C: //SRL H
						regs.H = srl(regs.H);
						cycles = 8;
						break;
					case 0x3D: //SRL L
						regs.L = srl(regs.L);
						cycles = 8;
						break;
					case 0x3F: //SRL A
						regs.A = srl(regs.A);
						cycles = 8;
						break;
					case 0x47: //BIT 0, A
						bit(regs.A, 0);
						cycles = 8;
						break;
					case 0x4F: //BIT 1, A
						bit(regs.A, 1);
						cycles = 8;
						break;
					case 0x67: //BIT 4, A
						bit(regs.A, 4);
						cycles = 8;
						break;
					case 0x6F: //BIT 5, A
						bit(regs.A, 5);
						cycles = 8;
						break;
					case 0x77: //BIT 6, A
						bit(regs.A, 6);
						cycles = 8;
						break;
					case 0x87: //RES 0,A
						regs.A = res(regs.A, 0);
						cycles = 8;
						break;
					default:
						printf("invalid opcode 0x%X%X at 0x%X\n",opcode,opcode2,regs.PC-2);
						cycles = 0;
						break;
				}
			}
			break;
		case 0xCD: //CALL nn
			call(mmu.read16(regs.PC));
			cycles = 24;
			break;
		case 0xD1: //POP DE
			regs.DE = pop();
			cycles = 12;
			break;
		case 0xD5: //PUSH DE
			push(regs.DE);
			cycles = 16;
			break;
		case 0xD6: //SUB A, n
			regs.A = sub(regs.A, mmu.read8(regs.PC++));
			cycles = 8;
			break;
		case 0xD9: //RETI
			ret();
			ime = true;
			cycles = 16;
			break;
		case 0xDA: //JP C, nn
			if(regs.F.C != 0) {
				regs.PC = mmu.read16(regs.PC);
				cycles = 16;
			} else {
				regs.PC += 2;
				cycles = 12;
			}
			break;
		case 0xE0: //LDH (n), A
			mmu.write8(mmu.read8(regs.PC++) + 0xFF00, regs.A);
			cycles = 12;
			break;
		case 0xE1: //POP HL
			regs.HL = pop();
			cycles = 12;
			break;
		case 0xE2: //LD (C), A
			mmu.write8(0xFF00 + regs.C, regs.A);
			cycles = 8;
			break;
		case 0xE5: //PUSH HL
			push(regs.HL);
			cycles = 16;
			break;
		case 0xE6: //AND n
			regs.A = AND(regs.A, mmu.read8(regs.PC++));
			cycles = 8;
			break;
		case 0xE9: //JP HL
			regs.PC = regs.HL;
			cycles = 4;
			break;
		case 0xEA: //LD (nn), A
			mmu.write8(mmu.read16(regs.PC), regs.A);
			regs.PC += 2;
			cycles = 16;
			break;
		case 0xEE: //XOR n
			regs.A = XOR(regs.A, mmu.read8(regs.PC++));
			cycles = 8;
			break;
		case 0xEF: //RST 0x28
			rst(0x28);
			cycles = 16;
			break;
		case 0xF0: //LDH A, (n)
			regs.A = mmu.read8(0xFF00 + mmu.read8(regs.PC++));
			cycles = 12;
			break;
		case 0xF1: //POP AF
			regs.AF = pop();
			cycles = 12;
			break;
		case 0xF3: //DI
			//TODO docs say this should be only in affect from after the next instruction on
			ime = 0;
			cycles = 4;
			break;
		case 0xF5: //PUSH AF
			push(regs.AF);
			cycles = 16;
			break;
		case 0xFA: //LD A, (nn)
			regs.A = mmu.read8(mmu.read16(regs.PC));
			regs.PC += 2;
			cycles = 16;
			break;
		case 0xFB: //EI
			//TODO docs say this should be only in affect from after the next instruction on
			ime = 1;
			cycles = 4;
			break;
		case 0xFE: //CP n
			sub(regs.A, mmu.read8(regs.PC++));
			cycles = 8;
			break;
		case 0xFF: //RST 0x38
			rst(0x38);
			cycles = 16;
			break;
		default:
			printf("invalid opcode 0x%X at 0x%X\n",opcode,regs.PC-1);
			cycles = 0;
			break;
	}

	return cycles;
}
