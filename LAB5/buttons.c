#include <stm32f1xx.h>

uint16_t player1 = 0;
uint16_t player2 = 0;
uint32_t count = 0;
uint8_t p1_last_state = 1; 
uint8_t p2_last_state = 1;

void atraso(uint32_t div){
    uint32_t n = 500000 / div;
    while (n--) __asm__("nop");
}

void animate(GPIO_TypeDef* GPIOx, uint16_t pino, GPIO_TypeDef* GPIOy, uint16_t pino2, const int time){
    if (time) {    //Animação de Vitória
        for (uint8_t i = 0; i < 3; i++){        
            GPIOx->BSRR = (1 << pino);
            atraso(5);
            GPIOx->BSRR = (1 << (pino + 16)); 
            GPIOy->BSRR = (1 << pino2);          // Liga
            atraso(5);
            GPIOy->BSRR = (1 << (pino2 + 16));   // Desliga
        }
    } else {
        for (uint8_t i = 0; i < 5; i++){        
            GPIOx->BSRR = (1 << pino);
            GPIOy->BSRR = (1 << pino2);          // Liga
            atraso(8);
            GPIOx->BSRR = (1 << (pino + 16)); 
            GPIOy->BSRR = (1 << (pino2 + 16));   // Desliga
        }
    }
}

int main(void){
    // 1. Habilita Clock nos Portos A e B (Barramento APB2 na F1)
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);

    // 2. Configura PB8 e PB9 como Saída (Push-Pull 2MHz)
    // Usamos CRH pois são os pinos 8 e 9. Modo 0x2 é Saída.
    GPIOB->CRH &= ~((0xF << 0) | (0xF << 4)); // Limpa bits 0-3 (PB8) e 4-7 (PB9)
    GPIOB->CRH |=  ((0x2 << 0) | (0x2 << 4)); // Configura como Saída

    // 3. Configura PA11 e PA12 como Entrada com Pull-Up
    // Usamos CRH pois são pinos 11 e 12. Modo 0x8 é Input Pull-up/down.
    GPIOA->CRH &= ~((0xF << 12) | (0xF << 16)); // Limpa bits dos pinos 11 e 12
    GPIOA->CRH |=  ((0x8 << 12) | (0x8 << 16)); // Define como Entrada
    GPIOA->ODR |= (1 << 11) | (1 << 12);        // Ativa o Resistor de Pull-Up

    while(1){      
        if(count == 0){
          // Animação inicial simplificada para teste
          animate(GPIOB, 8, GPIOB, 9, 0);
          player1 = 0; player2 = 0;
        }

        // Leitura Player 1 (PA11)
        uint8_t p1_current = (GPIOA->IDR & (1 << 11)) ? 1 : 0;
        if (p1_last_state == 1 && p1_current == 0) {
            player1++;
            GPIOB->BSRR = (1 << 8); // Acende LED PB8
            atraso(200); 
        } 
        if (p1_current == 1) GPIOB->BSRR = (1 << 24); // Apaga ao soltar
        p1_last_state = p1_current;

        // Leitura Player 2 (PA12)
        uint8_t p2_current = (GPIOA->IDR & (1 << 12)) ? 1 : 0;
        if (p2_last_state == 1 && p2_current == 0) {
            player2++;
            GPIOB->BSRR = (1 << 9); // Acende LED PB9
            atraso(200); 
        }
        if (p2_current == 1) GPIOB->BSRR = (1 << 25); // Apaga ao soltar
        p2_last_state = p2_current;
        
        count++;
        if (count == 1200000){
            GPIOB->BSRR = (1 << (8 + 16)); // Desliga
            GPIOB->BSRR = (1 << (9 + 16)); // Desliga
            atraso(10);
            if (player1 < player2){
                animate(GPIOB, 7, GPIOB, 9, 1);
            }
            if (player2 < player1){
                animate(GPIOB, 8, GPIOB, 7, 1);
            } else {
                GPIOB->BSRR = (1 << 8);
                GPIOB->BSRR = (1 << 9);
                atraso(10);
                GPIOB->BSRR = (1 << 8 + 16);
                GPIOB->BSRR = (1 << 9 + 16);                
            }
            atraso(5);
            animate(GPIOB, 8, GPIOB, 9, 0);
            count = 0;
        }
    }
}