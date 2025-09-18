#include "emu.h"
#include <iostream>

Emulator::Emulator(const char * filename) {
    //Initialize PPU

    //Initialize APU

    //Load ROM
    //Ripped from my CHIP 8 emulator
    this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

    if (romFile.is_open()) {

        unsigned int i = 0;
        int size = romFile.tellg();
        romFile.seekg(0, std::ios::beg);

        //Parse file header
        //Verify file is an ines file (first 4 bytes)
        //Will need to throw an error gracefully
        char byte;

        romFile.read(&byte, 1);

        if (byte != 'N') {

        }

        romFile.read(&byte, 1);
        
        if (byte != 'E') {

        }

        romFile.read(&byte, 1);

        if (byte != 'S') {

        }

        romFile.read(&byte, 1);

        //End-of-file character
        if (byte != 0x1A) {

        }

        //Get the number of PRG-ROM banks
        romFile.read(&byte, 1);
        prg_rom = byte;

        //Get the number of CHR-ROM banks
        romFile.read(&byte, 1);
        chr_rom = byte;

        //Parse header flags
        //This will be done later in more depth
        romFile.read(&byte, 1);
        flag6 = byte;

        romFile.read(&byte, 1);
        flag7 = byte;

        //Load everything into memory based on memory mapper
        //The mapper we use is determined by an 8 bit number whose lower nibble is the upper nibble of flag6 and whose upper nibble is the
        // upper nibble of flag7
        //For now, assume no mapper
        int mapperNum = (flag7 & 0xF0) | (flag6 >> 4);

        //Initialize CPU
        cpu = CPU(mapperNum);
        
        //Instead of using OOP principles to implement mappers, each mapper will have a write function stored in a table

    }
    else {

        std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

    }
}