#include <xc.h>
#include "libpic30.h"
#include "lcd.h"
#include "adc.h"

unsigned char prev6 = 1, curr6 = 1; // Gracz 1
unsigned char prev7 = 1, curr7 = 1; // Gracz 2

// Zmienne zegara
unsigned int czas_g1 = 0;
unsigned int czas_g2 = 0;
unsigned int nastawa_czasu = 0; // W sekundach

// 0 - Ustawianie czasu, 1 - Odlicza G1, 2 - Odlicza G2, 3 - Koniec gry
unsigned char stan_gry = 0; 

void init(void)
{
    // Konfiguracja przycisków RD6 i RD7 na wejście
    TRISD |= (1 << 6) | (1 << 7);
    
    // Inicjalizacja modułów
    LCD_Initialize();
    
    // Inicjalizacja potencjometru
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
    
    // Linia 1: Czas Gracza 1
    LCD_PutString("G1: ", 4);
    LCD_PutChar('0' + (mm1 / 10));
    LCD_PutChar('0' + (mm1 % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss1 / 10));
    LCD_PutChar('0' + (ss1 % 10));
    if(stan_gry == 1) LCD_PutString(" <-", 3); // Znacznik kto teraz gra

    LCD_PutChar('\n');

    // Linia 2: Czas Gracza 2
    LCD_PutString("G2: ", 4);
    LCD_PutChar('0' + (mm2 / 10));
    LCD_PutChar('0' + (mm2 % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss2 / 10));
    LCD_PutChar('0' + (ss2 % 10));
    if(stan_gry == 2) LCD_PutString(" <-", 3); // Znacznik kto teraz gra
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
        // --- TRYB 0: USTAWIENIE CZASU I CZEKANIE NA START ---
        if(stan_gry == 0)
        {
            unsigned long adc_val = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
            if(adc_val != 0xFFFF)
            {
                unsigned int nowa_nastawa = 60; // domyślnie 1 min
                
                // Dzielimy zakres 0-1023 na 3 strefy
                if(adc_val > 341 && adc_val <= 682) nowa_nastawa = 180; // 3 min
                else if(adc_val > 682) nowa_nastawa = 300; // 5 min
                
                // Aktualizujemy tylko gdy wartość uległa zmianie
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

            // Jeśli Gracz 1 wcisnął przycisk (zrobił ruch), startuje zegar Gracza 2
            if(curr6 == 1 && prev6 == 0)
            {
                stan_gry = 2;
                wyswietlCzas();
            }
            // Jeśli Gracz 2 wcisnął przycisk, startuje zegar Gracza 1
            else if(curr7 == 1 && prev7 == 0)
            {
                stan_gry = 1;
                wyswietlCzas();
            }
        }
        
        // --- TRYB 1 i 2: TRWA GRA ---
        else if(stan_gry == 1 || stan_gry == 2)
        {
            int i;
            unsigned char zmiana_gracza = 0;
            
            // Pętla odliczająca ~1 sekundę podzielona na 20 części, 
            // aby błyskawicznie rejestrować wciśnięcie przycisku szachowego
            for(i = 0; i < 20; i++)
            {
                prev6 = PORTDbits.RD6;
                prev7 = PORTDbits.RD7;
                
                __delay32(50000); 
                
                curr6 = PORTDbits.RD6;
                curr7 = PORTDbits.RD7;

                // Gracz 1 zakończył ruch -> przełącz na Gracza 2
                if(stan_gry == 1 && curr6 == 1 && prev6 == 0)
                {
                    stan_gry = 2;
                    zmiana_gracza = 1;
                    break; // Przerwij odliczanie sekundy
                }
                // Gracz 2 zakończył ruch -> przełącz na Gracza 1
                if(stan_gry == 2 && curr7 == 1 && prev7 == 0)
                {
                    stan_gry = 1;
                    zmiana_gracza = 1;
                    break; // Przerwij odliczanie sekundy
                }
            }
            
            // Jeśli pętla została przerwana wciśnięciem, tylko odświeżamy ekran z nowym stanem
            if(zmiana_gracza == 1)
            {
                wyswietlCzas();
                continue;
            }

            // Jeśli pętla doszła do końca (minęła pełna sekunda gry bez kliknięcia)
            if(stan_gry == 1)
            {
                czas_g1--;
                if(czas_g1 == 0) stan_gry = 3; // Koniec czasu G1
            }
            else if(stan_gry == 2)
            {
                czas_g2--;
                if(czas_g2 == 0) stan_gry = 3; // Koniec czasu G2
            }
            
            // Odświeżenie lub koniec
            if(stan_gry == 3)
            {
                // Przekazujemy do funkcji, który gracz miał zero na zegarze
                if(czas_g1 == 0) wyswietlKoniec(1);
                else wyswietlKoniec(2);
            }
            else
            {
                wyswietlCzas();
            }
        }
        
        // --- TRYB 3: KONIEC GRY (ktoś przegrał przez czas) ---
        else if (stan_gry == 3)
        {
            prev6 = PORTDbits.RD6;
            prev7 = PORTDbits.RD7;
            
            __delay32(50000); 
            
            curr6 = PORTDbits.RD6;
            curr7 = PORTDbits.RD7;
            
            // Dowolny przycisk resetuje zegar do menu startowego
            if((curr6 == 1 && prev6 == 0) || (curr7 == 1 && prev7 == 0))
            {
                stan_gry = 0;
                nastawa_czasu = 0; // Wymusi to odświeżenie ekranu w trybie 0
            }
        }
    }

    return 0;
}
