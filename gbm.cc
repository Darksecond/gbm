#include "gameboy/cart.h"
#include "gameboy/mmu.h"
#include "gameboy/processor.h"
#include "gameboy/gpu.h"
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
	public:
		System(IO &io) : gpu(io), mmu(cart,gpu), proc(mmu) {
		}

		bool step() {

			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_QUIT:
						return false;
				}
			}

			int cycles = proc.step();
			gpu.step(cycles);
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
	io.clear(White);
	io.create();

	GB::System system(io);
	system.cart.load("../tetris.gb"); //ROM ONLY
	//system.cart.load("../zelda.gb"); //ROM+MBC1+RAM+BATT
	//system.cart.load("../pkmn_blue.gb"); //ROM+MBC3+RAM+BATT
	//system.cart.load("../pkmn_gold.gbc"); //ROM+MBC3+TIMER+RAM+BATT
	//system.cart.load("../zelda_dx.gbc"); //ROM+MBC5+RAM+BATT
	//system.cart.load("../ff_legend.gb"); //ROM+MBC2+BATT
	
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
