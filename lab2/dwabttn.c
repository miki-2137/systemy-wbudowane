/*
 * File:   dwabttn.c
 * Author: local
 *
 * Created on 15 kwietnia 2026, 10:31
 */


#include "xc.h"
#include "libpic30.h"

int mainxd(void) {
    
    unsigned portValue = 0x0001;
    char current6 = 0, prev6 = 0, current7 = 0, prev7 = 0;
    
    int value = 1;
    
    TRISA = 0x0000;
    TRISD = 0xFFFF;
    
    while(1){
        switch(value){
            case 1:
                LATA = portValue;
                break;
            case 2:
                LATA = portValue + 1;
                break;
            case 3:
                LATA = portValue + 3;
                break;
            default:
                LATA = value;
        }
        prev6 = PORTDbits.RD6;
        prev7 = PORTDbits.RD7;
        __delay32(150000);
        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;
        
        if(current6 - prev6 == 1){
            value--;
            portValue = 1;
            if(value <= 0){
                value = 3;
            }
        }
        
        if(current7 - prev7 == 1){
            value++;
            portValue = 1;
            if(value >= 4){
                value = 1;
            }
        }
    }
    
    return 0;
}
