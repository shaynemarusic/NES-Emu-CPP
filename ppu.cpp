#include "ppu.h"

// Constructors

// Default constructor
PPU::PPU() {
    std::fill_n(memory, 65536, 0);
    vertical_mirroring = 0;
    memory_mapper = 0;

    // Initialize frame buffer
    for (int i = 0; i < 256; i++) {
        std::fill_n(frame_buffer[i], 240, 0);
    }

    // Set MMIO registers
    ppuctrl = 0;
    ppumask = 0;
    ppustatus = 0;
    oamaddr = 0;
    oamdata = 0;
    ppuscroll = 0;
    ppuaddr = 0;
    ppudata = 0;
    oamdma = 0;
    
    // Set internal registers and other variables
    v = 0;
    t = 0;
    w = 0;
    x = 0;
    current_nametable_byte = 0;

    // Set state variables
    scanline = 261;
    dot = 0;
    frame = 0;

    // Set bus
    address_bus = 0;
}

// Setters + Getters
void PPU::set_memory_mapper(int mapper) { memory_mapper = mapper; }

int PPU::get_mapper() const { return memory_mapper; }

void PPU::set_mirroring(int vert) { vertical_mirroring = vert; }

int PPU::get_mirroring() const { return vertical_mirroring; }

// Modifies t - sets the nametable select bits (bits 10-11) with the 2 least significant bits
void PPU::set_ppuctrl(uint16_t value) { 
    
    ppuctrl = value;
    t = (t & 0x73FF) | (value & 0xFC);

}

uint8_t PPU::get_ppuctrl() const { return ppuctrl; }

void PPU::set_ppumask(uint8_t value) { ppumask = value; }

uint8_t PPU::get_ppumask() const { return ppumask; }

void PPU::set_ppustatus(uint8_t value) { ppustatus = value; }

// This sets w to 0
uint8_t PPU::get_ppustatus() {

    w = false;
    return ppustatus; 

}

void PPU::set_oamaddr(uint8_t value) { oamaddr = value; }

uint8_t PPU::get_oamaddr() const { return oamaddr; }

void PPU::set_oamdata(uint8_t value) { oamdata = value; }

uint8_t PPU::get_oamdata() const { return oamdata; }

// Toggles w
// Modifies t and x
void PPU::set_ppuscroll(uint16_t value) { 

    // First write
    if (!w) {

        ppuscroll = (ppuscroll & 0xFF00) | value;
        // x is set to the lower 3 bits of value
        x = value & 7;
        // The lower 5 bits of t (coarse x bits) are set to the upper 5 bits of value
        t = (t & 0x7EE0) | (value >> 3);

    }
    // Second write
    else {
        
        ppuscroll = (ppuscroll & 0xFF) | (value << 8);
        // Upper 3 bits of t (fine y bits) are set to the lower 3 bits of value, bits 5-9 of t (coarse y bits) are 
        // set to the next 5 bits of value
        t = (t & 0x4C1F) | ((value & 0x7) << 12) | ((value & 0xF8) << 2);

    }
    w = !w;

 }

uint16_t PPU::get_ppuscroll() const { return ppuscroll; }

// Toggles w
// Modifies t. On the second write, t is copied to v - this copying occurs about a dot after the actual write
void PPU::set_ppuaddr(uint16_t value) {

    // First write
    if (!w) {
        
        ppuaddr = (ppuaddr & 0xFF00) | value;
        // bits 8-13 of t are set to the 6 least significant bits of value, the most significant bit is cleared
        t = (t & 0x1FF) | ((value & 0x3F) << 8);
    
    }
    // Second write
    else {
        
        ppuaddr = (ppuaddr & 0xFF) | (value << 8);
        // The lower byte of t is set to value
        t = (t & 0x7F00) | value;
        // Copy to v - not sure how important the timing of this is, but I'll just assume it's important
        // TODO
    
    }
    w = !w;

}

uint16_t PPU::get_ppuaddr() const { return ppuaddr; }

// TODO
// This causes v to increment depending on the value of ppuctrl and has some other funky effects that I will mess with later
void PPU::set_ppudata(uint8_t value) { ppudata = value; }

uint8_t PPU::get_ppudata() const { return ppudata; }

void PPU::set_oamdma(uint8_t value) { oamdma = value; }

uint8_t PPU::get_oamdma() const { return oamdma; }

// Execution related functions

// Runs the PPU for one dot/PPU cycle - a PPU cycle is a third of a CPU cycle
void PPU::tick() {
    // Need to consider where in the rendering process we are - e.g. what scanline we are on/where in the scanline we are at
    // Defer updating the window until a full pass of the screen has been made
    // We will keep pixel information in a buffer

    // These are the visible scanlines - the ppu actually modifies visible pixels in this section
    if (scanline <= 239) {

    }
    // The post-render scanline - the ppu just idles during this scanline
    else if (scanline == 240) {

    }
    // This is the beginning of VBlank period - the VBlank is set during the second tick of scanline 241 and an NMI is triggered
    else if (scanline == 241) {

    }
    // VBlank - the PPU essentially does nothing until it reaches scanline 261
    else if (scanline < 261) {

    }
    // Scanline 261 - This is the prerender scanline (aka scanline -1). The PPU loads data to prepare for rendering in scanline 0
    // Length of this scanline is dependent on if an even or odd frame is being rendered
    else {

    }

    dot++;
    // Reached the end of a scanline
    if (dot == 340) {
        dot = 0;
        scanline += 1;
    }
}

// Tile fetching related functions

// The nametable address is essentially just v ignoring the 3 most significant bits (Y fine) and or'ed with 0x2000
void PPU::fetch_nametable_address() { address_bus = 0x2000 | (v & 0xFFF); }

// The attribute address takes the form NN 1111 YYY XXX where N are the nametable select bits of v, 1111 is the attribute offset,
// and YYY and XXX are the upper 3 bits of the Y coarse and X coarse bits in v (essentially y/4 and x/4)
void PPU::fetch_attribute_address() { address_bus = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) | 0x07); }

// The pattern table address take the form 0 H NNNN NNNN P yyy where H determines which table is being fetched from (this comes from
// PPUCTRL) NNNN NNNN is the nametable tile number, P determines if it's the significant byte or the less significant byte, and yyy
// is fine y
void PPU::fetch_patterntable_low_address() { address_bus = (((uint16_t)ppuctrl << 7) & 0x1000) | (((uint16_t)current_nametable_byte << 4) & 0xFF0) | ((v >> 12) & 0x7); }

void PPU::fetch_patterntable_high_address() { address_bus = 8 | (((uint16_t)ppuctrl << 7) & 0x1000) | (((uint16_t)current_nametable_byte << 4) & 0xFF0) | ((v >> 12) & 0x7); }
