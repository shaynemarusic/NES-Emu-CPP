#include "emu.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

std::string hex(uint32_t value, int width);

Emulator::Emulator() {
    running = false;

    //Initialize PPU
    //ppu = PPU()

    //Initialize APU
    //apu = APU()

    //Initialize CPU
    cpu = CPU();
}

// Emulator::Emulator(const char * filename) {
//     running = false;
//     //Initialize PPU

//     //Initialize APU

//     //Load ROM
//     //Ripped from my CHIP 8 emulator
//     this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

//     if (romFile.is_open()) {

//         unsigned int i = 0;
//         int size = romFile.tellg();
//         romFile.seekg(0, std::ios::beg);

//         //Parse file header
//         //Verify file is an ines file (first 4 bytes)
//         //Will need to throw an error gracefully
//         char byte;

//         romFile.read(&byte, 1);

//         if (byte != 'N') {

//         }

//         romFile.read(&byte, 1);
        
//         if (byte != 'E') {

//         }

//         romFile.read(&byte, 1);

//         if (byte != 'S') {

//         }

//         romFile.read(&byte, 1);

//         //End-of-file character
//         if (byte != 0x1A) {

//         }

//         //Get the number of PRG-ROM banks
//         romFile.read(&byte, 1);
//         prg_rom = byte;

//         //Get the number of CHR-ROM banks
//         romFile.read(&byte, 1);
//         chr_rom = byte;

//         //Parse header flags
//         //This will be done later in more depth
//         romFile.read(&byte, 1);
//         flag6 = byte;

//         romFile.read(&byte, 1);
//         flag7 = byte;

//         //flags 8-10 are rarely used, bytes 11-15 are unused padding, so just jump to byte 16

//         //Load everything into memory based on memory mapper
//         //The mapper we use is determined by an 8 bit number whose lower nibble is the upper nibble of flag6 and whose upper nibble is the
//         //upper nibble of flag7
//         //For now, assume no mapper
//         int mapperNum = (flag7 & 0xF0) | (flag6 >> 4);

//         //Initialize CPU
//         cpu = CPU(mapperNum);
        
//         // Instead of using OOP principles to implement mappers, each mapper will have a write function stored in a table

//         // Check for trainer; low-key don't know what to do if there is one in terms of writing to memory, so will just skip the trainer
//         // if there is one for now

//         if ((flag6 & 4) == 4) {
//             romFile.seekg(528);
//         }
//         else {
//             romFile.seekg(16);
//         }

//         // Read in the initial PRG-ROM into the CPU - varies based on mapper
//         // May need to re-engineer this later but this shall do for having no mapper probably

//         // Mirror the rom bank
//         if (prg_rom == 1) {
//             for (int i = 0; i < 16000; i++) {
//                 romFile.read(&byte, 1);
//                 cpu.memory[0x8000 + i] = byte;
//                 cpu.memory[0xC000 + i] = byte;
//             }
//         }
//         // Load first two banks into memory otherwise
//         else {
//             for (int i = 0; i < 32000; i++) {
//                 romFile.read(&byte, 1);
//                 cpu.memory[0x8000 + i] = byte;
//             }
//         }

//         // Read CHR-ROM
//         // CHR-ROM is used by the PPU to fill the pattern table, hence I will just move on for now
//         // If CHR ROM is 0, CHR RAM is used
//         if (chr_rom != 0) {
//             // NEEDS TO BE REPLACED LATER
//             romFile.seekg(8192 * chr_rom, std::ios::cur);
//         }

//         // Lastly, there's PlayChoice ROM which is kinda niche, but will deal with anyway
//         // If the 1st bit of flag 7 is set, there's 8KB of additional data to read called INST ROM as well as
//         // 16 bytes of PROM Data output used to decrypt the INST and 16 bytes of PROM CounterOut output used similarly which is sometimes ignored
//         if (flag7 & 2 == 2) {
//             // NEEDS TO BE REPLACED LATER
//             romFile.seekg(8192, std::ios::cur);
//         }

//         // May be an additional bit of data at the end of file but it can be safely ignored

//         // Can now begin execution

//         // Perform reset interrupt
//         cpu.interrupt_reset();
//         running = true;

//         // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
//         while (running) {
//             cpu.decode();
//         }

//         romFile.close();
//     }
//     else {

//         std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

//     }
// }

void Emulator::run(const char * filename) {
    running = false;
    // Reset components to known state
    cpu.manual_reset();

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

        //flags 8-10 are rarely used, bytes 11-15 are unused padding, so just jump to byte 16

        //Load everything into memory based on memory mapper
        //The mapper we use is determined by an 8 bit number whose lower nibble is the upper nibble of flag6 and whose upper nibble is the
        //upper nibble of flag7
        //For now, assume no mapper
        int mapperNum = (flag7 & 0xF0) | (flag6 >> 4);

        //Set CPU memory mapper
        cpu.set_memMap(mapperNum);
        
        // Instead of using OOP principles to implement mappers, each mapper will have a write function stored in a table

        // Check for trainer; low-key don't know what to do if there is one in terms of writing to memory, so will just skip the trainer
        // if there is one for now

        if ((flag6 & 4) == 4) {
            romFile.seekg(528);
        }
        else {
            romFile.seekg(16);
        }

        // Read in the initial PRG-ROM into the CPU - varies based on mapper
        // May need to re-engineer this later but this shall do for having no mapper probably

        // Mirror the rom bank
        if (prg_rom == 1) {
            for (int i = 0; i < 16000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
                cpu.memory[0xC000 + i] = byte;
            }
        }
        // Load first two banks into memory otherwise
        else {
            for (int i = 0; i < 32000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
            }
        }

        // Read CHR-ROM
        // CHR-ROM is used by the PPU to fill the pattern table, hence I will just move on for now
        // If CHR ROM is 0, CHR RAM is used
        if (chr_rom != 0) {
            // NEEDS TO BE REPLACED LATER
            romFile.seekg(8192 * chr_rom, std::ios::cur);
        }

        // Lastly, there's PlayChoice ROM which is kinda niche, but will deal with anyway
        // If the 1st bit of flag 7 is set, there's 8KB of additional data to read called INST ROM as well as
        // 16 bytes of PROM Data output used to decrypt the INST and 16 bytes of PROM CounterOut output used similarly which is sometimes ignored
        if (flag7 & 2 == 2) {
            // NEEDS TO BE REPLACED LATER
            romFile.seekg(8192, std::ios::cur);
        }

        // May be an additional bit of data at the end of file but it can be safely ignored

        // Can now begin execution

        // Perform reset interrupt
        cpu.interrupt_reset();
        running = true;

        // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
        while (running) {
            cpu.decode();
        }

        romFile.close();
    }
    else {

        std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

    }
}

// Function that just runs Kevin Horton's nestest in automation mode and creates a log file
void Emulator::nes_test() {
    running = false;
    cpu.manual_reset();

    this->romFile.open("nestest.nes", std::ios::in | std::ios::binary | std::ios::ate);
    std::ifstream good_log;
    good_log.open("nestest good log.txt");
    
    // Set the program counter to 0xC000 to run in automation mode
    cpu.set_PC(0xC000);

    // Load the test ROM
    // Skip header for now
    romFile.seekg(0x10);
    char byte;

    // Load the test into ROM
    for (int i = 0; i < 16000; i++) {
        romFile.read(&byte, 1);
        cpu.memory[0x8000 + i] = byte;
        cpu.memory[0xC000 + i] = byte;
    }

    // Create log file
    std::ofstream test_log = std::ofstream("nestest log.txt");

    std::string pc, acc, hi, low, sp, p, x, y, op;

    constexpr uint8_t pcIncrement[256] = {
    // 0x00
    1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0x10
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0x20
    3,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0x30
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0x40
    1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0x50
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0x60
    1,2,1,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0x70
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0x80
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0x90
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0xA0
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0xB0
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0xC0
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0xD0
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3,
    // 0xE0
    2,2,2,2,2,2,2,2,1,2,1,2,3,3,3,3,
    // 0xF0
    2,2,1,2,2,2,2,2,1,3,1,3,3,3,3,3
    };

    const char* const opcodeName[256] = {
    // 0x00
    "BRK","ORA","*KIL","*SLO","*NOP","ORA","ASL","*SLO",
    "PHP","ORA","ASL","*ANC","*NOP","ORA","ASL","*SLO",

    // 0x10
    "BPL","ORA","*KIL","*SLO","*NOP","ORA","ASL","*SLO",
    "CLC","ORA","*NOP","*SLO","*NOP","ORA","ASL","*SLO",

    // 0x20
    "JSR","AND","*KIL","*RLA","BIT","AND","ROL","*RLA",
    "PLP","AND","ROL","*ANC","BIT","AND","ROL","*RLA",

    // 0x30
    "BMI","AND","*KIL","*RLA","*NOP","AND","ROL","*RLA",
    "SEC","AND","*NOP","*RLA","*NOP","AND","ROL","*RLA",

    // 0x40
    "RTI","EOR","*KIL","*SRE","*NOP","EOR","LSR","*SRE",
    "PHA","EOR","LSR","*ALR","JMP","EOR","LSR","*SRE",

    // 0x50
    "BVC","EOR","*KIL","*SRE","*NOP","EOR","LSR","*SRE",
    "CLI","EOR","*NOP","*SRE","*NOP","EOR","LSR","*SRE",

    // 0x60
    "RTS","ADC","*KIL","*RRA","*NOP","ADC","ROR","*RRA",
    "PLA","ADC","ROR","*ARR","JMP","ADC","ROR","*RRA",

    // 0x70
    "BVS","ADC","*KIL","*RRA","*NOP","ADC","ROR","*RRA",
    "SEI","ADC","*NOP","*RRA","*NOP","ADC","ROR","*RRA",

    // 0x80
    "*NOP","STA","*NOP","*SAX","STY","STA","STX","*SAX",
    "DEY","*NOP","TXA","*XAA","STY","STA","STX","*SAX",

    // 0x90
    "BCC","STA","*KIL","*AHX","STY","STA","STX","*SAX",
    "TYA","STA","TXS","*TAS","*SHY","STA","*SHX","*AHX",

    // 0xA0
    "LDY","LDA","LDX","*LAX","LDY","LDA","LDX","*LAX",
    "TAY","LDA","TAX","*LAX","LDY","LDA","LDX","*LAX",

    // 0xB0
    "BCS","LDA","*KIL","*LAX","LDY","LDA","LDX","*LAX",
    "CLV","LDA","TSX","*LAS","LDY","LDA","LDX","*LAX",

    // 0xC0
    "CPY","CMP","*NOP","*DCP","CPY","CMP","DEC","*DCP",
    "INY","CMP","DEX","*AXS","CPY","CMP","DEC","*DCP",

    // 0xD0
    "BNE","CMP","*KIL","*DCP","*NOP","CMP","DEC","*DCP",
    "CLD","CMP","*NOP","*DCP","*NOP","CMP","DEC","*DCP",

    // 0xE0
    "CPX","SBC","*NOP","*ISB","CPX","SBC","INC","*ISB",
    "INX","SBC","NOP","*SBC","CPX","SBC","INC","*ISB",

    // 0xF0
    "BEQ","SBC","*KIL","*ISB","*NOP","SBC","INC","*ISB",
    "SED","SBC","*NOP","*ISB","*NOP","SBC","INC","*ISB"
    };

    constexpr bool unofficialOpcode[256] = {
    // 0x00
    false,false,true,true,true,false,false,true,false,false,false,true,true,false,false,true,
    // 0x10
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true,
    // 0x20
    false,false,true,true,false,false,false,true,false,false,false,true,false,false,false,true,
    // 0x30
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true,
    // 0x40
    false,false,true,true,true,false,false,true,false,false,false,true,false,false,false,true,
    // 0x50
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true,
    // 0x60
    false,false,true,true,true,false,false,true,false,false,false,true,false,false,false,true,
    // 0x70
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true,
    // 0x80
    true,false,true,true,false,false,false,true,false,true,false,true,false,false,false,true,
    // 0x90
    false,false,true,true,false,false,false,true,false,false,false,true,true,false,true,true,
    // 0xA0
    false,false,false,true,false,false,false,true,false,false,false,true,false,false,false,true,
    // 0xB0
    false,false,true,true,false,false,false,true,false,false,false,true,false,false,false,true,
    // 0xC0
    false,false,true,true,false,false,false,true,false,false,false,true,false,false,false,true,
    // 0xD0
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true,
    // 0xE0
    false,false,true,true,false,false,false,true,false,false,true,false,false,false,false,true,
    // 0xF0
    false,false,true,true,true,false,false,true,false,false,true,true,true,false,false,true
    };

    enum class AddressingMode {
    IMP,
    ACC,
    IMM,
    ZP,
    ZPX,
    ZPY,
    ABS,
    ABSX,
    ABSY,
    IND,
    INDX,
    INDY,
    REL,
    JSR
    };

    constexpr AddressingMode opcodeMode[256] = {
    // 0x00
    AddressingMode::IMP,  AddressingMode::INDX, AddressingMode::IMP,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::ACC,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0x10
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX,

    // 0x20
    AddressingMode::JSR,  AddressingMode::INDX, AddressingMode::IMP,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::ACC,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0x30
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX,

    // 0x40
    AddressingMode::IMP,  AddressingMode::INDX, AddressingMode::IMP,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::ACC,  AddressingMode::IMM,
    AddressingMode::JSR,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0x50
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX,

    // 0x60
    AddressingMode::IMP,  AddressingMode::INDX, AddressingMode::IMP,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::ACC,  AddressingMode::IMM,
    AddressingMode::IND,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0x70
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX,

    // 0x80
    AddressingMode::IMM,  AddressingMode::INDX, AddressingMode::IMM,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::IMP,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0x90
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPY,  AddressingMode::ZPY,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSY, AddressingMode::ABSY,

    // 0xA0
    AddressingMode::IMM,  AddressingMode::INDX, AddressingMode::IMM,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::IMP,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0xB0
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPY,  AddressingMode::ZPY,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSY, AddressingMode::ABSY,

    // 0xC0
    AddressingMode::IMM,  AddressingMode::INDX, AddressingMode::IMM,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::IMP,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0xD0
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX,

    // 0xE0
    AddressingMode::IMM,  AddressingMode::INDX, AddressingMode::IMM,  AddressingMode::INDX,
    AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,   AddressingMode::ZP,
    AddressingMode::IMP,  AddressingMode::IMM,  AddressingMode::IMP,  AddressingMode::IMM,
    AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,  AddressingMode::ABS,

    // 0xF0
    AddressingMode::REL,  AddressingMode::INDY, AddressingMode::IMP,  AddressingMode::INDY,
    AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,  AddressingMode::ZPX,
    AddressingMode::IMP,  AddressingMode::ABSY, AddressingMode::IMP,  AddressingMode::ABSY,
    AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX, AddressingMode::ABSX
    };

    running = true;
    std::string line;
    while (running) {
        std::getline(good_log, line);
        // Log state of registers, PC, etc before instruction is decoded
        uint16_t pcint = cpu.get_PC();
        pc  = hex(pcint, 4);
        acc = hex(cpu.get_accumulator(), 2);
        sp  = hex(cpu.get_stack(), 2);
        p   = hex(cpu.get_status(), 2);
        x   = hex(cpu.get_x(), 2);
        y   = hex(cpu.get_y(), 2);

        // Log values of opcode and operands
        op = hex(cpu.get_next_opcode(), 2);
        hi  = hex(cpu.get_next_high_nibble(), 2);
        low = hex(cpu.get_next_low_nibble(), 2);

        // Write to log file
        test_log << pc << std::setw(4);
        // Next part is opcode dependent
        test_log << op;
        int op_int = cpu.get_next_opcode();
        if (pcIncrement[op_int] == 1) {
            test_log << std::setw(11);
        }
        else if (pcIncrement[op_int] == 2) {
            test_log << " " << low << std::setw(8);
        }
        else if (pcIncrement[op_int] == 3) {
            test_log << " " << low << " " << hi << std::setw(5);
        }

        if (unofficialOpcode[op_int]) {
            
        }
        test_log << opcodeName[op_int];
        AddressingMode mode = opcodeMode[op_int];
        uint16_t exp, ind_add, exp_low, exp_high;
        switch (mode) {
            case AddressingMode::IMP:
                test_log << std::setw(31);
                break;
            case AddressingMode::ACC:
                test_log << " A" << std::setw(29);
                break;
            case AddressingMode::IMM:
                test_log << " #$" << low << std::setw(26);
                break;
            case AddressingMode::ZP:
                test_log << " $" << low << " = " << hex(cpu.memory[cpu.get_next_low_nibble()], 2) << std::setw(22);
                break;
            case AddressingMode::ZPX:
                exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                test_log << " $" << low << ",X @ " << hex(exp, 2) << " = " << hex(cpu.memory[exp], 2) << std::setw(15);
                break;
            case AddressingMode::ZPY:
                exp = (cpu.get_next_low_nibble() + cpu.get_y()) & 0xFF;
                test_log << " $" << low << ",Y @ " << hex(exp, 2) << " = " << hex(cpu.memory[exp], 2) << std::setw(15);
                break;
            case AddressingMode::JSR:
                test_log << " $" << hi << low << std::setw(25);
                break;
            case AddressingMode::ABSX:
                exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_x();
                test_log << " $" << hi << low << ",X @ " << hex(exp, 4) << " = " << hex(cpu.memory[exp], 2) << std::setw(11);
                break;
            case AddressingMode::ABSY:
                exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_y();
                test_log << " $" << hi << low << ",Y @ " << hex(exp, 4) << " = " << hex(cpu.memory[exp], 2) << std::setw(11);
                break;
            case AddressingMode::IND: 
                exp_low = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                exp_high = ((uint16_t) cpu.get_next_high_nibble() << 8) | ((cpu.get_next_low_nibble() + 1) & 0xFF);
                exp = ((uint16_t) cpu.memory[exp_high] << 8) | (uint16_t)cpu.memory[exp_low];
                test_log << " ($" << hi << low << ") = " << hex(exp, 4) << std::setw(16);
                break;
            case AddressingMode::INDX:
                exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                ind_add = ((uint16_t) cpu.memory[(exp + 1) & 0xFF] << 8) | cpu.memory[exp];
                test_log << " ($" << low << ",X) @ " << hex(exp, 2) << " = " << hex(ind_add, 4) << " = " << hex(cpu.memory[ind_add], 2) << std::setw(6);
                break;
            case AddressingMode::INDY:
                exp = (((uint16_t) cpu.memory[(cpu.get_next_low_nibble() + 1) & 0xFF] << 8) | cpu.memory[cpu.get_next_low_nibble()]);
                ind_add = exp + cpu.get_y();
                test_log << " ($" << low << "),Y = " << hex(exp, 4) << " @ " << hex(ind_add, 4) << " = " << hex(cpu.memory[ind_add], 2) << std::setw(4);
                break;
            case AddressingMode::REL:
                exp = pcint + (int8_t)cpu.get_next_low_nibble() + 2;
                test_log << " $" << hex(exp, 4) << std::setw(25);
                break;
            case AddressingMode::ABS:
                exp = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                test_log << " $" << hi << low << " = " << hex(cpu.memory[exp], 2) << std::setw(20);
                break;
        }

        // Decode and execute instruction
        cpu.decode();

        // Write state of registers
        test_log << "A:" << acc << " X:" << x << " Y:" << y << " P:" << p << " SP:" << sp << " ";

        // TODO: Add PPU and cycle info
        // Hackey bs for the sake of comparing files
        for (int i = 74; i < line.size(); i++) {
            test_log << line[i];
        }
        test_log << std::endl;
        if (pc == "66CE") {
            running = false;
        }
    }

    romFile.close();
    good_log.close();
}

// Helper function for writing log files
std::string hex(uint32_t value, int width)
{
    std::stringstream ss;
    ss << std::uppercase
       << std::hex
       << std::setw(width)
       << std::setfill('0')
       << value;
    return ss.str();
}