#include "emu.h"

int main(int argc, char *argv [] ) {
    Emulator emu = Emulator();
    //emu.nes_test();
    emu.run("Donkey Kong (World) (Rev A).nes");
    return 0;
}