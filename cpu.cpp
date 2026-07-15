#include "cpu.h"
#include <unordered_map>
#include <stdexcept>
#include <string>

// Table of all official instruction sizes
constexpr uint8_t pcIncrement[256] = {
    // 0x00
    2,2,0,0,0,2,2,0,1,2,1,0,0,3,3,0,
    // 0x10
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0,
    // 0x20
    3,2,0,0,2,2,2,0,1,2,1,0,3,3,3,0,
    // 0x30
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0,
    // 0x40
    1,2,0,0,0,2,2,0,1,2,1,0,3,3,3,0,
    // 0x50
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0,
    // 0x60
    1,2,0,0,0,2,2,0,1,2,1,0,3,3,3,0,
    // 0x70
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0,
    // 0x80
    0,2,0,0,2,2,2,0,1,0,1,0,3,3,3,0,
    // 0x90
    2,2,0,0,2,2,2,0,1,3,1,0,0,3,0,0,
    // 0xA0
    2,2,2,0,2,2,2,0,1,2,1,0,3,3,3,0,
    // 0xB0
    2,2,0,0,2,2,2,0,1,3,1,0,3,3,3,0,
    // 0xC0
    2,2,0,0,2,2,2,0,1,2,1,0,3,3,3,0,
    // 0xD0
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0,
    // 0xE0
    2,2,0,0,2,2,2,0,1,2,1,0,3,3,3,0,
    // 0xF0
    2,2,0,0,0,2,2,0,1,3,0,0,0,3,3,0
};

//The main constructor
CPU::CPU(int memory_mapper) {

    //Initialize memory
    memory = std::unique_ptr<int8_t[]>(new int8_t[65536]);
    std::fill_n(this->memory.get(), 65536, 0);

    stackPointer = 0xFF;
    // This value isn't actually correct; the program counter is initialized the value of the reset vector at 0xFFFC and 0xFFFD
    programCounter = 0xFFFC;

    mem_map = memory_mapper;
}

//This should never be used in practice, but needs to exist so the compiler doesn't freak
CPU::CPU() {

    //Initialize memory
    memory = std::unique_ptr<int8_t[]>(new int8_t[65536]);
    std::fill_n(this->memory.get(), 65536, 0);

    stackPointer = 0xFD;
    programCounter = 0xFFFC;
    statusRegister = 36;
    xReg = 0;
    yReg = 0;
    accumulator = 0;

    mem_map = 0;
}

// Used to reset the state of memory and other variables when a new ROM is loaded
void CPU::manual_reset() {
    std::fill_n(this->memory.get(), 65536, 0);
    stackPointer = 0xFD;
    programCounter = 0xFFFC;
    mem_map = 0;
    statusRegister = 36;
    xReg = 0;
    yReg = 0;
    accumulator = 0;
}

//Decodes and executes instructions
void CPU::decode() {

    opcode = memory[programCounter];
    low_nibble = memory[programCounter + 1];
    high_nibble = memory[programCounter + 2];

    //Need to add an additional check to see if the opcode is valid - will do later
    programCounter += pcIncrement[opcode];

    //Switch over the low order nibble
    /*Operands indicate addressing mode. Note that 6502 is little endian so addresses are stored in memory least significant byte first
    The first byte of every instruction is the opcode
    Addressing modes are as follows:
    A - Accumulator, 1 byte instruction, instruction operates on accumulator
    abs - Absolute, 3 byte instruction where the address of the data is formed by the last 2 bytes
    abs, X/Y - Absolute Indexed, 3 byte instruction where the address is incremented by X/Y with carry
    # - Immediate, 2 byte instruction where the last byte is the operand (this operand does not represent an address)
    impl - Implied, 1 byte instruction where the operand is implied
    ind - Indirect, 3 byte instruction where the operand is an address pointing to another address
    X, ind - Indexed Indirect/X-indexed Indirect, 2 byte instruction where the operand is found by adding the byte with contents of X,
    &ing the result with 0xFF, and then using the data at that address as an address
    ind, Y - Indirect Indexed/Indirect Y-indexed, 2 byte instruction where the operand is found by using the byte as an address, adding Y
    to the address the byte points to, and using the result as an address (ex, $10, Y where Y = $5 and the data at $10 is $4013, uses the
    address $4018)
    rel - Relative, 2 byte instruction, used only for branch instructions, the byte is interpretted as signed and is the offset if the branch
    is taken
    zpg - Zero Page, 2 byte instruction where the byte is an address in the range 0x00 - 0xFF
    zpg, X/Y - Zero Page Indexed, 2 byte instruction where the byte is added with X/Y, &ed with 0xFF, and used as an address
    */
    switch (opcode) {

        //BRK impl
        case 0x00:
            BRK();
            break;
        //ORA X, ind
        case 0x01:
            ORA(Xind(low_nibble));
            break;
        //ORA zpg
        case 0x05:
            ORA(zpg(low_nibble));
            break;
        //ASL zpg
        case 0x06:
            ASL(zpgAdd(low_nibble));
            break;
        //PHP impl
        case 0x08:
            PHP();
            break;
        //ORA #
        case 0x09:
            ORA(low_nibble);
            break;
        //ASL A
        case 0x0A:
            ASLA();
            break;
        //ORA abs
        case 0x0D:
            ORA(abs(low_nibble, high_nibble));
            break;
        //ASL abs
        case 0x0E:
            ASL(absAdd(low_nibble, high_nibble));
            break;
        //BPL rel
        case 0x10:
            BPL(low_nibble);
            break;
        //ORA ind, Y
        case 0x11:
            ORA(indY(low_nibble));
            break;
        //ORA zpg, X
        case 0x15:
            ORA(zpgX(low_nibble));
            break;
        //ASL zpg, X
        case 0x16:
            ASL(zpgXAdd(low_nibble));
            break;
        //CLC impl
        case 0x18:
            CLC();
            break;
        //ORA abs, Y
        case 0x19:
            ORA(absY(low_nibble, high_nibble));
            break;
        //ORA abs, X
        case 0x1D:
            ORA(absX(low_nibble, high_nibble));
            break;
        //ASL abs, X
        case 0x1E:
            ASL(absXAdd(low_nibble, high_nibble));
            break;
        //JSR abs
        case 0x20:
            JSR(absAdd(low_nibble, high_nibble));
            break;
        //AND X, ind
        case 0x21:
            AND(Xind(low_nibble));
            break;
        //BIT zpg
        case 0x24:
            BIT(zpg(low_nibble));
            break;
        //AND zpg
        case 0x25:
            AND(zpg(low_nibble));
            break;
        //ROL zpg
        case 0x26:
            ROL(zpgAdd(low_nibble));
            break;
        //PLP impl
        case 0x28:
            PLP();
            break;
        //AND #
        case 0x29:
            AND(low_nibble);
            break;
        //ROL A
        case 0x2A:
            ROLA();
            break;
        //BIT abs
        case 0x2C:
            BIT(abs(low_nibble, high_nibble));
            break;
        //AND abs
        case 0x2D:
            AND(abs(low_nibble, high_nibble));
            break;
        //ROL abs
        case 0x2E:
            ROL(absAdd(low_nibble, high_nibble));
            break;
        //BMI rel
        case 0x30:
            BMI(low_nibble);
            break;
        //AND ind, Y
        case 0x31:
            AND(indY(low_nibble));
            break;
        //AND zpg, X
        case 0x35:
            AND(zpgX(low_nibble));
            break;
        //ROL zpg, X
        case 0x36:
            ROL(zpgXAdd(low_nibble));
            break;
        //SEC impl
        case 0x38:
            SEC();
            break;
        //AND abs, Y
        case 0x39:
            AND(absY(low_nibble, high_nibble));
            break;
        //AND abs, X
        case 0x3D:
            AND(absX(low_nibble, high_nibble));
            break;
        //ROL abs, X
        case 0x3E:
            ROL(absXAdd(low_nibble, high_nibble));
            break;
        //RTI impl
        case 0x40:
            RTI();
            break;
        //EOR X, ind
        case 0x41:
            EOR(Xind(low_nibble));
            break;
        //EOR zpg
        case 0x45:
            EOR(zpg(low_nibble));
            break;
        //LSR zpg
        case 0x46:
            LSR(zpgAdd(low_nibble));
            break;
        //PHA impl
        case 0x48:
            PHA();
            break;
        //EOR #
        case 0x49:
            EOR(low_nibble);
            break;
        //LSR A
        case 0x4A:
            LSRA();
            break;
        //JMP abs
        case 0x4C:
            JMP(absAdd(low_nibble, high_nibble));
            break;
        //EOR abs
        case 0x4D:
            EOR(abs(low_nibble, high_nibble));
            break;
        //LSR abs
        case 0x4E:
            LSR(absAdd(low_nibble, high_nibble));
            break;
        //BVC rel
        case 0x50:
            BVC(low_nibble);
            break;
        //EOR ind, Y
        case 0x51:
            EOR(indY(low_nibble));
            break;
        //EOR zpg, X
        case 0x55:
            EOR(zpgX(low_nibble));
            break;
        //LSR zpg, X
        case 0x56:
            LSR(zpgXAdd(low_nibble));
            break;
        //CLI impl
        case 0x58:
            CLI();
            break;
        //EOR abs, Y
        case 0x59:
            EOR(absY(low_nibble, high_nibble));
            break;
        //EOR abs, X
        case 0x5D:
            EOR(absX(low_nibble, high_nibble));
            break;
        //LSR abs, X
        case 0x5E:
            LSR(absXAdd(low_nibble, high_nibble));
            break;
        //RTS impl
        case 0x60:
            RTS();
            break;
        //ADC X, ind
        case 0x61:
            ADC(Xind(low_nibble));
            break;
        //ADC zpg
        case 0x65:
            ADC(zpg(low_nibble));
            break;
        //ROR zpg
        case 0x66:
            ROR(zpgAdd(low_nibble));
            break;
        //PLA impl
        case 0x68:
            PLA();
            break;
        //ADC #
        case 0x69:
            ADC(low_nibble);
            break;
        //ROR A
        case 0x6A:
            RORA();
            break;
        //JMP ind
        case 0x6C:
            JMP(indAdd(low_nibble, high_nibble));
            break;
        //ADC abs
        case 0x6D:
            ADC(abs(low_nibble, high_nibble));
            break;
        //ROR abs
        case 0x6E:
            ROR(absAdd(low_nibble, high_nibble));
            break;
        //BVS rel
        case 0x70:
            BVS(low_nibble);
            break;
        //ADC ind, Y
        case 0x71:
            ADC(indY(low_nibble));
            break;
        //ADC zpg, X
        case 0x75:
            ADC(zpgX(low_nibble));
            break;
        //ROR zpg, X
        case 0x76:
            ROR(zpgXAdd(low_nibble));
            break;
        //SEI impl
        case 0x78:
            SEI();
            break;
        //ADC abs, Y
        case 0x79:
            ADC(absY(low_nibble, high_nibble));
            break;
        //ADC abs, X
        case 0x7D:
            ADC(absX(low_nibble, high_nibble));
            break;
        //ROR abs, X
        case 0x7E:
            ROR(absXAdd(low_nibble, high_nibble));
            break;
        //STA X, ind
        case 0x81:
            write(XindAdd(low_nibble), accumulator);
            break;
        //STY zpg
        case 0x84:
            write(zpgAdd(low_nibble), yReg);
            break;
        //STA zpg:
        case 0x85:
            write(zpgAdd(low_nibble), accumulator);
            break;
        //STX zpg
        case 0x86:
            write(zpgAdd(low_nibble), xReg);
            break;
        //DEY impl
        case 0x88:
            DEY();
            break;
        //TXA impl
        case 0x8A:
            TXA();
            break;
        //STY abs
        case 0x8C:
            write(absAdd(low_nibble, high_nibble), yReg);
            break;
        //STA abs
        case 0x8D:
            write(absAdd(low_nibble, high_nibble), accumulator);
            break;
        //STX abs
        case 0x8E:
            write(absAdd(low_nibble, high_nibble), xReg);
            break;
        //BCC rel
        case 0x90:
            BCC(low_nibble);
            break;
        //STA ind, Y
        case 0x91:
            write(indYAdd(low_nibble), accumulator);
            break;
        //STY zpg, X
        case 0x94:
            write(zpgXAdd(low_nibble), yReg);
            break;
        //STA zpg, X
        case 0x95:
            write(zpgXAdd(low_nibble), accumulator);
            break;
        //STX zpg, Y
        case 0x96:
            write(zpgYAdd(low_nibble), yReg);
            break;
        //TYA impl
        case 0x98:
            TYA();
            break;
        //STA abs, Y
        case 0x99:
            write(absYAdd(low_nibble, high_nibble), accumulator);
            break;
        //TXS impl
        case 0x9A:
            TXS();
            break;
        //STA abs, X
        case 0x9D:
            write(absXAdd(low_nibble, high_nibble), accumulator);
            break;
        //LDY #
        case 0xA0:
            LDY(low_nibble);
            break;
        //LDA X, ind
        case 0xA1:
            LDA(Xind(low_nibble));
            break;
        //LDX #
        case 0xA2:
            LDX(low_nibble);
            break;
        //LDY zpg
        case 0xA4:
            LDY(zpg(low_nibble));
            break;
        //LDA zpg
        case 0xA5:
            LDA(zpg(low_nibble));
            break;
        //LDX zpg
        case 0xA6:
            LDX(zpg(low_nibble));
            break;
        //TAY impl
        case 0xA8:
            TAY();
            break;
        //LDA #
        case 0xA9:
            LDA(low_nibble);
            break;
        //TAX impl
        case 0xAA:
            TAX();
            break;
        //LDY abs
        case 0xAC:
            LDY(abs(low_nibble, high_nibble));
            break;
        //LDA abs
        case 0xAD:
            LDA(abs(low_nibble, high_nibble));
            break;
        //LDX abs
        case 0xAE:
            LDX(abs(low_nibble, high_nibble));
            break;
        //BCS rel
        case 0xB0:
            BCS(low_nibble);
            break;
        //LDA ind, Y
        case 0xB1:
            LDA(indY(low_nibble));
            break;
        //LDY zpg, X
        case 0xB4:
            LDY(zpgX(low_nibble));
            break;
        //LDA zpg, X
        case 0xB5:
            LDA(zpgX(low_nibble));
            break;
        //LDX zpg, Y
        case 0xB6:
            LDX(zpgY(low_nibble));
            break;
        //CLV impl
        case 0xB8:
            CLV();
            break;
        //LDA abs, Y
        case 0xB9:
            LDA(absY(low_nibble, high_nibble));
            break;
        //TSX impl
        case 0xBA:
            TSX();
            break;
        //LDY abs, X
        case 0xBC:
            LDY(absX(low_nibble, high_nibble));
            break;
        //LDA abs, X
        case 0xBD:
            LDA(absY(low_nibble, high_nibble));
            break;
        //LDX abs, Y
        case 0xBE:
            LDX(absY(low_nibble, high_nibble));
            break;
        //CPY #
        case 0xC0:
            CPY(low_nibble);
            break;
        //CMP X, ind
        case 0xC1:
            CMP(Xind(low_nibble));
            break;
        //CPY zpg
        case 0xC4:
            CPY(zpg(low_nibble));
            break;
        //CMP zpg
        case 0xC5:
            CMP(zpg(low_nibble));
            break;
        //DEC zpg
        case 0xC6:
            DEC(zpgAdd(low_nibble));
            break;
        //INY impl
        case 0xC8:
            INY();
            break;
        //CMP #
        case 0xC9:
            CMP(low_nibble);
            break;
        //DEX impl
        case 0xCA:
            DEX();
            break;
        //CPY abs
        case 0xCC:
            CPY(abs(low_nibble, high_nibble));
            break;
        //CMP abs
        case 0xCD:
            CMP(abs(low_nibble, high_nibble));
            break;
        //DEC abs
        case 0xCE:
            DEC(absAdd(low_nibble, high_nibble));
            break;
        //BNE rel
        case 0xD0:
            BNE(low_nibble);
            break;
        //CMP ind, Y
        case 0xD1:
            CMP(indY(low_nibble));
            break;
        //CMP zpg, X
        case 0xD5:
            CMP(zpgX(low_nibble));
            break;
        //DEC zpg, X
        case 0xD6:
            DEC(zpgXAdd(low_nibble));
            break;
        //CLD impl
        case 0xD8:
            CLD();
            break;
        //CMP abs, Y
        case 0xD9:
            CMP(absY(low_nibble, high_nibble));
            break;
        //CMP abs, X
        case 0xDD:
            CMP(absX(low_nibble, high_nibble));
            break;
        //DEC abs, X
        case 0xDE:
            DEC(absXAdd(low_nibble, high_nibble));
            break;
        //CPX #
        case 0xE0:
            CPX(low_nibble);
            break;
        //SBC X, ind
        case 0xE1:
            SBC(Xind(low_nibble));
            break;
        //CPX zpg
        case 0xE4:
            CPX(zpg(low_nibble));
            break;
        //SBC zpg
        case 0xE5:
            SBC(zpg(low_nibble));
            break;
        //INC zpg
        case 0xE6:
            INC(zpgAdd(low_nibble));
            break;
        //INX impl
        case 0xE8:
            INX();
            break;
        //SBC #
        case 0xE9:
            SBC(low_nibble);
            break;
        //NOP impl
        case 0xEA:
            break;
        //CPX abs
        case 0xEC:
            CPX(abs(low_nibble, high_nibble));
            break;
        //SBC abs
        case 0xED:
            SBC(abs(low_nibble, high_nibble));
            break;
        //INC abs
        case 0xEE:
            INC(absAdd(low_nibble, high_nibble));
            break;
        //BEQ rel
        case 0xF0:
            BEQ(low_nibble);
            break;
        //SBC ind, Y
        case 0xF1:
            SBC(indY(low_nibble));
            break;
        //SBC zpg, X
        case 0xF5:
            SBC(zpgX(low_nibble));
            break;
        //INC zpg, X
        case 0xF6:
            INC(zpgXAdd(low_nibble));
            break;
        //SED impl
        case 0xF8:
            SED();
            break;
        //SBC abs, Y
        case 0xF9:
            SBC(absY(low_nibble, high_nibble));
            break;
        //SBC abs, X
        case 0xFD:
            SBC(absX(low_nibble, high_nibble));
            break;
        //INC abs, X
        case 0xFE:
            INC(absXAdd(low_nibble, high_nibble));
            break;
        default:
            //Throw an exception - ADD LATER
            throw std::invalid_argument("Error: Invalid opcode " + std::to_string(opcode) + " decoded");
        
    }

}

//Addressing functions - these ones return the operands at the address
//Indirect Indexed
uint8_t CPU::Xind(uint8_t low) {

    uint16_t exp = (low + xReg) & 0xFF;
    return memory[((uint16_t) memory[exp + 1] << 8) | memory[exp]];

}

//Indexed Indirect
uint8_t CPU::indY(uint8_t low) {

    uint8_t high;
    low = memory[low];
    high = memory[low + 1];
    uint16_t exp = (((uint16_t) high << 8) | low) + yReg;
    return memory[exp];

}

//Absolute
uint8_t CPU::abs(uint8_t low, uint8_t high) { return memory[(((uint16_t) high) << 8) | low]; }

//Absolute, X Indexed
uint8_t CPU::absX(uint8_t low, uint8_t high) { return memory[((((uint16_t) high) << 8) | low) + xReg]; }

//Absolute, Y Indexed
uint8_t CPU::absY(uint8_t low, uint8_t high) { return memory[((((uint16_t) high) << 8) | low) + yReg]; }

//Zero Page
uint8_t CPU::zpg(uint8_t low) { return memory[low]; }

//Zero Page, X Indexed
uint8_t CPU::zpgX(uint8_t low) { return memory[(low + xReg) & 0xFF]; }

//Zero Page, Y Indexed
uint8_t CPU::zpgY(uint8_t low) { return memory[(low + yReg) & 0xFF]; }

//Addressing modes - these ones return the addresses themselves

//Indirect
uint16_t CPU::indAdd(uint8_t low, uint8_t high) {

    uint16_t exp = ((uint16_t) high << 8) | low;
    return ((uint16_t) memory[exp + 1] << 8) | memory[exp];

}

//Indirect Indexed
uint16_t CPU::XindAdd(uint8_t low) {

    uint16_t exp = (low + xReg) & 0xFF;
    return ((uint16_t) memory[exp + 1] << 8) | memory[exp];

}

//Indexed Indirect
uint16_t CPU::indYAdd(uint8_t low) {

    uint8_t high;
    low = memory[low];
    high = memory[low + 1];
    uint16_t exp = (((uint16_t) high << 8) | low) + yReg;
    return exp;

}

//Absolute
uint16_t CPU::absAdd(uint8_t low, uint8_t high) { return (((uint16_t) high) << 8) | low; }

//Absolute, X Indexed
uint16_t CPU::absXAdd(uint8_t low, uint8_t high) { return ((((uint16_t) high) << 8) | low) + xReg; }

//Absolute, Y Indexed
uint16_t CPU::absYAdd(uint8_t low, uint8_t high) { return ((((uint16_t) high) << 8) | low) + yReg; }

//Zero Page
uint16_t CPU::zpgAdd(uint8_t low) { return (uint16_t) low; }

//Zero Page, X Indexed
uint16_t CPU::zpgXAdd(uint8_t low) { return (low + xReg) & 0xFF; }

//Zero Page, Y Indexed
uint16_t CPU::zpgYAdd(uint8_t low) { return (low + yReg) & 0xFF; }

//Instructions

//Logic Instructions

//Bitwise OR operand with the accumulator and set the accumulator to the result
//Sets the sign and zero flags if applicable
void CPU::ORA(uint8_t operand) {

    accumulator = operand | accumulator;
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Bitwise AND operand with accumulator and set the accumulator to the result
//Sets the sign and zero flags if applicable
void CPU::AND(uint8_t operand) {

    accumulator = operand & accumulator;
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Performs bitwise Exclusive OR on the operand with the accumulator, storing the result in the accumulator
//Sets the sign and zero bits if applicable
void CPU::EOR(uint8_t operand) {

    accumulator = (accumulator & ~operand) | (~accumulator & operand);
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Bit test instruction
//Sets the sign and overflow flags equal to the 7th and 6th bits of the operand resp. (using a 0 based index)
//Sets the zero flag if the bitwise AND of the operand and the accumulator is 0 (the result of the AND is not stored anywhere)
void CPU::BIT(uint8_t operand) { 
    statusRegister = (operand & 0xC0) | ((accumulator & operand) == 0 ? 0x2 : 0) | (statusRegister & 0x3D); }

//Shift Instructions

//Arithmetic shift left the byte at the given address
//Shift in a 0
//Set the Zero and Sign bits if applicable
//Set the carry flag to the value of the shifted out bit
void CPU::ASL(uint16_t address) {

    uint8_t temp = memory[address] << 1;
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (memory[address] >> 7) | (statusRegister & 0x7C);
    int8_t shifted = memory[address] << 1;
    write(address, shifted);

}

//The ASL instruction but with accumulator addressing (that is, it operates directly on the accumulator)
void CPU::ASLA() {

    uint8_t temp = accumulator << 1;
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (accumulator >> 7) | (statusRegister & 0x7C);
    accumulator = accumulator << 1;

}

//Logical shift right the byte at the given address
//Shift in a 0
//Set the zero bit if applicable
//Set the carry bit to the bit shifted out
void CPU::LSR(uint16_t address) {

    statusRegister = ((memory[address] >> 1) == 0 ? 0x2 : 0) | (memory[address] & 0x1) | (statusRegister & 0x7C);
    int8_t shifted = memory[address] >> 1;
    write(address, shifted);
     
}

//LSR accumulator addressing
void CPU::LSRA() {

    statusRegister = ((accumulator >> 1) == 0 ? 0x2 : 0) | (accumulator & 0x1) | (statusRegister & 0x7C);
    accumulator = accumulator >> 1;

}

//Rotate left one bit instruction
//Performs a left shift but shifts in the carry bit instead of exclusively 0
void CPU::ROL(uint16_t address) {

    int8_t temp = (memory[address] << 1) | (statusRegister & 0x1);
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (memory[address] >> 7) | (statusRegister & 0x7C);
    write(address, temp);

}

//ROL accumulator addressing
void CPU::ROLA() {

    int8_t temp = (accumulator << 1) | (statusRegister & 0x1);
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (accumulator >> 7) | (statusRegister & 0x7C);
    accumulator = temp;

}

//Rotate right one bit instruction
//Performs a right shift but shifts in the carry bit instead of exclusively 0
void CPU::ROR(uint16_t address) {

    int8_t temp = (memory[address] >> 1) | ((statusRegister & 0x1) << 7);
    statusRegister = ((memory[address] >> 1) == 0 ? 0x2 : 0) | (memory[address] & 0x1) | (statusRegister & 0x7C);
    write(address, temp);

}

void CPU::RORA() {

    int8_t temp = (accumulator >> 1) | ((statusRegister & 0x1) << 7);
    statusRegister = ((accumulator >> 1) == 0 ? 0x2 : 0) | (accumulator & 0x1) | (statusRegister & 0x7C);
    accumulator = temp;

}

//Arithmetic Instructions

//Adds the operand and carry from previous instruction to the accumulator and store the result in the accumulator
//This instruction behaves differently in Decimal Mode but the NES does not implement Decimal Mode so we don't care
//Sets the zero, overflow, carry, and sign flags when applicable
void CPU::ADC(uint8_t operand) {

    int16_t temp = (uint8_t)accumulator + (statusRegister & 0x1) + operand;
    statusRegister = (((temp ^ accumulator) & (temp ^ operand) & 0x80) == 0x80 ? 0x40 : 0) | ((temp & 0xFF) == 0 ? 0x2 : 0) | (temp & 0x80) | ((temp & 0x100) == 0x100 ? 0x1 : 0) | (statusRegister & 0x3C);
    accumulator = temp & 0xFF;

}


//Subtracts the operand and the complement of the carry flag from the accumulator and stores the result in the accumulator
//Sets the zero, overflow, carry, and sign flags when applicable
//Carry flag is set if the result is greater than or equal to 0
void CPU::SBC(int8_t operand) {

    int16_t temp = accumulator - operand - (~statusRegister & 0x1);
    statusRegister = ((temp > 127 || temp < -128) ? 0x40 : 0) | (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (temp >= 0 ? 0x1 : 0) | (statusRegister & 0x3C);
    accumulator = temp & 0xFF;

}

//Compares the operand to the accumulator by subtracting the operand from the accumulator
//Sets the zero, carry, and sign flags
//The carry bit is set if the operand is greater than or equal to the accumulator
//The accumulator is unaffected
void CPU::CMP(int8_t operand) {

    int16_t temp = (int16_t) accumulator - (int16_t) operand;
    statusRegister = (temp & 0x80) | (temp >= 0 ? 0x1 : 0) | (temp == 0 ? 0x2 : 0) | (statusRegister & 0x7C);

}

//Similar to CMP but using the X register
void CPU::CPX(int8_t operand) {

    int16_t temp = (int16_t) xReg - (int16_t) operand;
    statusRegister = (temp & 0x80) | (temp >= 0 ? 0x1 : 0) | (temp == 0 ? 0x2 : 0) | (statusRegister & 0x7C);

}

//Similar to CMP but using the Y register
void CPU::CPY(int8_t operand) {

    int16_t temp = (int16_t) yReg - (int16_t) operand;
    statusRegister = (temp & 0x80) | (temp >= 0 ? 0x1 : 0) | (temp == 0 ? 0x2 : 0) | (statusRegister & 0x7C);

}

//Increment Instructions

//Decrements the value at the address by one
//Only affects zero and sign flags
void CPU::DEC(uint16_t address) {

    int8_t decremented = memory[address] - 1;
    write(address, decremented);
    statusRegister = (memory[address] & 0x80) | (memory[address] == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Same as DEC but affecting the X register
void CPU::DEX() {

    xReg--;
    statusRegister = (xReg & 0x80) | (xReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Same as DEC but affecting the Y register
void CPU::DEY() {

    yReg--;
    statusRegister = (yReg & 0x80) | (yReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Increments the value at the address by one
//Only affects zero and sign flags
void CPU::INC(uint16_t address) {

    int8_t incremented = memory[address] + 1;
    write(address, incremented);
    statusRegister = (memory[address] & 0x80) | (memory[address] == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Same as INC but affecting the X register
void CPU::INX() {

    xReg++;
    statusRegister = (xReg & 0x80) | (xReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Same as INC but affecting the Y register
void CPU::INY() {

    yReg++;
    statusRegister = (yReg & 0x80) | (yReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Flag Instructions

//Clears the carry flag
void CPU::CLC() { statusRegister = statusRegister & 0xFE; }

//Clears the decimal mode flag. Don't think I actually need to implement this for NES emulation but who cares
void CPU::CLD() { statusRegister = statusRegister & 0xF7; }

//Clears the interrupt disable flag
void CPU::CLI() { statusRegister = statusRegister & 0xFB; }

//Clears the overflow flag
void CPU::CLV() { statusRegister = statusRegister & 0xBF; }

//Sets the carry flag
void CPU::SEC() { statusRegister = statusRegister | 0x1; }

//Sets the decimal mode flag
void CPU::SED() { statusRegister = statusRegister | 0x8; }

//Sets the interrupt disable flag
void CPU::SEI() { statusRegister = statusRegister | 0x4; }

//Branch Instructions

//Branch on carry clear
//If the carry bit is 0, branch to programCounter + operand
void CPU::BCC(int8_t operand) { programCounter += (statusRegister & 0x1) == 0 ? operand : 0; }

//Branch on carry set
void CPU::BCS(int8_t operand) { programCounter += (statusRegister & 0x1) == 0x1 ? operand : 0; }

//Branch on zero set (aka branch on equal)
//Branch if the zero bit is set
void CPU::BEQ(int8_t operand) { programCounter += (statusRegister & 0x2) == 0x2 ? operand : 0; }

//Branch on result minus
//Branch if the sign bit is set
void CPU::BMI(int8_t operand) { programCounter += (statusRegister & 0x80) == 0x80 ? operand : 0; }

//Branch on zero clear (aka branch on not equal)
void CPU::BNE(int8_t operand) { programCounter += (statusRegister & 0x2) == 0 ? operand : 0; }

//Branch on result plus
//Branch if the sign bit is cleared
void CPU::BPL(int8_t operand) { programCounter += (statusRegister & 0x80) == 0 ? operand : 0; }

//Branch on overflow clear
//Branch if the overflow bit is set to 0
void CPU::BVC(int8_t operand) { programCounter += (statusRegister & 0x40) == 0 ? operand : 0; }

//Branch on overflow set
void CPU::BVS(int8_t operand) { programCounter += (statusRegister & 0x40) == 0x40 ? operand : 0; }

//Load and Store Instructions

//Load Accumulator with the operand
//Affects sign and zero flags
void CPU::LDA(uint8_t operand) {
    accumulator = operand;
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Load X register with the operand
void CPU::LDX(uint8_t operand) {

    xReg = operand;
    statusRegister = (xReg & 0x80) | (xReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Load Y register with the operand
void CPU::LDY(uint8_t operand) {

    yReg = operand;
    statusRegister = (yReg & 0x80) | (yReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

// Replaced by write() and mapper functions
// //Store the accumulator at the address
// void CPU::STA(uint16_t address) { memory[address] = accumulator; }

// //Store the X register at the address
// void CPU::STX(uint16_t address) { memory[address] = xReg; }

// //Store the Y register at the address
// void CPU::STY(uint16_t address) { memory[address] = yReg; }

//Transfer Instructions

//Copies the value of accumulator into the X register
//Affects zero and sign flags
void CPU::TAX() {

    xReg = accumulator;
    statusRegister = (xReg & 0x80) | (xReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Copies the value of accumulator into the Y register
void CPU::TAY() {

    yReg = accumulator;
    statusRegister = (yReg & 0x80) | (yReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Copies the value of the stack pointer into the X register
void CPU::TSX() {

    xReg = stackPointer;
    statusRegister = (xReg & 0x80) | (xReg == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Copies the X register into the accumulator
void CPU::TXA() {

    accumulator = xReg;
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Copies the X register into the stack pointer
void CPU::TXS() {

    stackPointer = xReg;
    statusRegister = (stackPointer & 0x80) | (stackPointer == 0 ? 0x2 : 0) | (stackPointer & 0x7D);

}

//Copies the Y register into the accumulator
void CPU::TYA() {

    accumulator = yReg;
    statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Stack Instructions

//Push accumulator onto the stack
void CPU::PHA() {

    write(0x100 + stackPointer, (int8_t&)accumulator);
    stackPointer--;

}

//Push the status register onto the stack
//Affects break flag and the fifth unused bit
void CPU::PHP() {

    uint8_t val = statusRegister | 48;
    write(0x100 + stackPointer, (int8_t&)val);
    stackPointer--;

}

//Pull the contents of the stack and put in the accumulator
//Affects sign and zero flags
void CPU::PLA() {

    stackPointer++;
    accumulator = memory[0x100 + stackPointer];
    statusRegister = statusRegister = (accumulator & 0x80) | (accumulator == 0 ? 0x2 : 0) | (statusRegister & 0x7D);

}

//Pull the contents of the stack and put in the status register
// Ignore the break flag (e.g. set it to 0)
void CPU::PLP() {

    stackPointer++;
    statusRegister = memory[0x100 + stackPointer] & 0xEF | 0x20;

}

// Helper function to push the program counter to the stack
void CPU::pushPC() {
    int8_t high = programCounter >> 8;
    int8_t low = programCounter & 15;
    write(0x100 + stackPointer, low);
    write(0x100 + stackPointer - 1, high);
    stackPointer -= 2;
}

//Control Instructions

//Sets the program counter to the address
void CPU::JMP(uint16_t address) { programCounter = address; }

//Jump to the subroutine at the address
//Push the program counter onto the stack
void CPU::JSR(uint16_t address) {

    int8_t low = programCounter;
    int8_t high = programCounter >> 8;
    write(0x100 + stackPointer, low);
    write(0x100 + stackPointer - 1, high);
    stackPointer -= 2;
    programCounter = address;

}

//Return from subroutine
void CPU::RTS() {

    stackPointer += 2;
    programCounter = (((uint16_t) memory[0x100 + stackPointer - 1]) << 8) | memory[0x100 + stackPointer];

}

// Break Instruction -- triggers an IRQ (aka maskable interrupt)
// Can be ignored if the interrupt disable flag is set
// Though technically a 1 byte instruction, BRK is treated like a 2 byte instruction, and hence it is treated as such in the pcIncrement table
void CPU::BRK() {
    if (statusRegister & 4 == 4) {
        return;
    }

    //Store program counter on the stack
    pushPC();

    //Store the status register
    PHP();

    //Set interrupt disable
    SEI();

    //Load address of interrupt handling routine from FFFE and FFFF
    int8_t high = memory[0xFFFF];
    int8_t low = memory[0xFFFE];

    programCounter = absAdd(low, high);
}

// Return from interrupt - pulls the status register and program counter from the stack
void CPU::RTI() {
    stackPointer++;
    statusRegister = memory[stackPointer + 0x100];
    stackPointer++;
    int8_t low = memory[stackPointer + 0x100];
    stackPointer++;
    int8_t high = memory[stackPointer + 0x100];
    programCounter = absAdd(low, high);
}

//Mapper Write function implementations

// Base write function from which the mapper writes are called
void CPU::write(uint16_t address, int8_t& val) {
    // Mirror in correct location
    // Three mirrors in this range
    if (address <= 0x1FFF) {
        uint16_t mirror = (address + 0x800) % 0x2000;
        memory[mirror] = val;
        mirror = (mirror + 0x800) % 0x2000;
        memory[mirror] = val;
        mirror = (mirror + 0x800) % 0x2000;
        memory[mirror] = val;

        memory[address] = val;
    }
    // Mirrored every 8 bytes
    else if (address <= 0x401F) {
        uint16_t mirror = address;
        for (int i = 0; i < 1024; i++) {
            mirror = (mirror + 0x8) % 0x2000 + 0x2000;
            memory[mirror] = val;
        }

        memory[address] = val;
    }
    // Don't think anything needs to be done here tbh; no mirroring
    else if (address <= 0x7FFF) {
        memory[address] = val;
    }
    // Writing to ROM. Do memory mapper stuff
    else {
        (this->*writes[mem_map])(address, val);
    }
}

// Default memory mapper write. Does nothing
void CPU::default_write(uint16_t address, int8_t& val) {
    return;
}

// Interrupt handling functions

// Reset interrupt which is triggered on system startup or whenever the reset button is pressed
void CPU::interrupt_reset() {
    // Set interrupt disable
    //this->SEI();
    // Load address of interrupt handling routine into program counter - in this case the address is stored at $FFFC and $FFFD
    int8_t low = memory[0xFFFC];
    int8_t high = memory[0xFFFD];
    int16_t address = absAdd(low, high);

    programCounter = address;

    // Continue execution as normal;
}

// This is essentially the same as BRK, however, IRQs that aren't raised by BRK don't set the B flag; this behavior is rarely used,
// but worth acknowledging
void CPU::interrupt_IRQ_generic() {
    if ((statusRegister & 4) == 4) {
        return;
    }

    //Store program counter on the stack
    pushPC();

    //Store the status register
    statusRegister |= 32;
    write(0x100 + stackPointer, (int8_t&)statusRegister);
    stackPointer--;

    //Set interrupt disable
    SEI();

    //Load address of interrupt handling routine from FFFE and FFFF
    int8_t high = memory[0xFFFF];
    int8_t low = memory[0xFFFE];

    programCounter = absAdd(low, high);
}

// Non-maskable Interrupt - these are generated by the PPU when V-Blank occurs at the end of each frame
// Not affected by the interrupt disable flag, however, can be disabled by PPU register 1 (will have to deal with this when designing PPU probably)
void CPU::interrupt_NMI() {
    //Store program counter on the stack
    pushPC();

    //Store the status register
    statusRegister |= 32;
    write(0x100 + stackPointer, (int8_t&)statusRegister);
    stackPointer--;

    //Set interrupt disable
    SEI();

    //Load address of interrupt handling routine from FFFA and FFFB
    int8_t high = memory[0xFFFB];
    int8_t low = memory[0xFFFA];

    programCounter = absAdd(low, high);
}

// Setters/Getters

void CPU::set_PC(uint16_t pc) { programCounter = pc; }

uint16_t CPU::get_PC() const { return programCounter; }

void CPU::set_accumulator(int8_t acc) { accumulator = acc; }

int8_t CPU::get_accumulator() const { return accumulator; }

void CPU::set_status(uint8_t status) { statusRegister = status; }

uint8_t CPU::get_status() const { return statusRegister; }

void CPU::set_x(int8_t x) { xReg = x; }

int8_t CPU::get_x() const { return xReg; }

void CPU::set_y(int8_t y) { yReg = y; }

int8_t CPU::get_y() const { return yReg; }

void CPU::set_stack(uint8_t stack) { stackPointer = stack; }

uint8_t CPU::get_stack() const { return stackPointer; }

void CPU::set_memMap(int memory_map) { mem_map = memory_map; }

int CPU::get_memMap() const { return mem_map; }

void CPU::set_opcode(uint8_t op) { opcode = op; }

uint8_t CPU::get_opcode() const { return opcode; }

void CPU::set_high_nibble(uint8_t nibble) { high_nibble = nibble; }

uint8_t CPU::get_high_nibble() const { return high_nibble; }

void CPU::set_low_nibble(uint8_t nibble) { low_nibble = nibble; }

uint8_t CPU::get_low_nibble() const { return low_nibble; }

uint8_t CPU::get_next_low_nibble() const { return memory[programCounter + 1]; }

uint8_t CPU::get_next_high_nibble() const { return memory[programCounter + 2]; }

uint8_t CPU::get_next_opcode() const { return memory[programCounter]; }