#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL

volatile uint8_t display_1 = 0;    //Registra o primeiro display em 8 bits;
volatile uint8_t display_2 = 0;    //Registra o segundo display em 8 bits;
volatile uint8_t NUM[10] = {0x7E,0x30,0x6D,0x79,0x33,0x5B,0x5F,0x70,0x7F,0x7B}; //Vetor de 8 bits com 10 itens
volatile uint8_t is_debouncing = 0;

ISR(TIMER1_COMPA_vect){
    TCCR1B = 0;
    if(is_debouncing = 1){
        if((PINC & (1 << PC0)) == 0){
            display_2++;

            if((display_2 % 10) == 0){
                display_2 = 0;
                display_1++;
                if(display_1 == 10){
                    display_2 = 0;
                    display_1 = 0;
                }
            } 
        }
        is_debouncing = 2;
    } else {
        is_debouncing = 0;
    }                                                                                                                                                                                                             
    _delay_ms(75);
}

int main(void){
    DDRD = 0xFF;            //Habilita as portas D0~D6 como saídas
    DDRB = 0x06;            //Habilita 00000110
    DDRC &= ~(1<<PD0);      //Habilita C0 como entrada;
    PORTC |= (1<<PD0);      //Pull-up em todas as portas D habilitadas

    TCCR1A = 0;
    TCCR1B = 0;
    
    // 2. Define o valor de comparação para 50ms
    // (16,000,000 / 1024) * 0.05s = 781.25
    OCR1A = 600;
    
    // 3. Habilita a interrupção de Comparação A
    TIMSK1 |= (1 << OCIE1A);

    sei();
    
    while(1){
        if(((PINC & (1 << PC0)) == 0) && (is_debouncing == 0)){
            // 1. Muda o estado para "Debounce"
            is_debouncing = 1;
            
            // 2. LIGA o Timer1 (Modo CTC, Prescaler 1024)
            TCNT1 = 0; // Zera o contador do timer
            TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
        }
        if(((PINC & (1 << PC0)) != 0) && (is_debouncing == 2)){
            // Reseta o estado para "Pronto"
            is_debouncing = 0;
        }

        PORTD = NUM[display_1];
        PORTB = 0x2; //Ativa o primeiro e desativa o segundo display
        _delay_ms(2);
        PORTB = 0x4; //Ativa o segundo e desativa o primeiro display
        PORTD = NUM[display_2];
        _delay_ms(2);
    }
} 