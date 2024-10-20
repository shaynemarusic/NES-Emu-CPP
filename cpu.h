#pragma once
#include <cstdint>
#include <fstream>

//This class represents the CPU (duh). The NES used the Ricoh 2AO3 which was a slightly modified MOS 6502
class CPU {

    private:
        //Registers
        //Stores results of arithmetic and logic operations
        int8_t accumulator;
        /*Only 7 bits of this register are actually used; each bit corresponds to a different flag
        The structure of the register: NV-BDIZC
        N: sign flag (0 is pos, 1 is neg)
        V: overflow flag set when an overflow is detected
        -: unused bit
        B: break flag that's set when a break instruction is executed
        D: decimal mode flag, unused in the NES
        I: interrupt disbale flag, interrupts are ignored while this flag is set
        Z: zero flag, set if the result of the last instruction was 0
        C: carry flag, set if the last instruction resulted in an overflow or underflow
        */
        uint8_t statusRegister;
        uint16_t programCounter;
        //The stack is stored in 0x0100-0x01FF. This register acts as an offset from 0x0100
        //Pointer is decremented when data is pushed onto the stack and incremented when data is taken off
        //No detection of stack overflow
        uint8_t stackPointer;
        //Used for addressing
        int8_t xReg;
        //Like the X register but cannot affect the stack pointer
        int8_t yReg;
        //This may or may not be necessary
        std::fstream romFile;

        //Memory
        /*The 6502 had a 64KB memory
        Layout of memory:
            RAM - 0x00000 to 0x01FFF. 0x00000 to 0x000FF is Zero Page, 0x00100 to 0x001FF is the Stack, 0x00200 to 0x007FF is RAM, and 
            0x00800 to 0x01FFF mirrors the data beneath it three times
            I/O registers - 0x02000 to 0x0401F. 0x02000 to 0x02007 are memory mapped I/O registers. I/O registers are mirrored every 8 bytes
            from 0x02008 to 0x03FFF
            Expansion ROM - 0x04020 to 0x05FFF
            SRAM - 0x06000 to 0x07FFF. This is Save RAM used to access save data in cartridges
            PRG-ROM - 0x08000 to 0x0FFFF. This is program ROM. It is divided into two banks: 0x08000 to 0x0BFFF and 0x0C000 to 0x0FFFF.
            How these banks are utilized is dependent on the size of the game
        */
        std::unique_ptr<int8_t[]> memory;

        //Instructions

        //Logic Instructions 
        void ORA(uint8_t operand);
        void AND(uint8_t operand);
        void EOR(uint8_t operand);
        void BIT(uint8_t operand);

        //Shift Instructions
        void ASL(uint16_t address);
        void ASLA();
        void LSR(uint16_t address);
        void LSRA();
        void ROL(uint16_t address);
        void ROLA();
        void ROR(uint16_t address);
        void RORA();

        //Arithmetic Instructions
        void ADC(int8_t operand);
        void SBC(int8_t operand);
        void CMP(int8_t operand);
        void CPX(int8_t operand);
        void CPY(int8_t operand);

        //Increment Instructions
        void DEC(uint16_t address);
        void DEX();
        void DEY();
        void INC(uint16_t address);
        void INX();
        void INY();

        //Flag Instructions
        void CLC();
        void CLD();
        void CLI();
        void CLV();
        void SEC();
        void SED();
        void SEI();

        //Branch Instructions
        void BCC(int8_t operand);
        void BCS(int8_t operand);
        void BEQ(int8_t operand);
        void BMI(int8_t operand);
        void BNE(int8_t operand);
        void BPL(int8_t operand);
        void BVC(int8_t operand);
        void BVS(int8_t operand);

        //Load and Store Instructions
        void LDA(uint8_t operand);
        void LDX(uint8_t operand);
        void LDY(uint8_t operand);
        void STA(uint16_t address);
        void STX(uint16_t address);
        void STY(uint16_t address);

        //Transfer Instructions
        void TAX();
        void TAY();
        void TSX();
        void TXA();
        void TXS();
        void TYA();

        //Stack Instructions
        void PHA();
        void PHP();
        void PLA();
        void PLP();

        //Control Instructions
        void JMP(uint16_t address);
        void JSR(uint16_t address);
        void RTS();

        //Addressing modes
        uint8_t Xind(uint8_t low);
        uint8_t indY(uint8_t low);
        uint8_t abs(uint8_t low, uint8_t high);
        uint8_t absX(uint8_t low, uint8_t high);
        uint8_t absY(uint8_t low, uint8_t high);
        uint8_t zpg(uint8_t low);
        uint8_t zpgX(uint8_t low);
        uint8_t zpgY(uint8_t low);

        uint16_t XindAdd(uint8_t byte);
        uint16_t indYAdd(uint8_t byte);
        uint16_t absAdd(uint8_t low, uint8_t high);
        uint16_t absXAdd(uint8_t low, uint8_t high);
        uint16_t absYAdd(uint8_t low, uint8_t high);
        uint16_t zpgAdd(uint8_t low);
        uint16_t zpgXAdd(uint8_t low);
        uint16_t zpgYAdd(uint8_t low);
        uint16_t indAdd(uint8_t low, uint8_t high);

    public:
        CPU(const char * filename);
        //Destructor may or may not be needed. Depends on implementation details yet to be ironed out
        //~CPU();
        void decode();

};