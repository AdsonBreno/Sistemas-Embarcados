#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

int main(void){
    DDRC = 0b0111111;  //Habilita as portas C0~C5 como saídas
    DDRD &= ~(1<<PD0);     //Habilita D0 como entrada;
    DDRB |= (1<<1);  //Habilita B1 como saída;
    PORTD |= (1<<PD0);    //Pull-up em todas as portas D habilitadas

    uint8_t num = 0;    //Int de 8 bits;
    uint8_t NUM[10] = {0x3E,0x18,0x35,0x39,0x1B,0x2B,0x2F,0x38,0x3F,0x3B}; //Vetor de 8 bits com 10 itens

    while(1){
        PORTC = NUM[num];
        
        if((PIND & (1<<0)) == 0){
            num++;
            while((PIND & (1<<0)) == 0);
        } else if(num == 10){
            num = 0;
            _delay_ms(100);
            PORTC = NUM[num];
        }
        
    }
}