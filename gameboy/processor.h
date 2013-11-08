#pragma once

#include "mmu.h"
#include "../util.h"

namespace GB {

	struct Processor {
		struct {
			union {
				uint16_t AF;
				struct {
					union {
						uint8_t raw;
						RegBit<4> C;
						RegBit<5> H;
						RegBit<6> N;
						RegBit<7> Z;
					} F;
					uint8_t A;
				};
			};
			union {
				uint16_t BC;
				struct {
					uint8_t C;
					uint8_t B;
				};
			};
			union {
				uint16_t DE;
				struct {
					uint8_t E;
					uint8_t D;
				};
			};
			union {
				uint16_t HL;
				struct {
					uint8_t L;
					uint8_t H;
				};
			};
			uint16_t SP;
			uint16_t PC;
		} regs;

		MMU& mmu; //Memory mapper

		bool ime;
		bool halt;

		inline void push(uint16_t a) {
			regs.SP -= 2;
			mmu.write16(regs.SP, a);
		}

		inline void call(uint16_t a) {
			push(regs.PC + 2);
			regs.PC = a;
		}

		inline uint8_t XOR(uint8_t a, uint8_t b) {
			uint8_t tmp = a ^ b;
			regs.F.N = 0;
			regs.F.H = 0;
			regs.F.C = 0;
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			return tmp;
		}

		inline uint8_t dec(uint8_t a) {
			--a;
			//has the lower nibble borrowed? if so, no carry
			if((a & 0x0F) == 0x0F)
				regs.F.H = 1;
			else
				regs.F.H = 0;
			if(a==0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 1;
			return a;
		}

		inline void jr(uint8_t a) {
			if((a & 0x80) == 0x80) { //a is negative
				//remove two's complement
				--a;
				a = ~a;
				regs.PC -= a;
			} else {
				regs.PC += a;
			}
		}

		inline uint8_t sub(uint8_t a, uint8_t b) {
			uint8_t tmp = a - b;
			//will there be a borrow? if so, no carry
			if(a < b)
				regs.F.C = 1;
			else
				regs.F.C = 0;
			//will the lower nibble borrow? if so, no carry
			if((a & 0x0F) < (b & 0x0F))
				regs.F.H = 1;
			else
				regs.F.H = 0;
			//if applicable, set zero flag.
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 1; //set subtract flag to 1
			return tmp;
		}

		inline uint8_t res(uint8_t a, uint8_t b) {
			return a &= ~(1 << b);
		}

		inline void rst(uint8_t a) {
			push(regs.PC);
			regs.PC = a;
		}

		inline uint8_t AND(uint8_t a, uint8_t b) {
			uint8_t tmp = a & b;
			regs.F.N = 0;
			regs.F.H = 1;
			regs.F.C = 0;
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			return tmp;
		}

		inline uint16_t pop() {
			regs.SP += 2;
			return mmu.read16(regs.SP - 2);
		}

		inline void ret() {
			regs.PC = pop();
		}

		inline uint8_t OR(uint8_t a, uint8_t b) {
			uint8_t tmp = a | b;
			regs.F.N = 0;
			regs.F.H = 1;
			regs.F.C = 0;
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			return tmp;
		}

		inline uint8_t sla(uint8_t a) {
			regs.F.C = (a & 0x80) >> 7;
			a <<= 1;
			if(a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 0;
			return a;
		}

		inline uint8_t rl(uint8_t a) {
			uint8_t tmp = regs.F.C;
			regs.F.C = (a & 0x80) >> 7;
			a = (a << 1) + tmp;
			if (a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 0;
			return a;
		}

		inline uint8_t inc(uint8_t a) {
			++a;
			if((a & 0x0F) == 0)
				regs.F.H = 1;
			else
				regs.F.H = 0;
			if(a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			return a;
		}

		inline uint8_t swap(uint8_t a) {
			uint8_t tmp = a & 0x0F;
			a >>= 4;
			a |= (tmp << 4);
			if(a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 0;
			regs.F.C = 0;
			return a;
		}

		inline uint8_t ADD(uint8_t a, uint8_t b) {
			uint8_t tmp = a + b;
			if(0xFF - a < b)
				regs.F.C = 1;
			else
				regs.F.C = 0;
			if(0x0F - (a & 0x0F) < (b & 0x0F))
				regs.F.H = 1;
			else
				regs.F.H = 0;
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			return tmp;
		}

		inline uint16_t ADD16(uint16_t a, uint16_t b) {
			uint16_t tmp = a + b;
			if(0xFFFF - a < b)
				regs.F.C = 1;
			else
				regs.F.C = 0;
			if(0x0FFF - (a & 0x0FFF) < (b & 0x0FFF))
				regs.F.H = 1;
			else
				regs.F.H = 0;
			if(tmp == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			return tmp;
		}

		inline void bit(uint8_t reg, uint8_t b) {
			if((reg & (1 << b)) == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 1;

		}

		inline uint8_t srl(uint8_t a) {
			regs.F.C = a & 0x01;
			a >>= 1;
			if(a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 0;
			return a;
		}

		inline uint8_t rr(uint8_t a) {
			int tmp = regs.F.C;
			regs.F.C = a & 0x01;
			a = (a >> 1) + (tmp << 7);
			if(a == 0)
				regs.F.Z = 1;
			else
				regs.F.Z = 0;
			regs.F.N = 0;
			regs.F.H = 0;
			return a;
		}

		void handle_interrupts();
		void handle_interrupt(uint8_t interrupt,uint16_t vector,uint8_t IE,uint8_t IF);
		int decode();
	public:
		Processor(MMU& mmu);

		void reset();
		void print();
		int step();
	};
}
