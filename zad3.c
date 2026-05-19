
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
    TRISB |= (1 << 3);
}

int main(void) {
    
    init();
    
    unsigned long adc_value;
    unsigned char poprzedni_rb3 = 1;
    
    unsigned char stan_alarmu = 0;
    unsigned int licznik_czasu = 0;
    bool alarm_potwierdzony = false;
    
    while(1){
        adc_value = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
        
        unsigned char stan_rb3 = PORTDbits.RD6;
        
        if(poprzedni_rb3 == 1 && stan_rb3 == 0){
            alarm_potwierdzony = true;
            stan_alarmu = 0;
            LATA = 0x00;
            __delay32(40000);
        }
        poprzedni_rb3 = stan_rb3;
        
        if(adc_value != 0xFFFF) {
            if(adc_value < 512) {
                stan_alarmu = 0;
                alarm_potwierdzony = false;
                LATA = 0x00;
            } 
            else if (adc_value >= 512 && !alarm_potwierdzony) {
                if(stan_alarmu == 0) {
                    stan_alarmu = 1;
                    licznik_czasu = 0;
                }
            }
        }
        
        if(stan_alarmu == 1) {
            licznik_czasu++;
            
            if(licznik_czasu % 2 == 0) {
                if(LATA == 0x00) LATA = 0x01;
                else LATA = 0x00;
            }
            
            if(licznik_czasu >= 20) {
                stan_alarmu = 2;
            }
        } 
        else if (stan_alarmu == 2) {
            LATA = 0xFF;
        }
        __delay32(1000000); 
    }
    
    return 0;
}
