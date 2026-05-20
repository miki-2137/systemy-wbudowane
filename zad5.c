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
#include "libpic30.h"
#include "lcd.h"
#include "adc.h"

unsigned char prev6 = 1, curr6 = 1;
unsigned char prev7 = 1, curr7 = 1;

unsigned int czas_g1 = 0;
unsigned int czas_g2 = 0;
unsigned int nastawa_czasu = 0;

unsigned char stan_gry = 0; 

void init(void)
{
    TRISD |= (1 << 6) | (1 << 7);
    
    LCD_Initialize();
    
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
}

void wyswietlMenu(void)
{
    LCD_ClearScreen();
    LCD_PutString("Nastawa czasu:", 14);
    LCD_PutChar('\n');
    
    if(nastawa_czasu == 300) LCD_PutString("5 minut", 7);
    else if(nastawa_czasu == 180) LCD_PutString("3 minuty", 8);
    else if(nastawa_czasu == 60) LCD_PutString("1 minuta", 8);
}

void wyswietlCzas(void)
{
    unsigned int mm1 = czas_g1 / 60;
    unsigned int ss1 = czas_g1 % 60;
    
    unsigned int mm2 = czas_g2 / 60;
    unsigned int ss2 = czas_g2 % 60;

    LCD_ClearScreen();
    
    LCD_PutString("G1: ", 4);
    LCD_PutChar('0' + (mm1 / 10));
    LCD_PutChar('0' + (mm1 % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss1 / 10));
    LCD_PutChar('0' + (ss1 % 10));
    if(stan_gry == 1) LCD_PutString(" <-", 3);

    LCD_PutChar('\n');

    LCD_PutString("G2: ", 4);
    LCD_PutChar('0' + (mm2 / 10));
    LCD_PutChar('0' + (mm2 % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss2 / 10));
    LCD_PutChar('0' + (ss2 % 10));
    if(stan_gry == 2) LCD_PutString(" <-", 3);
}

void wyswietlKoniec(unsigned char przegrany)
{
    LCD_ClearScreen();
    LCD_PutString("Koniec czasu!", 13);
    LCD_PutChar('\n');
    
    if(przegrany == 1) LCD_PutString("G1 przegral", 11);
    else LCD_PutString("G2 przegral", 11);
}

int main(void)
{
    init();

    while(1)
    {
        if(stan_gry == 0)
        {
            unsigned long adc_val = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
            if(adc_val != 0xFFFF)
            {
                unsigned int nowa_nastawa = 60;
                
                if(adc_val > 341 && adc_val <= 682) nowa_nastawa = 180;
                else if(adc_val > 682) nowa_nastawa = 300;
                
                if(nastawa_czasu != nowa_nastawa)
                {
                    nastawa_czasu = nowa_nastawa;
                    czas_g1 = nastawa_czasu;
                    czas_g2 = nastawa_czasu;
                    wyswietlMenu();
                }
            }

            prev6 = PORTDbits.RD6;
            prev7 = PORTDbits.RD7;
            
            __delay32(50000); 
            
            curr6 = PORTDbits.RD6;
            curr7 = PORTDbits.RD7;

            if(curr6 == 1 && prev6 == 0)
            {
                stan_gry = 2;
                wyswietlCzas();
            }
            else if(curr7 == 1 && prev7 == 0)
            {
                stan_gry = 1;
                wyswietlCzas();
            }
        }
        
        else if(stan_gry == 1 || stan_gry == 2)
        {
            int i;
            unsigned char zmiana_gracza = 0;
            
            for(i = 0; i < 20; i++)
            {
                prev6 = PORTDbits.RD6;
                prev7 = PORTDbits.RD7;
                
                __delay32(200000);
                
                curr6 = PORTDbits.RD6;
                curr7 = PORTDbits.RD7;

                if(stan_gry == 1 && curr6 == 1 && prev6 == 0)
                {
                    stan_gry = 2;
                    zmiana_gracza = 1;
                    break;
                }
                if(stan_gry == 2 && curr7 == 1 && prev7 == 0)
                {
                    stan_gry = 1;
                    zmiana_gracza = 1;
                    break;
                }
            }
            
            if(zmiana_gracza == 1)
            {
                wyswietlCzas();
                continue;
            }

            if(stan_gry == 1)
            {
                czas_g1--;
                if(czas_g1 == 0) stan_gry = 3;
            }
            else if(stan_gry == 2)
            {
                czas_g2--;
                if(czas_g2 == 0) stan_gry = 3;
            }
            
            if(stan_gry == 3)
            {
                if(czas_g1 == 0) wyswietlKoniec(1);
                else wyswietlKoniec(2);
            }
            else
            {
                wyswietlCzas();
            }
        }
        
        else if (stan_gry == 3)
        {
            prev6 = PORTDbits.RD6;
            prev7 = PORTDbits.RD7;
            
            __delay32(50000); 
            
            curr6 = PORTDbits.RD6;
            curr7 = PORTDbits.RD7;
            
            if((curr6 == 1 && prev6 == 0) || (curr7 == 1 && prev7 == 0))
            {
                stan_gry = 0;
                nastawa_czasu = 0;
            }
        }
    }

    return 0;
}
