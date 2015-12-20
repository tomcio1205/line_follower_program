#include <avr/io.h>                        /* Definicje pinów, portów itp. */
//definicja portow do ktorych podlaczone sa czujniki
#define CZ1 PC4
#define CZ2 PC5
#define CZ3 PC6
#define CZ4 PC7
#define CZ5 PA0
#define CZ6 PA1
#define CZ7 PA2
#define CZ8 PA3
#define CZ9 PA4
#define CZ10 PA5
#define CZ11 PA6
#define CZ12 PA7
//definicja portow do sterowania mostkiem
#define AIN1 PC3
#define AIN2 PD7
#define BIN1 PD2
#define BIN2 PD3
#define PW1 PD4
#define PW2 PD5
//defincja portu do czujnika tsop
#define TSOP PD6
//definicja portow do ktorych podlaczone sa ledy
#define LED1 PB4
#define LED2 PB3
#define LED3 PB2
