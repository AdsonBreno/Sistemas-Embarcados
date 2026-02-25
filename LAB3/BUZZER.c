#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// Endereços dos pinos em PORTC
#define PC_BUTTON_1 1 
#define PC_BUTTON_2 2 
#define PC_BUTTON_3 3 
#define PC_BUTTON_4 4 
#define PC_BUTTON_5 5 

// Endereços dos pinos em PORTD
#define PB_PWM_OUT 1 // Pino D9 (PB1/OC1A)
#define PD_LED_OUT 4 // Pino D4 (Debug)

// Array de notas com os valores OCR1A para Timer 1 (16-bit)
//0d470 = 16M/128(470+1) = 265.39Hz ~ 266.3Hz (C4)
//0d435 = 16M/128(470+1) = 287,35Hz ~ 285,8HZ (D4)
//0d375 = 16M/128(375+1) = 332,44Hz ~ 332,9Hz (E4)
//0d360 = 16M/128(360+1) = 346,26Hz ~ 347,5Hz (F4)
//0d310 = 16M/128(310+1) = 401,92Hz ~ 400,3Hz (G4)
const uint16_t notas_ocr[] = {470, 435, 375, 360, 310};

// ===================================
void PWM_Frequency(){
    // --- TIMER 0 e TIMER 2 (DESLIGADOS) ---
    TCCR0A = 0x00;
    TCCR0B = 0x00;
    TCCR2A = 0x00;
    TCCR2B = 0x00;
    
    // --- TIMER 1 (ATIVO: D9/OC1A - 16 bits) ---
    // MODO: CTC (WGM12=1)
    TCCR1A = 0x00; 
    TCCR1B = 0x08; 
    
    // Ação: Inicia com o Toggle DESLIGADO no OC1A (Pino 9)
    TCCR1A &= ~(0x40); 
    
    // Prescaler 64 (CS11 | CS10) - Inicia o Timer!
    TCCR1B |= 0x03;      
}
// ===================================
void Notes(){
    // IMPORTANTE: current_note_ocr DEVE ser uint16_t (16 bits)
    uint8_t button_found = 0;
    uint16_t current_note_ocr = 0; // CORRIGIDO: Deve ser uint16_t
    uint8_t pin_state = PINC;
    
    // Lógica de Leitura Direta (Buscando LOW = Botão Pressionado)
    if (!(pin_state & (1 << PC_BUTTON_1))) { // C4
        current_note_ocr = notas_ocr[0];
        button_found = 1;
    } 
    else if (!(pin_state & (1 << PC_BUTTON_2))) { // D4
        current_note_ocr = notas_ocr[1];
        button_found = 1;
    } 
    else if (!(pin_state & (1 << PC_BUTTON_3))) { // E4
        current_note_ocr = notas_ocr[2];
        button_found = 1;
    } 
    else if (!(pin_state & (1 << PC_BUTTON_4))) { // F4
        current_note_ocr = notas_ocr[3];
        button_found = 1;
    } 
    else if (!(pin_state & (1 << PC_BUTTON_5))) { 
        current_note_ocr = notas_ocr[4];
        button_found = 1;
    }

    // --- Ação de Controle ---
    
    if (button_found) {
        PORTD |= (1 << PD_LED_OUT); // Liga o LED
        
        // CORREÇÃO CRÍTICA: Atribuir a frequência ao registrador do Timer 1 (OCR1A)
        OCR1A = current_note_ocr; 
        
        // CORREÇÃO CRÍTICA: LIGA O SOM (Toggle no OC1A, pino D9)
        TCCR1A |= 0x40; 
        
    } else {
        PORTD &= ~(1 << PD_LED_OUT); // Desliga o LED
        // CORREÇÃO: DESLIGA O SOM (Toggle no OC1A, pino D9)
        TCCR1A &= ~(0x40);     
    }
}
int main(void){
    
    // Saídas D9 (PWM) e D4 (LED)
    DDRB |= (1 << PB_PWM_OUT); // D9 como SAÍDA
    DDRD |= (1 << PD_LED_OUT); 
    
    // Entradas PC1 a PC5: (Inalterado)
    DDRC &= ~((1 << PC_BUTTON_1) | (1 << PC_BUTTON_2) | (1 << PC_BUTTON_3) | (1 << PC_BUTTON_4) | (1 << PC_BUTTON_5)); 
    PORTC |= (1 << PC_BUTTON_1) | (1 << PC_BUTTON_2) | (1 << PC_BUTTON_3) | (1 << PC_BUTTON_4) | (1 << PC_BUTTON_5);  
    
    // Inicializar Timer 1
    PWM_Frequency(); 
    
    while(1){
        Notes(); 
    }
    return 0;
}