#include "cpu.h"

class Emulator {

    private:
        CPU cpu;
        //PPU ppu;
        //APU apu;
        std::fstream romFile;
        //Size of PRG ROM in 16KB units
        int prg_rom;
        //Size CHR ROM in 8KB units; 0 indicates use of CHR RAM
        int chr_rom;
        //iNES header flags used for determining which memory mapper to use
        int flag6;
        int flag7;
        int mapperNum;
    public:
        Emulator(const char * filename);
};

int main(int argc, char *argv []) {
    /*
    Steps:
    Instantiate 'parts' of the emulator
    Load the ROM/program into emulator memory
    Enter while loop where:
        listen for interrupts
        decode and execute instructions
    */
    return 0;
}