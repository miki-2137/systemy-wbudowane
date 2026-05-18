#include "xc.h"
#include "adc.h"
#include "libpic30.h"
#include "stdbool.h"

void init(void){
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
    
    TRISA = 0x0000;             // Diody jako wyjścia
    TRISB |= (1 << 3);          // Przycisk RB3 jako wejście
}

int main(void) {
    
    init();
    
    unsigned long adc_value;
    unsigned char poprzedni_rb3 = 1;
    
    // Zmienne obsługujące logikę alarmu
    unsigned char stan_alarmu = 0;     // 0 = wyłączony, 1 = mruganie, 2 = ciągłe świecenie wszystkich
    unsigned int licznik_czasu = 0;
    bool alarm_potwierdzony = false;   // Blokuje alarm po wciśnięciu przycisku dopóki ADC nie spadnie
    
    while(1){
        adc_value = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
        
        unsigned char stan_rb3 = PORTBbits.RB3; // Odczyt pinu RB3
        
        // 1. Obsługa przycisku wyłączającego alarm (zbocze opadające)
        if(poprzedni_rb3 == 1 && stan_rb3 == 0){
            alarm_potwierdzony = true;
            stan_alarmu = 0;
            LATA = 0x00;        // Zgaś wszystkie diody
            __delay32(40000);   // Debouncing przycisku
        }
        poprzedni_rb3 = stan_rb3;
        
        // 2. Obsługa wskazań potencjometru i wyzwalania alarmu
        if(adc_value != 0xFFFF) {
            if(adc_value < 512) {
                // Gdy wrócimy poniżej połowy zakresu - wyłączamy alarm i zdejmujemy blokadę
                stan_alarmu = 0;
                alarm_potwierdzony = false;
                LATA = 0x00;
            } 
            else if (adc_value >= 512 && !alarm_potwierdzony) {
                // Jeśli przekroczono próg i alarm nie został wyciszony przyciskiem
                if(stan_alarmu == 0) {
                    stan_alarmu = 1;
                    licznik_czasu = 0; // Zerowanie czasu przy starcie alarmu
                }
            }
        }
        
        // 3. Maszyna stanów wykonująca logikę świecenia
        if(stan_alarmu == 1) {
            licznik_czasu++;
            
            // Mruganie jedną diodą (np. RA0) co 5 pętli (ok. 250ms)
            if(licznik_czasu % 5 == 0) {
                if(LATA == 0x00) LATA = 0x01;
                else LATA = 0x00;
            }
            
            // Przejście do ciągłego świecenia po 5 sekundach.
            // Przy pętli trwającej ~50ms, 5 sekund to około 100 pętli
            if(licznik_czasu >= 100) {
                stan_alarmu = 2;
            }
        } 
        else if (stan_alarmu == 2) {
            LATA = 0xFF; // Po upływie czasu - ciągłe świecenie wszystkich diod (zależnie od uC i szerokości portu, 0xFF załącza 8 diod)
        }
        
        // Bazowe opóźnienie pojedynczego kroku pętli (w tym wypadku ~50ms)
        // Wartość do ewentualnego przeliczenia w zależności od kwarcu/zegara (FCY)
        __delay32(500000); 
    }
    
    return 0;
}// SECOND VERSION

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
    
    TRISA = 0x0000;             // Port A (diody) jako wyjścia
    TRISB |= (1 << 3);          // Port B bit 3 (przycisk RB3) jako wejście
}

int main(void) {
    
    init();
    
    unsigned long adc_value;
    unsigned char poprzedni_rb3 = 1;
    
    bool alarm_aktywny = false;
    bool alarm_zignorowany = false; // Blokuje alarm po wciśnięciu przycisku aż do zjechania potencjometrem
    unsigned int licznik_czasu = 0;
    
    while(1){
        adc_value = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
        
        // Odczyt stanu przycisku RB3
        unsigned char stan_rb3 = PORTBbits.RB3;
        
        // 1. Logika włączania i wyłączania alarmu potencjometrem
        // Połowa zakresu 10-bitowego ADC (0-1023) to 512
        if (adc_value < 512) {
            alarm_aktywny = false;
            alarm_zignorowany = false; // Reset blokady po spadku poniżej nastawy
            licznik_czasu = 0;
        } else if (adc_value >= 512 && !alarm_zignorowany) {
            alarm_aktywny = true;
        }
        
        // 2. Obsługa przycisku RB3 (wyłączenie alarmu)
        if(poprzedni_rb3 == 1 && stan_rb3 == 0){
            if (alarm_aktywny) {
                alarm_aktywny = false;
                alarm_zignorowany = true; // Alarm wyciszony do czasu aż pot. spadnie poniżej 512
            }
            __delay32(40000); // Prosty debounce
        }
        poprzedni_rb3 = stan_rb3;
        
        // 3. Logika świecenia diod na podstawie stanu alarmu
        if (!alarm_aktywny) {
            LATA = 0x00; // Zgaś wszystkie diody
        } else {
            licznik_czasu++;
            
            // Założenie: jedna iteracja pętli to ok. 100ms (zależy od kwarcu i opóźnienia)
            // 5 sekund = 50 iteracji po 100ms
            if (licznik_czasu < 50) {
                // Przez pierwsze 5 sekund - mruga jedna dioda (np. najmłodszy bit)
                if (licznik_czasu % 2 == 0) {
                    LATA = 0x01;
                } else {
                    LATA = 0x00;
                }
            } else {
                // Po 5 sekundach - zapalone wszystkie diody (ciągłe świecenie)
                LATA = 0xFF;
                
                // Zabezpieczenie przed przepełnieniem zmiennej
                if (licznik_czasu > 30000) licznik_czasu = 50; 
            }
        }
        
        // Opóźnienie głównej pętli (~100 ms)
        // Uwaga: Wartość 400000 może wymagać kalibracji w zależności od częstotliwości Twojego oscylatora (FCY), 
        // aby pętla trwała dokładnie tyle, by 50 cykli wynosiło 5 sekund.
        __delay32(400000);
    }
    
    return 0;
}
