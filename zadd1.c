/*
 * File:   main.c
 * Author: local
 *
 * Created on 15 kwietnia 2026, 10:49
 */

#include "xc.h"
#include "libpic30.h"
#include "stdbool.h"
#include <stdint.h>

// Ustawiamy jedno opóźnienie w main(), więc usuwamy delay z funkcji
// Zmienne 'static' sprawiają, że funkcja pamięta wartość przy kolejnym wywołaniu

void binaryUp(){
    static unsigned char count = 0;
    LATA = count++;
}

void binaryDown(){
    static unsigned char count = 255;
    LATA = count--;
}

void grayUp(){
    static unsigned char count = 0;
    unsigned char gray = count ^ (count >> 1);
    LATA = gray;
    count++;
}

void grayDown(){
    static unsigned char count = 255;
    unsigned char gray = count ^ (count >> 1);
    LATA = gray;
    count--;
}

void bcdUp(){
    static char jednosci = 0;
    static char dziesiatki = 0;
    
    LATA = (dziesiatki << 4) | jednosci;
    
    jednosci++;
    if(jednosci > 9){
        jednosci = 0;
        dziesiatki++;
        if(dziesiatki > 9){
            dziesiatki = 0;
        }
    }//LUB:
    static int count = 0;
    // BCD: (dziesiątki przesunięte o 4 bity w lewo) LUB (jednostki)
    unsigned int bcd = ((count / 10) << 4) | (count % 10);
    LATA = bcd;
    
    count++;
    if (count > 99) count = 0;
}

void bcdDown(){
    static char jednosci = 9;
    static char dziesiatki = 9;
    
    LATA = (dziesiatki << 4) | jednosci;
    
    jednosci--;
    if(jednosci < 0){
        jednosci = 9;
        dziesiatki--;
        if(dziesiatki < 0){
            dziesiatki = 9;
        }
    }//LUB:
    static int count = 99;
    unsigned int bcd = ((count / 10) << 4) | (count % 10);
    LATA = bcd;
    
    count--;
    if (count < 0) count = 99;
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
    static int zapalone = 0;
    static int kropka = 1;
    
    // Dodajemy zapalone diody i tę aktualnie lecącą
    LATA = zapalone + kropka; 
    
    int nastepny_krok = kropka * 2; // Sprawdzamy gdzie kropka będzie za chwilę
    
    // 128 to ostatnia dioda (ósmy bit)
    if (kropka == 128 || (nastepny_krok & zapalone) > 0) {
        zapalone = zapalone + kropka; // Przyklej kropkę do zapalonych
        kropka = 1;                   // Nowa kropka startuje od początku
        
        // 255 to wszystkie 8 diod zapalone
        if (zapalone == 255) {
            zapalone = 0;             // Wyczyść planszę
        }
    } else {
        kropka = kropka * 2;          // Lecimy dalej w lewo
    }
}

void prng(){
    static unsigned char lfsr = 1; // Ziarno, startujemy od 1
    
    LATA = lfsr & 0x3F; // Pokazujemy tylko 6 diod (0x3F to 00111111)
    
    // Sprawdzamy czy najmłodszy bit (skrajny prawy) to 1
    if (lfsr & 1) {
        lfsr = (lfsr >> 1) ^ 0b0111001; // Przesuń o 1 w prawo i zrób XOR z konfiguracją
    } else {
        lfsr = (lfsr >> 1); // Jak to było 0, to po prostu przesuń o 1 w prawo
    }
}

int main(void) {
    int value = 1;
    
    TRISA = 0x0000; // PORTA jako wyjścia (diody)
    TRISD = 0xFFFF; // PORTD jako wejścia (przyciski)
    
    // Inicjalizacja stanów początkowych przycisków przed pętlą (zapobiega to
    // błędnemu "kliknięciu" przy starcie programu)
    char prev6 = PORTDbits.RD6;
    char prev7 = PORTDbits.RD7;
    char current6, current7;
    
    while(1){
        // 1. Wykonanie tylko JEDNEGO kroku animacji na obrót pętli
        switch(value){
            case 1: binaryUp(); break;
            case 2: binaryDown(); break;
            case 3: grayUp(); break;
            case 4: grayDown(); break;
            case 5: bcdUp(); break;
            case 6: bcdDown(); break;
            case 7: snake(); break;
            case 8: queue(); break;
            case 9: prng(); break;
        }
        
        // 2. Globalne opóźnienie definiujące szybkość zmiany klatek
        __delay32(1000000); 
        
        // 3. Odczyt przycisków i detekcja zmiany stanu (zbocza)
        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;
        
        // Warunek na wciśnięcie/puszczenie: jeśli stan się zmienił z 0 na 1
        if(current6 == 1 && prev6 == 0){
            value--;
            if(value < 1){
                value = 9; // Poprawiony zakres dla 8 trybów
            }
        }
        
        if(current7 == 1 && prev7 == 0){
            value++;
            if(value > 9){
                value = 1; // Poprawiony zakres dla 8 trybów
            }
        }
        
        // Zapisanie aktualnego stanu do zmiennych 'prev' na potrzeby następnego obiegu pętli
        prev6 = current6;
        prev7 = current7;
    }
    
    return 0;
}
