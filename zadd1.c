// PIC24FJ128GA010 Configuration Bit Settings

// 'C' source line config statements

// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT Oscillator mode selected)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = ON              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include "xc.h"
#include "libpic30.h"
#include "stdbool.h"
#include <stdint.h>

void binaryUp(){
    static unsigned char count = 0;
    LATA = count++;
}

void binaryDown(){
    static unsigned char count = 255;
    LATA = count--;
}

void grayUp(){
    static unsigned char count = 0;
    unsigned char gray = count ^ (count >> 1);
    LATA = gray;
    count++;
}

void grayDown(){
    static unsigned char count = 255;
    unsigned char gray = count ^ (count >> 1);
    LATA = gray;
    count--;
}

void bcdUp(){
    static char jednosci = 0;
    static char dziesiatki = 0;
    
    LATA = (dziesiatki << 4) | jednosci;
    
    jednosci++;
    if(jednosci > 9){
        jednosci = 0;
        dziesiatki++;
        if(dziesiatki > 9){
            dziesiatki = 0;
        }
    }
}

void bcdDown(){
    static char jednosci = 9;
    static char dziesiatki = 9;
    
    LATA = (dziesiatki << 4) | jednosci;
    
    jednosci--;
    if(jednosci < 0){
        jednosci = 9;
        dziesiatki--;
        if(dziesiatki < 0){
            dziesiatki = 9;
        }
    }
}

void snake(){
    static unsigned char pattern = 0b00000111;
    static bool dir = 1;
    
    LATA = pattern;
    
    if(dir){
        pattern <<= 1;
        if (pattern >= 0b11100000) dir = 0;
    }
    else{
        pattern >>= 1;
        if(pattern <= 0b00000111) dir = 1;
    }
}

void queue(){
    static int q = 0;
    static int walk = 1;
    
    LATA = q + walk; 
    
    int next_step = walk * 2;
    
    if (walk == 128 || (next_step & q) > 0) {
        q += walk;
        walk = 1;
        
        if (q == 255) {
            q = 0;
        }
    } else {
        walk = walk * 2;
    }
}

void prng(){
    static unsigned char lfsr = 1;
    
    LATA = lfsr & 0x3F;
    if (lfsr & 1) {
        lfsr = (lfsr >> 1) ^ 0b0111001;
    } else {
        lfsr = (lfsr >> 1);
    }
}

int main(void) {
    int value = 1;
    
    TRISA = 0x0000;
    TRISD = 0xFFFF;
    
    char prev6 = PORTDbits.RD6;
    char prev7 = PORTDbits.RD7;
    char current6, current7;
    
    while(1){
        switch(value){
            case 1: binaryUp(); break;
            case 2: binaryDown(); break;
            case 3: grayUp(); break;
            case 4: grayDown(); break;
            case 5: bcdUp(); break;
            case 6: bcdDown(); break;
            case 7: snake(); break;
            case 8: queue(); break;
            case 9: prng(); break;
        }
        
        __delay32(750000); 
        
        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;
        
        if(current6 == 1 && prev6 == 0){
            value--;
            if(value < 1){
                value = 9;
            }
        }
        
        if(current7 == 1 && prev7 == 0){
            value++;
            if(value > 9){
                value = 1;
            }
        }
        
        prev6 = current6;
        prev7 = current7;
    }
    
    return 0;
}
