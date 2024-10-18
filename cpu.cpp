#include "cpu.h"
#include <unordered_map>

std::unordered_map<uint8_t, uint8_t> pcIncrement = {
    {0x01, 2}, {0x05, 2}, {0x09, 2}, {0x0D, 3}, {0x11, 2}, {0x15, 2}, {0x19, 3}, {0x1D, 3}
};

//The main constructor
CPU::CPU(const char * filename) {

    //Initialize memory
    memory = std::unique_ptr<int8_t[]>(new int8_t[65536]);
    std::fill_n(this->memory.get(), 65536, 0);

    stackPointer = 0xFF;
    programCounter = 0xFFFC;

    //Open the rom file - worry about the details of this later
    //this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

}

//Decodes and executes instructions
void CPU::decode() {

    uint8_t opcode, high, low;
    opcode = memory[programCounter];
    low = memory[programCounter + 1];
    high = memory[programCounter + 2];
    //programCounter++;

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
            break;
        //ORA X, ind
        case 0x01:
            ORA(Xind(low));
            break;
        //ORA zpg
        case 0x05:
            ORA(zpg(low));
            break;
        //ASL zpg
        case 0x06:
            ASL(zpgAdd(low));
            break;
        //PHP impl
        case 0x08:
            break;
        //ORA #
        case 0x09:
            ORA(low);
            break;
        //ASL A
        case 0x0A:
            ASLA();
            break;
        //ORA abs
        case 0x0D:
            ORA(abs(low, high));
            break;
        //ASL abs
        case 0x0E:
            ASL(absAdd(low, high));
            break;
        //BPL rel
        case 0x10:
            break;
        //ORA ind, Y
        case 0x11:
            ORA(indY(low));
            break;
        //ORA zpg, X
        case 0x15:
            ORA(zpgX(low));
            break;
        //ASL zpg, X
        case 0x16:
            ASL(zpgXAdd(low));
            break;
        //CLC impl
        case 0x18:
            break;
        //ORA abs, Y
        case 0x19:
            ORA(absY(low, high));
            break;
        //ORA abs, X
        case 0x1D:
            ORA(absX(low, high));
            break;
        //ASL abs, X
        case 0x1E:
            ASL(absXAdd(low, high));
            break;
        //JSR abs
        case 0x20:
            break;
        //AND X, ind
        case 0x21:
            AND(Xind(low));
            break;
        //BIT zpg
        case 0x24:
            BIT(zpg(low));
            break;
        //AND zpg
        case 0x25:
            AND(zpg(low));
            break;
        //ROL zpg
        case 0x26:
            ROL(zpgAdd(low));
            break;
        //PLP impl
        case 0x28:
            break;
        //AND #
        case 0x29:
            AND(low);
            break;
        //ROL A
        case 0x2A:
            ROLA();
            break;
        //BIT abs
        case 0x2C:
            BIT(abs(low, high));
            break;
        //AND abs
        case 0x2D:
            AND(abs(low, high));
            break;
        //ROL abs
        case 0x2E:
            ROL(absAdd(low, high));
            break;
        //BMI rel
        case 0x30:
            break;
        //AND ind, Y
        case 0x31:
            AND(indY(low));
            break;
        //AND zpg, X
        case 0x35:
            AND(zpgX(low));
            break;
        //ROL zpg, X
        case 0x36:
            ROL(zpgXAdd(low));
            break;
        //SEC impl
        case 0x38:
            break;
        //AND abs, Y
        case 0x39:
            AND(absY(low, high));
            break;
        //AND abs, X
        case 0x3D:
            AND(absX(low, high));
            break;
        //ROL abs, X
        case 0x3E:
            ROL(absXAdd(low, high));
            break;
        //RTI impl
        case 0x40:
            break;
        //EOR X, ind
        case 0x41:
            EOR(Xind(low));
            break;
        //EOR zpg
        case 0x45:
            EOR(zpg(low));
            break;
        //LSR zpg
        case 0x46:
            LSR(zpgAdd(low));
            break;
        //PHA impl
        case 0x48:
            break;
        //EOR #
        case 0x49:
            EOR(low);
            break;
        //LSR A
        case 0x4A:
            LSRA();
            break;
        //JMP abs
        case 0x4C:
            break;
        //EOR abs
        case 0x4D:
            EOR(abs(low, high));
            break;
        //LSR abs
        case 0x4E:
            LSR(absAdd(low, high));
            break;
        //BVC rel
        case 0x50:
            break;
        //EOR ind, Y
        case 0x51:
            EOR(indY(low));
            break;
        //EOR zpg, X
        case 0x55:
            EOR(zpgX(low));
            break;
        //LSR zpg, X
        case 0x56:
            LSR(zpgXAdd(low));
            break;
        //CLI impl
        case 0x58:
            break;
        //EOR abs, Y
        case 0x59:
            EOR(absY(low, high));
            break;
        //EOR abs, X
        case 0x5D:
            EOR(absX(low, high));
            break;
        //LSR abs, X
        case 0x5E:
            LSR(absXAdd(low, high));
            break;
        //RTS impl
        case 0x60:
            break;
        //ADC X, ind
        case 0x61:
            ADC(Xind(low));
            break;
        //ADC zpg
        case 0x65:
            ADC(zpg(low));
            break;
        //ROR zpg
        case 0x66:
            ROR(zpgAdd(low));
            break;
        //PLA impl
        case 0x68:
            break;
        //ADC #
        case 0x69:
            ADC(low);
            break;
        //ROR A
        case 0x6A:
            RORA();
            break;
        //JMP ind
        case 0x6C:
            break;
        //ADC abs
        case 0x6D:
            ADC(abs(low, high));
            break;
        //ROR abs
        case 0x6E:
            ROR(absAdd(low, high));
            break;
        //BVS rel
        case 0x70:
            break;
        //ADC ind, Y
        case 0x71:
            ADC(indY(low));
            break;
        //ADC zpg, X
        case 0x75:
            ADC(zpgX(low));
            break;
        //ROR zpg, X
        case 0x76:
            ROR(zpgXAdd(low));
            break;
        //SEI impl
        case 0x78:
            break;
        //ADC abs, Y
        case 0x79:
            ADC(absY(low, high));
            break;
        //ADC abs, X
        case 0x7D:
            ADC(absX(low, high));
            break;
        //ROR abs, X
        case 0x7E:
            ROR(absXAdd(low, high));
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
            DEY();
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
        //LDY #
        case 0xA0:
            break;
        //LDA X, ind
        case 0xA1:
            break;
        //LDX #
        case 0xA2:
            break;
        //LDY zpg
        case 0xA4:
            break;
        //LDA zpg
        case 0xA5:
            break;
        //LDX zpg
        case 0xA6:
            break;
        //TAY impl
        case 0xA8:
            break;
        //LDA #
        case 0xA9:
            break;
        //TAX impl
        case 0xAA:
            break;
        //LDY abs
        case 0xAC:
            break;
        //LDA abs
        case 0xAD:
            break;
        //LDX abs
        case 0xAE:
            break;
        //BCS rel
        case 0xB0:
            break;
        //LDA ind, Y
        case 0xB1:
            break;
        //LDY zpg, X
        case 0xB4:
            break;
        //LDA zpg, X
        case 0xB5:
            break;
        //LDX zpg, Y
        case 0xB6:
            break;
        //CLV impl
        case 0xB8:
            break;
        //LDA abs, Y
        case 0xB9:
            break;
        //TSX impl
        case 0xBA:
            break;
        //LDY abs, X
        case 0xBC:
            break;
        //LDA abs, X
        case 0xBD:
            break;
        //LDX abs, Y
        case 0xBE:
            break;
        //CPY #
        case 0xC0:
            CPY(low);
            break;
        //CMP X, ind
        case 0xC1:
            CMP(Xind(low));
            break;
        //CPY zpg
        case 0xC4:
            CPY(zpg(low));
            break;
        //CMP zpg
        case 0xC5:
            CMP(zpg(low));
            break;
        //DEC zpg
        case 0xC6:
            DEC(zpgAdd(low));
            break;
        //INY impl
        case 0xC8:
            INY();
            break;
        //CMP #
        case 0xC9:
            CMP(low);
            break;
        //DEX impl
        case 0xCA:
            DEX();
            break;
        //CPY abs
        case 0xCC:
            CPY(abs(low, high));
            break;
        //CMP abs
        case 0xCD:
            CMP(abs(low, high));
            break;
        //DEC abs
        case 0xCE:
            DEC(absAdd(low, high));
            break;
        //BNE rel
        case 0xD0:
            break;
        //CMP ind, Y
        case 0xD1:
            CMP(indY(low));
            break;
        //CMP zpg, X
        case 0xD5:
            CMP(zpgX(low));
            break;
        //DEC zpg, X
        case 0xD6:
            DEC(zpgXAdd(low));
            break;
        //CLD impl
        case 0xD8:
            break;
        //CMP abs, Y
        case 0xD9:
            CMP(absY(low, high));
            break;
        //CMP abs, X
        case 0xDD:
            CMP(absX(low, high));
            break;
        //DEC abs, X
        case 0xDE:
            DEC(absXAdd(low, high));
            break;
        //CPX #
        case 0xE0:
            CPX(low);
            break;
        //SBC X, ind
        case 0xE1:
            SBC(Xind(low));
            break;
        //CPX zpg
        case 0xE4:
            CPX(zpg(low));
            break;
        //SBC zpg
        case 0xE5:
            SBC(zpg(low));
            break;
        //INC zpg
        case 0xE6:
            INC(zpgAdd(low));
            break;
        //INX impl
        case 0xE8:
            INX();
            break;
        //SBC #
        case 0xE9:
            SBC(low);
            break;
        //NOP impl
        case 0xEA:
            break;
        //CPX abs
        case 0xEC:
            CPX(abs(low, high));
            break;
        //SBC abs
        case 0xED:
            SBC(abs(low, high));
            break;
        //INC abs
        case 0xEE:
            INC(absAdd(low, high));
            break;
        //BEQ rel
        case 0xF0:
            break;
        //SBC ind, Y
        case 0xF1:
            SBC(indY(low));
            break;
        //SBC zpg, X
        case 0xF5:
            SBC(zpgX(low));
            break;
        //INC zpg, X
        case 0xF6:
            INC(zpgXAdd(low));
            break;
        //SED impl
        case 0xF8:
            break;
        //SBC abs, Y
        case 0xF9:
            SBC(absY(low, high));
            break;
        //SBC abs, X
        case 0xFD:
            SBC(absX(low, high));
            break;
        //INC abs, X
        case 0xFE:
            INC(absXAdd(low, high));
            break;
        default:
            //Throw an exception - ADD LATER
        
        programCounter += pcIncrement[opcode];

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

//Addressing modes - these ones return the addresses themselves

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

    statusRegister = (operand & 0xC0) | (accumulator & operand == 0 ? 0x2 : 0) | (statusRegister & 0x3D);

}

//Shift Instructions

//Arithmetic shift left the byte at the given address
//Shift in a 0
//Set the Zero and Sign bits if applicable
//Set the carry flag to the value of the shifted out bit
void CPU::ASL(uint16_t address) {

    uint8_t temp = memory[address] << 1;
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (memory[address] >> 7) | (statusRegister & 0x7C);
    memory[address] = memory[address] << 1;

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
    memory[address] = memory[address] >> 1;
     
}

//LSR accumulator addressing
void CPU::LSRA() {

    statusRegister = ((accumulator >> 1) == 0 ? 0x2 : 0) | (accumulator & 0x1) | (statusRegister & 0x7C);
    accumulator = accumulator >> 1;

}

//Rotate left one bit instruction
//Performs a left shift but shifts in the carry bit instead of exclusively 0
void CPU::ROL(uint16_t address) {

    uint8_t temp = (memory[address] << 1) | (statusRegister & 0x1);
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (memory[address] >> 7) | (statusRegister & 0x7C);
    memory[address] = temp;

}

//ROL accumulator addressing
void CPU::ROLA() {

    uint8_t temp = (accumulator << 1) | (statusRegister & 0x1);
    statusRegister = (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (accumulator >> 7) | (statusRegister & 0x7C);
    accumulator = temp;

}

//Rotate right one bit instruction
//Performs a right shift but shifts in the carry bit instead of exclusively 0
void CPU::ROR(uint16_t address) {

    uint8_t temp = (memory[address] >> 1) | ((statusRegister & 0x1) << 7);
    statusRegister = ((memory[address] >> 1) == 0 ? 0x2 : 0) | (memory[address] & 0x1) | (statusRegister & 0x7C);
    memory[address] = temp;

}

void CPU::RORA() {

    uint8_t temp = (accumulator >> 1) | ((statusRegister & 0x1) << 7);
    statusRegister = ((accumulator >> 1) == 0 ? 0x2 : 0) | (accumulator & 0x1) | (statusRegister & 0x7C);
    accumulator = temp;

}

//Arithmetic Instructions

//Adds the operand and carry from previous instruction to the accumulator and store the result in the accumulator
//This instruction behaves differently in Decimal Mode but the NES does not implement Decimal Mode so we don't care
//Sets the zero, overflow, carry, and sign flags when applicable
void CPU::ADC(int8_t operand) {

    int16_t temp = accumulator + (statusRegister & 0x1) + operand;
    statusRegister = ((temp > 127 || temp < -128) ? 0x40 : 0) | (temp == 0 ? 0x2 : 0) | (temp & 0x80) | (temp > 255 ? 0x1 : 0) | (statusRegister & 0x3C);
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

    memory[address]--;
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

    memory[address]++;
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