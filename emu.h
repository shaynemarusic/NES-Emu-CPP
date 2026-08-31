#include "cpu.h"
#include "./SDL2/include/SDL.h"

class Emulator {

    private:
        CPU cpu;
        PPU ppu;
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
        bool running;

        // The CPU runs at 1.79 MHz on NTSC systems - clock speed is in nanoseconds
        double const clock_speed = 1000000000 / 1790000;
        double render_speed = 1000 / 60;

        // Used for rendering
        const int SCREEN_WIDTH = 512, SCREEN_HEIGHT = 480, LOGICAL_WIDTH = 256, LOGICAL_HEIGHT = 240;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        void read_file(const char * filename);
        void emulator_cleanup();
        void update_screen();
    public:
        //Emulator(const char * filename);
        Emulator();
        void nes_test();
        void run(const char * filename);
        void run_log(const char * filename);
};

// A bunch of useful tables
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