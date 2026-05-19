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
 
// Zmienne globalne
unsigned int powerIndex = 0;
unsigned int powerLevels[4] = {800, 600, 350, 200};

unsigned long czas = 0; 
unsigned char dziala = 0; // 0 - Stop, 1 - Gotowanie

void init(void)
{
    AD1PCFG = 0xFFFF; // Wszystkie piny analogowe jako cyfrowe
    
    TRISA = 0x0000;   // Port A na wyjscie - diody
    LATA = 0x0000;
    
    // Konfiguracja przycisków RB2, RB3, RB4, RB5 na wejście
    TRISD |= (1 << 6) | (1 << 7) | (1 << 13);
    TRISAbits.TRISA7 = 1;

    LCD_Initialize();
}

void wyswietl(void)
{
    unsigned int mm = czas / 60;
    unsigned int ss = czas % 60;

    LCD_ClearScreen();
    LCD_PutString("Moc: ", 5);

    if(powerLevels[powerIndex] == 800) LCD_PutString("800W", 4);
    else if(powerLevels[powerIndex] == 600) LCD_PutString("600W", 4);
    else if(powerLevels[powerIndex] == 350) LCD_PutString("350W", 4);
    else LCD_PutString("200W", 4);

    LCD_PutChar('\n');

    LCD_PutString("Czas: ", 6);
    LCD_PutChar('0' + (mm / 10));
    LCD_PutChar('0' + (mm % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss / 10));
    LCD_PutChar('0' + (ss % 10));
}

void wyswietlGotowanie(void)
{
    unsigned int mm = czas / 60;
    unsigned int ss = czas % 60;

    LCD_ClearScreen();
    LCD_PutString("Gotowanie...", 12);
    LCD_PutChar('\n');

    LCD_PutChar('0' + (mm / 10));
    LCD_PutChar('0' + (mm % 10));
    LCD_PutChar(':');
    LCD_PutChar('0' + (ss / 10));
    LCD_PutChar('0' + (ss % 10));
}

int main(void)
{
    init();
    wyswietl();

    // Zmienne do stanów przycisków
    unsigned char pop_stan_rb5 = 1, stan_rb5 = 1;
    unsigned char pop_stan_rb4 = 1, stan_rb4 = 1;
    unsigned char pop_stan_rb3 = 1, stan_rb3 = 1;
    unsigned char pop_stan_rb2 = 1, stan_rb2 = 1;

    while(1)
    {
        // Tryb ustawiania kuchenki
        if(dziala == 0)
        {
            poprzedni_rd6 = PORTDbits.RD6;
            poprzedni_rd7 = PORTDbits.RD7;
            poprzedni_ra7 = PORTAbits.RA7;
            poprzedni_rd13 = PORTDbits.RD13;

            __delay32(50000); // prosty debouncing z wzoru

            stan_rd6 = PORTDbits.RD6;
            stan_rd7 = PORTDbits.RD7;
            stan_ra7 = PORTAbits.RA7;
            stan_rd13 = PORTDbits.RD13;

            // Zmiana mocy (RB5)
            if(poprzedni_rd6 == 1 && stan_rd6 == 0)
            {
                powerIndex++;
                if(powerIndex > 3) powerIndex = 0;
                wyswietl();
            }

            // Dodaj 1 minutę (RB4)
            if(poprzedni_rd7 == 1 && stan_rd7 == 0)
            {
                czas += 60;
                wyswietl();
            }

            // Dodaj 10 sekund (RB3)
            if(poprzedni_ra7 == 1 && stan_ra7 == 0)
            {
                czas += 10;
                wyswietl();
            }

            // Start (RB2)
            if(poprzedni_rd13 == 1 && stan_rd13 == 0)
            {
                if(czas > 0)
                {
                    dziala = 1;
                    wyswietlGotowanie();
                }
            }
        }
        else 
        {
            poprzedni_rd13 = PORTDbits.RD13;
            
            __delay32(1000000);
            
            stan_rd13 = PORTDbits.RD13;

            // Przerwanie gotowania (RB2)
            if(poprzedni_rd13 == 1 && stan_rd13 == 0)
            {
                dziala = 0;
                wyswietl();
                continue; 
            }

            // Odliczanie czasu
            if(czas > 0)
            {
                czas--;
                wyswietlGotowanie();
            }

            // Zakończenie pracy
            if(czas == 0)
            {
                dziala = 0;

                LCD_ClearScreen();
                LCD_PutString("Koniec!", 7);
                LCD_PutChar('\n');
                LCD_PutString("00:00", 5);

                // Mruganie diodami na koniec pracy
                
                for(int i = 0; i < 6; i++)
                {
                    LATA = 0x00FF; // Zapala 8 diod dla lepszego efektu
                    __delay32(300000);
                    LATA = 0x0000;
                    __delay32(300000);
                }

                wyswietl(); // Powrót do menu startowego
            }
        }
    }

    return 0;
}
