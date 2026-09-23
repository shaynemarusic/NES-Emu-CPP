#include "emu.h"

#undef main

int main(int argc, char *argv [] ) {
    Emulator emu = Emulator();
    //emu.nes_test();
    //emu.run("Donkey Kong (World) (Rev A).nes");
    //emu.blargg();
    emu.run_log("03-immediate.nes");
    return 0;
}