#include <stm32f1xx.h>

// Variáveis voláteis para interrupção
volatile uint16_t player1 = 0;
volatile uint16_t player2 = 0;
volatile uint32_t count = 0;

// Estados para a máquina de debounce
typedef enum {IDLE, DEBOUNCING, PRESSED} ButtonState;
volatile ButtonState stateP1 = IDLE;
volatile ButtonState stateP2 = IDLE;
volatile uint32_t timerP1 = 0;
volatile uint32_t timerP2 = 0;

void atraso(uint32_t div){
    uint32_t n = 500000 / div;
    while (n--) __asm__("nop");
}

// OBRIGATÓRIO: Função que o hardware chama no clique
void EXTI15_10_IRQHandler(void) {
    // Se interrompeu no pino 11 e estamos em IDLE, começa o debounce
    if (EXTI->PR & (1 << 11)) {
        if (stateP1 == IDLE) {
            stateP1 = DEBOUNCING;
            timerP1 = count; // Salva o "tempo" atual
        }
        EXTI->PR = (1 << 11); // Limpa a bandeira de interrupção
    }
    // O mesmo para o pino 12
    if (EXTI->PR & (1 << 12)) {
        if (stateP2 == IDLE) {
            stateP2 = DEBOUNCING;
            timerP2 = count;
        }
        EXTI->PR = (1 << 12);
    }
}

// Sua função de configuração (ajustada)
void setup_interrupt(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI11_PA;
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI12_PA;
    EXTI->IMR |= (1 << 11) | (1 << 12);
    EXTI->FTSR |= (1 << 11) | (1 << 12); // Borda de descida
    NVIC_EnableIRQ(EXTI15_10_IRQn);
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
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);

    // Saídas PB8 e PB9
    GPIOB->CRH &= ~((0xF << 0) | (0xF << 4));
    GPIOB->CRH |=  ((0x2 << 0) | (0x2 << 4));

    // Entradas PA11 e PA12 com Pull-Up
    GPIOA->CRH &= ~((0xF << 12) | (0xF << 16));
    GPIOA->CRH |=  ((0x8 << 12) | (0x8 << 16));
    GPIOA->ODR |= (1 << 11) | (1 << 12);

    setup_interrupt();

    while(1){
        if(count == 0){
            animate(GPIOB, 8, GPIOB, 9, 0);
            player1 = 0; player2 = 0;
        }

        // --- MÁQUINA DE ESTADOS P1 ---
        if (stateP1 == DEBOUNCING) {
            // Se passaram 50 "counts" e o botão continua em 0, confirma
            if ((count - timerP1) > 50) {
                if (!(GPIOA->IDR & (1 << 11))) {
                    player1++;
                    stateP1 = PRESSED;
                    GPIOB->BSRR = (1 << 8); // Liga LED
                } else {
                    stateP1 = IDLE; // Foi só um ruído
                }
            }
        } else if (stateP1 == PRESSED) {
            if (GPIOA->IDR & (1 << 11)) { // Se soltou o botão
                stateP1 = IDLE;
                GPIOB->BSRR = (1 << 24); // Desliga LED
            }
        }

        // --- MÁQUINA DE ESTADOS P2 ---
        if (stateP2 == DEBOUNCING) {
            if ((count - timerP2) > 50) {
                if (!(GPIOA->IDR & (1 << 12))) {
                    player2++;
                    stateP2 = PRESSED;
                    GPIOB->BSRR = (1 << 9);
                } else {
                    stateP2 = IDLE;
                }
            }
        } else if (stateP2 == PRESSED) {
            if (GPIOA->IDR & (1 << 12)) {
                stateP2 = IDLE;
                GPIOB->BSRR = (1 << 25);
            }
        }

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