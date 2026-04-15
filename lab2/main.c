/*
 * File:   main.c
 * Author: local
 *
 * Created on 15 kwietnia 2026, 10:49
 */


#include "xc.h"
#include "libpic30.h"
#include "stdbool.h"

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
    unsigned portValue = 0x0001;
    char current6 = 0, prev6 = 0, current7 = 0, prev7 = 0;
    
    int value = 1;
    
    TRISA = 0x0000;
    TRISD = 0xFFFF;
    
    while(1){
        switch(value){
            case 1:
                binaryUp();
                break;
            case 2:
                binaryDown();
                break;
            case 3:
                grayUp();
                break;
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
