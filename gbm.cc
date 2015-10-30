#include "gameboy/cart.h"
#include "gameboy/mmu.h"
#include "gameboy/processor.h"
#include "gameboy/gpu.h"
#include "gameboy/input.h"
#include "IO.h"
#include <SDL.h>
#include <cstdio>
#include <cstdint>
#include <cassert>

//TODO Make a BusDevice and Bus
//     With read8,write8,read16,write16
//     And register_{addr,range}
namespace GB {

	struct System {
		GB::Cart cart;
		GB::GPU gpu;
		GB::MMU mmu;
		GB::Processor proc;
		GB::Input input;
		IO &io;
		uint32_t prev;
		uint32_t cycle_count;
		uint32_t clock;
	public:
		System(IO &io) : io(io), gpu(io,mmu), mmu(cart,gpu,input), proc(mmu) {
			prev = SDL_GetTicks();
		}

		bool step() {
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_QUIT:
						return false;
				}
			}

			int cycles = 0;
			for(int i=0;i<4000;++i) {
				input.step();
				int icycles = proc.step();
				gpu.step(icycles);
				cycles += icycles;
				if(icycles == 0) return 0;
			}

			uint32_t current = SDL_GetTicks();
			uint32_t delta = current - prev;
			prev = current;

			cycle_count += cycles;
			clock += delta;
			if(clock > 1000) {
				char title[100];
				sprintf(title, "GBM | %fMhz\n",cycle_count/(clock/1000.0)/1000000.0);
				io.set_title(title);
				clock = 0;
				cycle_count = 0;
			}

			return cycles > 0;
		}
	};
}

int main(int argc, char* argv[]) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr,"sdl initialization failed: %s\b", SDL_GetError());
		exit(1);
	}

	IO io;
	io.create();

	GB::System system(io);
	//system.cart.load("/Users/darksecond/build/gbm/tetris.gb"); //ROM ONLY
	//system.cart.load("../zelda.gb"); //ROM+MBC1+RAM+BATT
	//system.cart.load("../pkmn_blue.gb"); //ROM+MBC3+RAM+BATT
	//system.cart.load("../pkmn_gold.gbc"); //ROM+MBC3+TIMER+RAM+BATT
	//system.cart.load("../zelda_dx.gbc"); //ROM+MBC5+RAM+BATT
	//system.cart.load("../ff_legend.gb"); //ROM+MBC2+BATT
	//system.cart.load("../opus5.gb");
	system.cart.load(argv[1]);
	
	bool running = true;
	while(running) {

		printf("> ");
		char str[80];
		scanf("%s",str);
		if(strcmp(str, "quit")==0) {
			running = false;
		} else if(strcmp(str, "show")==0) {
			system.proc.print();
		} else if(strcmp(str, "step")==0) {
			system.proc.step();
			system.proc.print();
		} else if(strcmp(str, "run")==0) {
			while(system.step()); //Run until we come across a invalid opcode
		}
	}

	SDL_Quit();
	return 0;
}
