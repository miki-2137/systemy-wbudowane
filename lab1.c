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

#include <xc.h>
#include <libpic30.h>
#include <stdbool.h>
#include "lcd.h"
#include "buttons.h"
#include "string.h"
#include "adc.h"

void blink(){
    unsigned char portValue;
    while(1){
        portValue = 0x55;
        LATA = portValue;
        __delay32(1000000);
        portValue = 0xAA;
        LATA = portValue;
        __delay32(1000000);
    }
}

void binaryUp(){
    unsigned int count = 0;
    while(1) {
        LATA = count++;
        __delay32(1000000);
    }
}

void binaryDown(){
    unsigned int count = 255;
    while(1) {
        LATA = count--;
        __delay32(1000000);
    }
}

void grayUp(){
    unsigned int count = 0;
    while(1) {
        unsigned int gray = count ^ (count >> 1);
        LATA = gray;
        __delay32(1000000);
        count++;
    }
}

void grayDown(){
    unsigned int count = 255;
    while(1) {
        unsigned int gray = count ^ (count >> 1);
        LATA = gray;
        __delay32(1000000);
        count--;
    }
}

void bcdUp(){
    
}

void BcdDown(){
    
}

void snake(){
    unsigned int pattern = 0b00000111;
    bool dir = 1;
    while(1){
        LATA = pattern;
        __delay32(1000000);
        if(dir){
            pattern <<= 1;
            if (pattern >= 0b11100000) dir = 0;
        }
        else{
            pattern >>= 1;
            if(pattern <= 0b00000111) dir = 1;
        }
    }
}

//void Kolejka(){
//    
//}

int main(void) {
    AD1PCFG = 0xFFFF; // ustawienie portu na tryb cyfrowy
    TRISA = 0x0000; // ustawienie portu A na wyjscie
    
    //blink();
    
    //binaryUp();
    
    //binaryDown();
    
    //grayUp();
    
    grayDown();
    
    //snake();
    
    return -1;
}
