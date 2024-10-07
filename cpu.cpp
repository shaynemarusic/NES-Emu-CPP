#include "cpu.h"

//The main constructor
CPU::CPU(const char * filename) {

    //Initialize memory
    this->memory = std::unique_ptr<int8_t[]>(new int8_t[65536]);
    std::fill_n(this->memory.get(), 65536, 0);

    this->stackPointer = 0xFF;
    this->programCounter = 0xFFFC;

    //Open the rom file - worry about the details of this later
    //this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

}

//Decodes and executes instructions
void CPU::decode() {

    uint8_t upper, lower;
    upper = this->memory[programCounter + 1];
    lower = this->memory[programCounter];
    programCounter++;

    switch (lower) {

        case 0:
            break;
        case 1:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 8:
            break;
        case 9:
            break;
        case 0xA:
            break;
        case 0xC:
            break;
        case 0xD:
            break;
        case 0xE:
            break;

    }
}