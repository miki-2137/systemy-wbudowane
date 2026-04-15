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
#include "stdbool.h"
#include "lcd.h"
#include "buttons.h"
#include "string.h"
#include "adc.h"

int mainx(void) {
    
    unsigned portValue = 0x0001; // ustawienie poczatkowego stanu diody
    char current6 = 0, prev6 = 0; // zmienne do detekcji przycisku RD6
    
    int value = 1; // stan dzialania programu
    
    TRISA = 0x0000; // port A na wyjscie
    TRISD = 0xFFFF; // port D na wejscie
    
    while(1){
        switch(value){
            case 1:
                LATA = portValue;
                portValue++;
                if(portValue == 0xFF){
                    portValue = 0;
                }
        break;
            case 2:
                LATA = portValue;
                portValue--;
                if(portValue == 0xFF){
                    portValue = 0;
                }
                break;
        }
        prev6 = PORTDbits.RD6;
        __delay32(1000000);
        current6 = PORTDbits.RD6;
        
        if(prev6 == 1 && current6 == 0){
            value++;
            if(value > 2){
                value = 1;
            }
            portValue = 1;
        } 
    }
    
    //blink();
    
    //binaryUp();
    
    //binaryDown();
    
    //grayUp();
    
    //grayDown();
    
    //snake();
    
    return 0;
}
