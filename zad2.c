
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
#include "adc.h"
#include "libpic30.h"
#include "stdbool.h"

void init(void){
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
    
    TRISA = 0x0000;
    TRISB |= (1 << 3) | (1 << 4);
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

int main(void) {
    
    init();
    
    unsigned long adc_value;
    unsigned char tryb = 0;
    
    unsigned char poprzedni_rb3 = 1;
    unsigned char poprzedni_rb4 = 1;
    
    unsigned long opoznienie;
    
    while(1){
        adc_value = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
        
        if(adc_value != 0xFFFF) {
            if(adc_value < 205) opoznienie = 100000;
            else if(adc_value < 410) opoznienie = 300000;
            else if(adc_value < 615) opoznienie = 500000;
            else if(adc_value < 820) opoznienie = 700000;
            else opoznienie = 900000;
        }
        unsigned char stan_rb3 = PORTDbits.RD6;
        unsigned char stan_rb4 = PORTDbits.RD7;
        
        if(poprzedni_rb3 == 1 && stan_rb3 == 0){
            tryb++;
            if(tryb > 1) tryb = 0; 
            __delay32(40000);
        }
        poprzedni_rb3 = stan_rb3;
        
        if(poprzedni_rb4 == 1 && stan_rb4 == 0){
            if(tryb == 0) tryb = 1; 
            else tryb--;
            __delay32(40000);
        }
        poprzedni_rb4 = stan_rb4;
        
        if(tryb == 0){
            snake();
        }
        else{
            queue();
        }
        
        __delay32(opoznienie);
    }
    
    return 0;
}
