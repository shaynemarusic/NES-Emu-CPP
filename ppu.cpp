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
        // Idle cycle - the address bus is loaded with the address to the low background tile byte
        if (dot == 0) {
            fetch_patterntable_low_address();
        }
        // Regular execution - a pixel is selected from the shift registers and it is updated in the pixel buffer
        // At the same time, the PPU is continually fetching data for the next set of 8 pixels
        else if (dot < 257) {
            // "Draw" a pixel (this updates the shift registers as well)
            update_pixel();

            int dot_mod = dot % 8;
            // Check if we need to load shift registers
            // Every time a nametable byte is fetched with the exception of cycles 1 and 321, the fetched data is loaded into
            // the appropriate shift register
            if (dot != 1 && dot_mod == 1) load_shift_registers();

            // Fetching
            fetch();

            // Check if we need to increment coarse x - this is done every 8 cycles 
            if (dot_mod == 0) increment_coarse_x();

            // Check if we need to increment fine y - this is done only on cycle 256
            if (dot == 256) increment_fine_y();
        }
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

// This oversees fetching for most cycles where the PPU needs to fetch data
void PPU::fetch() {
    int dot_mod = dot % 8;
    switch (dot_mod) {
        // Fetch the background high byte
        case 0:
            current_pattern_high_byte = memory[address_bus];
            break;
        // Fetch nametable address
        case 1:
            fetch_nametable_address();
            break;
        // Fetch nametable byte
        case 2:
            current_nametable_byte = memory[address_bus];
            break;
        // Fetch attribute address
        case 3:
            fetch_attribute_address();
            break;
        // Fetch attribute byte
        case 4:
            current_attribute_byte = memory[address_bus];
            break;
        // Fetch pattern low address
        case 5:
            fetch_patterntable_low_address();
            break;
        // Fetch pattern low byte
        case 6:
            current_pattern_low_byte = memory[address_bus];
            break;
        // Fetch pattern high address
        case 7:
            fetch_patterntable_high_address();
            break;
    }
}

void PPU::load_shift_registers() {

    // This low bit/latch is selected using the coarse x and y bits from v (speficially bit 1 and bit 5)
    uint8_t bit_selector = ((1 & (v >> 1)) * 2 + (1 & (v >> 5)) * 4);
    low_attribute_latch = (current_attribute_byte >> bit_selector) & 1;
    high_attribute_latch = (current_attribute_byte >> (bit_selector + 1)) & 1;
    low_pattern_sr = (low_pattern_sr & 0xFF00) | current_pattern_low_byte;
    high_pattern_sr = (high_pattern_sr & 0xFF00) | current_pattern_high_byte;

}

// Basically ripped straight from the wiki page on scrolling
// We need to wrap if coarse x overflows - in that case we set coarse x to 0 and switch the horizontal nametable
void PPU::increment_coarse_x() {

    if ((v & 0x1F) == 31) {
        v &= ~0x1F;
        v ^= 0x400; // This toggles the horizontal nametable bit
    }
    else v++;

}

// Also ripped from the wiki
// Increment fine y, overflow to coarse y, and lastly if that overflows, flip the vertical nametable bit like incrementing coarse x
void PPU::increment_fine_y() {

    // If fine y is less than 7
    if ((v & 0x7000) != 0x7000) v += 0x1000;
    else {
        // Set fine y to 0
        v &= ~0x7000;
        uint16_t coarse_y = (v & 0x3E0) >> 5;
        if (coarse_y == 29) {
            coarse_y = 0;
            // Flip vertical nametable bit
            v ^= 0x800;
        }
        // Coarse y can be set out of bounds, which does not cause the nametable to switch - this has some funky effects that some 
        // games rely on
        else if (coarse_y == 31) coarse_y = 0;
        else coarse_y++;

        v = (v & ~0x3E0) | (coarse_y << 5);
    }

}

// This selects bits from our shift registers and updates the corresponding buffer entry with the new pixel data
void PPU::update_pixel() {

    // x selects a bit from each shift register to create a 4 bit number
    uint8_t bit = 15 - x;
    uint8_t low_pixel_bit = (low_pattern_sr >> bit) & 1;
    uint8_t high_pixel_bit = (high_pattern_sr >> bit) & 1;
    uint8_t bg_pixel = (high_pixel_bit << 1) | low_pixel_bit;

    uint8_t low_palette_bit = (low_attribute_sr >> bit) & 1;
    uint8_t high_palette_bit = (high_attribute_sr >> bit) & 1;
    uint8_t palette = (high_palette_bit << 1) | low_palette_bit;
    // palette * 4 + pixel identifies a color in the background palette

    // TODO - will just focus on the background for now
    // Cross reference with sprite pixel data to determine which gets drawn

    // Once that is done, we are left with a 5 bit number S AA PP where S selects the background or sprite palette, A
    // is the attribute data (palette number selector), and P is the pattern table data (pixel value)
    uint8_t drawn_pixel = (palette << 2) | bg_pixel;
    uint8_t color_index = memory[drawn_pixel | 0x3F00];

    // This byte is used to lookup a color in the system palette. On actual hardware, there is no RGB signal, but here we just store
    // a table of RGB values that someone else made
}