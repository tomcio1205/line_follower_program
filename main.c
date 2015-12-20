#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "conf.c"
#include <avr/interrupt.h>
#include <stdlib.h>
//AIN2 & BIN2 - jazda do przodu
//AIN1 & BIN1 - jazda do tylu
//AIN1 & BIN2 - skret w prawo
//AIN2 & BIN1 - skret w lewo
volatile int petla = 0, licznik = 0;
//int czujniki[12]={1,2,3,4,5,6,7,8,9,10,11,12};
int wagi[12] = {-60, -50, -30, -20, -10, 0, 0, 10, 20, 30, 50, 60};
int czujniki[12];
int stop [12] = {1,1,1,1,1,1,1,1,1,1,1,1};
int k = 0;
int z = 0;
int blad, pop_blad, Kp = 3.6, Kd = 0;
int prev_error = 0;
int przestrz = 0;
void PWM(int,int);
int PD();
int licz_blad();
void czytaj_czujniki();

int main(void)
{
	//definicja portow do ktorych podlaczone sa czujniki jako wejscia
	DDRC &= ~ (1<<CZ1) | (1<<CZ2) | (1<<CZ3) | (1<<CZ4);
	DDRA &= ~ (1<<CZ5) | (1<<CZ6) | (1<<CZ7) | (1<<CZ8) | (1<<CZ9) | (1<<CZ10) | (1<<CZ11) | (1<<CZ12);
	//wlaczenie pull-upow
	PORTC |= (1<<CZ1) | (1<<CZ2) | (1<<CZ3) | (1<<CZ4);
	PORTA |= (1<<CZ5) | (1<<CZ6) | (1<<CZ7) | (1<<CZ8) | (1<<CZ9) | (1<<CZ10) | (1<<CZ11) | (1<<CZ12);
	//definicja portow do sterowania mostkiem jako wyjscia
	DDRD |= (1<<AIN2) | (1<<BIN1) | (1<<BIN2) |(1<<PW1) | (1<<PW2);
	DDRC |= (1<<AIN1) ;
	//defincja portu do czujnika tsop jako wejscie
	DDRD &= ~ (1<<TSOP);
	//definicja portow do ktorych podlaczone sa ledy jako wyjscia
	DDRB  |= (1<<LED1) | (1<<LED2) | (1<<LED3) ;

	//  ustawiamy timer1 w tryb fast pwm 8bit, preskaler 64
    TCCR1A |= (1<<WGM10);
	TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS11);
	//  Clear OC1A/OC1B on compare match, set OC1A/OC1B at BOTTOM
	TCCR1A |= (1<<COM1A1) | (1<<COM1B1);
	//dla obu kanalow wartosc poczatkowa ustawina na 0
    OCR1A = 0;
    OCR1B = 0;
    //włączamy timer0 w tryb ctc do generowania przerwań, ustawiamy preskaler 1024
    TCCR0 |= (1<<WGM01) | (1<<CS00) | (1<< CS02);
    //ustawiamy czestotliwosc na 100 Hz 16MHz/1024/156
    OCR0 = 156;
    //włączamy przerwanie po osiągnięciu zadanej wartosci
    TIMSK |= (1<<OCIE0);
    // uruchamiamy globalną obsługę przerwań
    sei();
    // globalna flaga informująca o konieczności wywołania pętli algorytmu

   while (1) //Pętla główna
   {
	   z = 0;
	   czytaj_czujniki();
	   blad = licz_blad();
	   int regulacja = PD();
	   PWM(80 + regulacja, 80 - regulacja);
	   for (int i = 0; i<12 ; i++)
	   {
		   k = czujniki[i]*stop[i];
		   z = z+k;
		   i++;
	   }
	   if (z==0)
	   {
		   PWM(0,0);
	   }
       PORTB |= (1<<LED2);
       PORTB |= (1<<LED3);

       if (petla)
       {
    	   PORTB ^= (1<<LED1);
    	   petla = 0;
       }

    }
}
ISR(TIMER0_COMP_vect)
{
	licznik++;
	if(licznik == 100)
	{
		licznik = 0;
		petla = 1;
	}
}

void PWM(int lewy, int prawy)
{
    if(lewy >= 0)
    {
        if(lewy>255)
            lewy = 255;
        PORTD |= (1<<AIN2);
        PORTC &= ~ (1<<AIN1);
    }
    else
    {
        if(lewy<-255)
            lewy = -255;
        PORTD &= ~ (1<<AIN2);
        PORTC |= (1<<AIN1);
    }

    if(prawy >= 0)
    {
        if(prawy>255)
            prawy = 255;
 	   PORTD |= (1<<BIN2);
	   PORTD &= ~ (1<<BIN1);
    }
    else
    {
        if(prawy<-255)
            prawy = -255;
  	   PORTD &= ~ (1<<BIN2);
 	   PORTD |= (1<<BIN1);
    }

    OCR1A = abs(lewy);
    OCR1B = abs(prawy);
}
void czytaj_czujniki()
{
	czujniki[0] = PINC & (1<<CZ1);
	czujniki[1] = PINC & (1<<CZ2);
	czujniki[2] = PINC & (1<<CZ3);
	czujniki[3] = PINC & (1<<CZ4);
	czujniki[4] = PINA & (1<<CZ5);
	czujniki[5] = PINA & (1<<CZ6);
	czujniki[6] = PINA & (1<<CZ7);
	czujniki[7] = PINA & (1<<CZ8);
	czujniki[8] = PINA & (1<<CZ9);
	czujniki[9] = PINA & (1<<CZ10);
	czujniki[10] = PINA & (1<<CZ11);
	czujniki[11] = PINA & (1<<CZ12);

}
int licz_blad()
{
	int error = 0;
	int ilosc = 0;
	//int waga = 10;

   /* if (przestrz)
    {
    	waga = 5;
    }*/

    for(int i=0; i<12; i++)
    {
        error += czujniki[i]*wagi[i];        // wagi kolejnych czujników (dla i z zakresu [0;7]): -30, -20, -10, 0, 10, 20, 30
        //error += czujniki[i]*(i-3)*waga;
    	ilosc += czujniki[i];            // czujniki[i] ma wartość 1/0
    }

    if (ilosc != 0)
    {
        error /= ilosc;                        // liczymy średnią wagę;
        prev_error = error;
    }
    else
    	error = prev_error;
    /*	if(prev_error < -40)
    	{
    		error = -70;
    		//przestrz = 1;
    	}
    	else if (prev_error > 40)
    	{
    		error = 70;
    	//	przestrz = 2;
    	}
    	else
    		error = 0;
    }
   /* if(przestrz == 1 && error >= 0)        // zerowanie flagi przestrzelenia zakrętu po powrocie na środek linii
        przestrz = 0;
    else if(przestrz == 2 && error <= 0)
        przestrz = 0;*/

    return error;
}
int PD()
{
    //zmienna blad zawiera aktualny wynik fukcji licz_blad()
    int rozniczka = blad - pop_blad;
    pop_blad = blad;
    return Kp*blad + Kd*rozniczka;
}

