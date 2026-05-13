#pragma config POSCMOD = XT
#pragma config OSCIOFNC = ON
#pragma config FCKSM = CSDCMD
#pragma config FNOSC = PRI
#pragma config IESO = ON
#pragma config WDTPS = PS32768
#pragma config FWPSA = PR128
#pragma config WINDIS = ON
#pragma config FWDTEN = ON
#pragma config ICS = PGx2
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config JTAGEN = OFF

#include <xc.h>
#include "libpic30.h"
#include "adc.h"

void init(void) {
    AD1PCFG = 0xFFFF;
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
    
    TRISA = 0x0000;
    TRISB |= (1 << 3) | (1 << 4);
}

int main(void) {
    init();
    
    unsigned long adc_value;
    unsigned long opoznienie = 500000;
    
    unsigned char program = 0;
    unsigned char poprzedni_rb3 = 1;
    unsigned char poprzedni_rb4 = 1;

    unsigned char kolejka = 0x01;
    unsigned char wypelnienie = 0x00;
    unsigned char lfsr = 0b111001;

    while(1) {
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

        if(poprzedni_rb3 == 1 && stan_rb3 == 0) {
            program++;
            if(program > 1) program = 0;
            
            kolejka = 0x01;
            wypelnienie = 0x00;
            lfsr = 0b111001;
            __delay32(40000);
        }
        
        if(poprzedni_rb4 == 1 && stan_rb4 == 0) {
            if(program == 0) program = 1;
            else program--;
            
            kolejka = 0x01;
            wypelnienie = 0x00;
            lfsr = 0b111001;
            __delay32(40000);
        }
        
        poprzedni_rb3 = stan_rb3;
        poprzedni_rb4 = stan_rb4;

        if (program == 0) {
            LATA = wypelnienie | kolejka;
            if ((kolejka == 0x80) || ((kolejka << 1) & wypelnienie)) {
                wypelnienie |= kolejka;
                kolejka = 0x01;
                if (wypelnienie == 0xFF) {
                    wypelnienie = 0x00;
                }
            } else {
                kolejka <<= 1;
            }
        } else {
            unsigned char bit = ((lfsr >> 5) ^ (lfsr >> 4)) & 1;
            lfsr = (lfsr << 1) | bit;
            LATA = lfsr;
        }

        __delay32(opoznienie);
    }

    return 0;
}
