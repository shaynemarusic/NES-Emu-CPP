#include <cstdint>
#include <memory>
#include <unordered_map>

// This class represents the PPU (duh, again). The NES used a 2C02
class PPU {

    private:
        // Memory mapped registers - these correlate to 0x2000 to 0x2007 of CPU memory and thus the CPU will have pointers to these
        // 0x2000
        uint8_t ppuctrl;
        // 0x2001
        uint8_t ppumask;
        // 0x2002
        uint8_t ppustatus;
        // 0x2003
        uint8_t oamaddr;
        // 0x2004
        uint8_t oamdata;
        // 0x2005
        uint16_t ppuscroll;
        // 0x2006
        uint16_t ppuaddr;
        // 0x2007
        uint8_t ppudata;
        // 0x4014
        uint8_t oamdma;

        // The PPU alaso has 4 internal registers
        // This represents the current VRAM address that the PPU is working with. Essentially, the nametable and attribute table 
        // addresses are calculated using v, and it is updated accordingly. The actual register is only 15 bits
        uint16_t v;
        // This is a temporary VRAM address which is copied into v at specific points in execution. It is also 15 bits
        uint16_t t;
        // Fine X scroll register. Not sure how this one is used yet. Only 3 bits
        uint8_t x;
        // Write latch. Since the CPU can only write one bite to the PPU at a time, this is used to indicate if the byte is the least
        // significant byte or the most significant byte. This register is shared by ppuaddr and ppuscroll, and can be reset by
        // having the CPU read from 0x2002
        bool w;

        uint16_t address_bus;
        uint8_t current_nametable_byte;
        uint8_t current_pattern_low_byte;
        uint8_t current_pattern_high_byte;
        uint8_t current_attribute_byte;
        uint16_t high_pattern_sr;
        uint16_t low_pattern_sr;
        uint16_t high_attribute_sr;
        uint16_t low_attribute_sr;
        uint8_t high_attribute_latch;
        uint8_t low_attribute_latch;
        uint8_t read_buffer;

        // Used to track where in the rendering process the PPU is at
        int scanline;
        int dot;
        // Used to track even/odd frames
        int frame;

        // Not sure if this is needed
        int memory_mapper;
        // Used to configure nametable mirroring
        int vertical_mirroring;

        // Memory
        /*
        Like the CPU, the PPU can address 64KB of memory, though it only actually has 16 (addresses past 3FFF are wrapped around)
        The situation is actually similar for the CPU, however, I was unaware when desigining that class; may need to adjust later
        Layout:
            Pattern Tables: 0000 - 0x1FFF
                Pattern tables store the 8x8 pixel tiles which can be drawn
                Many games store pattern tables in CHR-ROM, but those that don't use RAM and fill them during execution
                Each of the two adjacent tiles in a pattern table contain one of the least significant 
                bits of a color that is combined with info from an attribute table to determine the color of a given pixel. 
                For example if 0x0000[0] = 1 and 0x0008[0] = 0, then the lower order bits of that associated pixel's color are 0b01
                - Pattern Table 1: 0000 - 0x0FFF
                - Pattern Table 2: 0x1000 - 0x1FFF
            Name Tables: 0x2000 - 0x3EFF
                A Name table is a table of 32x30 tiles where each tile points to one of the tiles in the pattern tables (meaning each
                table covers a region of 256x240 pixels). Attribute tables contain the upper two bits of the color info for the
                associated tiles
                - Name Table 0: 0x2000 - 0x23BF
                - Attribute Table 0: 0x23C0 - 23FF
                - Name Table 1: 0x2400 - 0x27BF
                - Attribute Table 1: 0x27C0 - 0x27FF
                - Name Table 2: 0x2800 - 0x2BBF
                - Attribute Table 2: 0x2BC0 - 0x2BFF
                - Name Table 3: 0x2C00 - 0x2FBF
                - Attribute Table 3: 0x2FC0 - 0x2FFF
                - Mirrors of 0x2000 - 0x2EFF: 0x3000 - 0x3EFF - can probably be ignored
            Palettes: 0x3F00 - 0x3F1F
                Each palette contains 16 entries. Each entry is not a color itself, but rather an index to a color in the system
                palette. Bits 6 and 7 of each entry are ignorable
                - Image Palette: 0x3F00 - 0x3F0F
                    The colors usable for the background
                    0x3F00 is The Background Color (used when a sprite is transparent) and it is mirrored every four bytes
                - Sprite Palette: 0x3F10 - 0x3F1F
                    The colors usable for the sprites
                - Mirrors of 0x3F00 - 0x3F1F: 0x3F20 - 0x3FFF
            Mirrors: 0x4000 - 0x10000
        */

        // Fetching tiles is a 2 cycle process - on the first cycle, the address to fetch from is loaded on to the address bus
        // On the second cycle, the actual tile is fetched
        void fetch_nametable_address();
        void fetch_attribute_address();
        void fetch_patterntable_low_address();
        void fetch_patterntable_high_address();
        void fetch();

        void load_shift_registers();

        void increment_coarse_x();
        void increment_fine_y();

        void update_pixel();

        void shift_srs();

        // Return true if certain flags are true
        bool is_render_enabled();
        bool is_background_enabled();
        bool is_sprite_enabled();

        // Write functions
        void default_write(uint16_t address, uint8_t& val);

        // Nametable arrangement is determined by mapper and as such we use a table of function pointers to handle this
        std::unordered_map<int, void (PPU::*) (uint16_t address, uint8_t& val)> writes = {
            {0, &PPU::default_write}
        };

        uint8_t read(uint16_t address);
    public:
        uint8_t memory[65536];
        // Pixel information
        uint8_t frame_buffer[256 * 240 * 4];
        // Used to trigger NMIs
        bool nmi_trigger;
        PPU();
        void tick();
        void write(uint16_t address, uint8_t val);

        // Setters + Getters
        void set_memory_mapper(int mapper);
        int get_mapper() const;
    
        void set_mirroring(int vert);
        int get_mirroring() const;

        void set_ppuctrl(uint16_t value);
        uint8_t get_ppuctrl() const;

        void set_ppumask(uint8_t value);
        uint8_t get_ppumask() const;

        void set_ppustatus(uint8_t value);
        uint8_t get_ppustatus();

        void set_oamaddr(uint8_t value);
        uint8_t get_oamaddr() const;

        void set_oamdata(uint8_t value);
        uint8_t get_oamdata() const;

        void set_ppuscroll(uint16_t value);
        uint16_t get_ppuscroll() const;

        void set_ppuaddr(uint16_t value);
        uint16_t get_ppuaddr() const;

        void set_ppudata(uint8_t value);
        uint8_t get_ppudata();

        void set_oamdma(uint8_t value);
        uint8_t get_oamdma() const;
};