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

unsigned int moc[4] = {800, 600, 350, 200};
unsigned int wybrana = 0;
unsigned long czas = 0;

void init(void)
{
    AD1PCFG = 0xFFFF;
    
    TRISA = 0x0000;
    LATA = 0x0000;
    
    TRISD |= (1 << 6) | (1 << 7) | (1 << 13);
    TRISAbits.TRISA7 = 1;

    LCD_Initialize();
}

void wyswietl(void)
{
    unsigned int min = czas / 60;
    unsigned int sek = czas % 60;

    LCD_ClearScreen();
    LCD_PutString("Moc: ", 5);

    if(moc[wybrana] == 800) LCD_PutString("800W", 4);
    else if(moc[wybrana] == 600) LCD_PutString("600W", 4);
    else if(moc[wybrana] == 350) LCD_PutString("350W", 4);
    else LCD_PutString("200W", 4);

    LCD_PutChar('\n');

    LCD_PutString("Czas: ", 6);
    LCD_PutChar('0' + (min / 10));
    LCD_PutChar('0' + (min % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (sek / 10));
    LCD_PutChar('0' + (sek % 10));
}

void wyswietlGotowanie(void)
{
    unsigned int min = czas / 60;
    unsigned int sek = czas % 60;

    LCD_ClearScreen();
    LCD_PutString("Gotowanie", 9);
    LCD_PutChar('\n');

    LCD_PutChar('0' + (min / 10));
    LCD_PutChar('0' + (min % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (sek / 10));
    LCD_PutChar('0' + (sek % 10));
}

int main(void)
{
    init();
    wyswietl();

    unsigned char poprzedni_rd6 = 1, stan_rd6 = 1;
    unsigned char poprzedni_rd7 = 1, stan_rd7 = 1;
    unsigned char poprzedni_ra7 = 1, stan_ra7 = 1;
    unsigned char poprzedni_rd13 = 1, stan_rd13 = 1;
    
    unsigned char wlaczona = 0;

    while(1)
    {
        if(wlaczona == 0)
        {
            poprzedni_rd6 = PORTDbits.RD6;
            poprzedni_rd7 = PORTDbits.RD7;
            poprzedni_ra7 = PORTAbits.RA7;
            poprzedni_rd13 = PORTDbits.RD13;

            __delay32(50000);

            stan_rd6 = PORTDbits.RD6;
            stan_rd7 = PORTDbits.RD7;
            stan_ra7 = PORTAbits.RA7;
            stan_rd13 = PORTDbits.RD13;

            if(poprzedni_rd6 == 1 && stan_rd6 == 0)
            {
                wybrana++;
                if(wybrana > 3) wybrana = 0;
                wyswietl();
            }

            if(poprzedni_rd7 == 1 && stan_rd7 == 0)
            {
                czas += 60;
                wyswietl();
            }

            if(poprzedni_ra7 == 1 && stan_ra7 == 0)
            {
                czas += 10;
                wyswietl();
            }

            if(poprzedni_rd13 == 1 && stan_rd13 == 0)
            {
                if(czas > 0)
                {
                    wlaczona = 1;
                    wyswietlGotowanie();
                }
            }
        }
        else 
        {
            poprzedni_rd13 = PORTDbits.RD13;
            
            __delay32(4000000);
            
            stan_rd13 = PORTDbits.RD13;

            if(poprzedni_rd13 == 1 && stan_rd13 == 0)
            {
                wlaczona = 0;
                wyswietl();
                continue; 
            }

            if(czas > 0)
            {
                czas--;
                wyswietlGotowanie();
            }

            if(czas == 0)
            {
                wlaczona = 0;

                LCD_ClearScreen();
                LCD_PutString("Koniec!!!", 9);
                LCD_PutChar('\n');
                LCD_PutString("00:00", 5);

                __delay32(10000000);

                wyswietl();
            }
        }
    }

    return 0;
}
