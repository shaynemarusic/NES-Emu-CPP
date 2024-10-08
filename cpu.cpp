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

    uint8_t opcode;
    opcode = this->memory[programCounter];
    programCounter++;

    //Switch over the low order nibble
    switch (opcode) {

        //BRK impl
        case 0x00:
            break;
        //ORA X, ind
        case 0x01:
            break;
        //ORA zpg
        case 0x05:
            break;
        //ASL zpg
        case 0x06:
            break;
        //PHP impl
        case 0x08:
            break;
        //ORA #
        case 0x09:
            break;
        //ASL A
        case 0x0A:
            break;
        //ORA abs
        case 0x0D:
            break;
        //ASL abs
        case 0x0E:
            break;
        //BPL rel
        case 0x10:
            break;
        //ORA ind, Y
        case 0x11:
            break;
        //ORA zpg, X
        case 0x15:
            break;
        //ASL zpg, X
        case 0x16:
            break;
        //CLC impl
        case 0x18:
            break;
        //ORA abs, Y
        case 0x19:
            break;
        //ORA abs, X
        case 0x1D:
            break;
        //ASL abs, X
        case 0x1E:
            break;
        //JSR abs
        case 0x20:
            break;
        //AND X, ind
        case 0x21:
            break;
        //BIT zpg
        case 0x24:
            break;
        //AND zpg
        case 0x25:
            break;
        //ROL zpg
        case 0x26:
            break;
        //PLP impl
        case 0x28:
            break;
        //AND #
        case 0x29:
            break;
        //ROL A
        case 0x2A:
            break;
        //BIT abs
        case 0x2C:
            break;
        //AND abs
        case 0x2D:
            break;
        //ROL abs
        case 0x2E:
            break;
        //BMI rel
        case 0x30:
            break;
        //AND ind, Y
        case 0x31:
            break;
        //AND zpg, X
        case 0x35:
            break;
        //ROL zpg, X
        case 0x36:
            break;
        //SEC impl
        case 0x38:
            break;
        //AND abs, Y
        case 0x39:
            break;
        //AND abs, X
        case 0x3D:
            break;
        //ROL abs, X
        case 0x3E:
            break;
        //RTI impl
        case 0x40:
            break;
        //EOR X, ind
        case 0x41:
            break;
        //EOR zpg
        case 0x45:
            break;
        //LSR zpg
        case 0x46:
            break;
        //PHA impl
        case 0x48:
            break;
        //EOR #
        case 0x49:
            break;
        //LSR A
        case 0x4A:
            break;
        //JMP abs
        case 0x4C:
            break;
        //EOR abs
        case 0x4D:
            break;
        //LSR abs
        case 0x4E:
            break;
        //BVC rel
        case 0x50:
            break;
        //EOR ind, Y
        case 0x51:
            break;
        //EOR zpg, X
        case 0x55:
            break;
        //LSR zpg, X
        case 0x56:
            break;
        //CLI impl
        case 0x58:
            break;
        //EOR abs, Y
        case 0x59:
            break;
        //EOR abs, X
        case 0x5D:
            break;
        //LSR abs, X
        case 0x5E:
            break;
        //RTS impl
        case 0x60:
            break;
        //ADC X, ind
        case 0x61:
            break;
        //ADC zpg
        case 0x65:
            break;
        //ROR zpg
        case 0x66:
            break;
        //PLA impl
        case 0x68:
            break;
        //ADC #
        case 0x69:
            break;
        //ROR A
        case 0x6A:
            break;
        //JMP ind
        case 0x6C:
            break;
        //ADC abs
        case 0x6D:
            break;
        //ROR abs
        case 0x6E:
            break;
        //BVS rel
        case 0x70:
            break;
        //ADC ind, Y
        case 0x71:
            break;
        //ADC zpg, X
        case 0x75:
            break;
        //ROR zpg, X
        case 0x76:
            break;
        //SEI impl
        case 0x78:
            break;
        //ADC abs, Y
        case 0x79:
            break;
        //ADC abs, X
        case 0x7D:
            break;
        //ROR abs, X
        case 0x7E:
            break;
        //STA X, ind
        case 0x81:
            break;
        //STY zpg
        case 0x84:
            break;
        //STA zpg:
        case 0x85:
            break;
        //STX zpg
        case 0x86:
            break;
        //DEY impl
        case 0x88:
            break;
        //TXA impl
        case 0x8A:
            break;
        //STY abs
        case 0x8C:
            break;
        //STA abs
        case 0x8D:
            break;
        //STX abs
        case 0x8E:
            break;
        //BCC rel
        case 0x90:
            break;
        //STA ind, Y
        case 0x91:
            break;
        //STY zpg, X
        case 0x94:
            break;
        //STA zpg, X
        case 0x95:
            break;
        //STX zpg, Y
        case 0x96:
            break;
        //TYA impl
        case 0x98:
            break;
        //STA abs, Y
        case 0x99:
            break;
        //TXS impl
        case 0x9A:
            break;
        //STA abs, X
        case 0x9D:
            break;

    }
}