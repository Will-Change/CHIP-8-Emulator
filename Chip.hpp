//we just declare all of our class members, kinda the easiest one tbh, wanted to not make a header file but i guess we'll just have to run with this then, does using a header file make our program more readable? maybe i dont know

#ifndef CHIP_8_H
#define CHIP_8_H

//#ifndef and #define are known as header guards. Their primary purpose is to prevent C++ header files from being included multiple times.

#include <stdint.h>//header file to use our datatypes the way we do 

class Chip8 {
    
    //we will just declare all our class members in this part that will match the CHIP-8 discription
    
private:
    uint16_t stack[16];// our 16 level stack
    uint16_t sp;// its a 8 bit stack pointer, but everyone seems to act like its 16 bit so i dont know dude

    uint8_t memory[4096];//4k memory from 0x000 to 0xFFF
    
    uint8_t V[16];//registers are nothing but dedicated locations on a cpu for storage,our little chip-8 buddy here has 16 8 bit registers, its expensive so we dont have much, but we still have the 4k memory tho(V0-VF)

    //most of the stuff we have after this has to be 16 bit as it may have to store 0xFFF
    
    uint16_t pc;//cpu needs to keep track of which instruction to execute next, this is takes care of that, we'll keep incrementing this guy by 2 lol
    uint16_t opcode;//ass the name suggests
    uint16_t I;// this is going to be our index register, its nothing but a special register used to store memory addreses for use in operation

    uint8_t delay_timer;// counts to zero nothing special kinda self explanatory
    uint8_t sound_timer;// sound timer, makes a beep when not zero, will actually incorporate this later

    void init();

public:
    uint8_t  gfx[64 * 32];//graphics buffer as a 2d array
    uint8_t  key[16];//key buffer
    bool drawFlag;// etermines if something was written to buffer and needs to be pushed

    Chip8();//constructor
    ~Chip8();//destructor

    void emulate_cycle();//function that is used for a single emulated CPU cycle.
    bool loadROM(const int argc, const char *file_name);//function that is used for loading programs into our memory.
};

#endif // CHIP_8_H
