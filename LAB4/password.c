#include <stm32f1xx.h>

uint8_t senha_mestra[3] = {0, 0, 0};
uint8_t entrada_usuario[3] = {0, 0, 0};
uint8_t indice_senha = 0;
uint8_t modo_gravacao = 0; 
uint8_t p_last[3] = {1, 1, 1}; 

void atraso(uint32_t div){
    uint32_t n = 500000 / div;
    while (n--) __asm__("nop");
}

void blink(GPIO_TypeDef* GPIOx, uint16_t pino) {
    for (uint8_t i = 0; i < 3; i++){
        GPIOx->BSRR = (1 << pino);          
        atraso(5);
        GPIOx->BSRR = (1 << (pino + 16));   
        atraso(5);
    }
}

int main(void){
    // 1. Clock para GPIOA e GPIOB
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);

    // 2. Configura LEDs (PB8 e PB9) como Saída (CRH)
    GPIOB->CRH &= ~((0xF << 0) | (0xF << 4)); 
    GPIOB->CRH |=  ((0x2 << 0) | (0x2 << 4)); 
    GPIOB->BSRR = (1 << 24) | (1 << 25); // Garante início apagado

    // 3. Configura Botões nos pinos PA0, PA1, PA2 e PA5 (CRL)
    // Na F1, CRL cuida dos pinos 0 até 7. Cada pino usa 4 bits.
    // Modo 0x8 = Entrada com Pull-up/down
    GPIOA->CRL &= ~((0xF << 0) | (0xF << 4) | (0xF << 8) | (0xF << 20)); // Limpa PA0, PA1, PA2, PA5
    GPIOA->CRL |=  ((0x8 << 0) | (0x8 << 4) | (0x8 << 8) | (0x8 << 20)); // Seta como Input Pull

    // 4. Ativa resistores de PULL-UP escrevendo 1 no ODR
    GPIOA->ODR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 5);

    atraso(10);

    while(1){
        // --- BOTÃO DE GRAVAÇÃO (PA5) ---
        if (!(GPIOA->IDR & (1 << 5))) {
            atraso(50); 
            if (!(GPIOA->IDR & (1 << 5))) {
                modo_gravacao = 1;
                indice_senha = 0;
                GPIOB->BSRR = (1 << 8) | (1 << 9); 
                while (!(GPIOA->IDR & (1 << 5))); 
                atraso(15); 
            }
        }

        // --- CAPTURA DOS BOTÕES (PA0, PA1, PA2) ---
        uint8_t pinos[3] = {0, 1, 2};
        for (uint8_t i = 0; i < 3; i++) {
            uint8_t estado_atual = (GPIOA->IDR & (1 << pinos[i])) ? 1 : 0;
            
            if (p_last[i] == 1 && estado_atual == 0) { 
                atraso(250); 
                if (modo_gravacao) {
                    senha_mestra[indice_senha++] = i + 1;
                } else {
                    entrada_usuario[indice_senha++] = i + 1;
                }
                while (!(GPIOA->IDR & (1 << pinos[i]))); 
                atraso(250); 
            }
            p_last[i] = estado_atual;
        }

        if (indice_senha == 3) {
            if (modo_gravacao) {
                modo_gravacao = 0;
                GPIOB->BSRR = (1 << 8 + 16) | (1 << 9 + 16); //Desliga os LEDs 
            } else {
                uint8_t acertos = 0;
                for(int j=0; j<3; j++) {
                    if(entrada_usuario[j] == senha_mestra[j]) acertos++;
                }
                if (acertos == 3) blink(GPIOB, 8); 
                else blink(GPIOB, 9); 
            }
            indice_senha = 0; 
        }
    }
}
/*
//Código para o Wokwi

#include <stm32c0xx.h>

// Vetores para as senhas
uint8_t senha_mestra[3] = {0, 0, 0};
uint8_t entrada_usuario[3] = {0, 0, 0};
uint8_t indice_senha = 0;
uint8_t modo_gravacao = 0; // 0 = Verificação, 1 = Gravação

// Estados para detecção de borda (Debounce)
uint8_t p_last[4] = {1, 1, 1, 1}; // Botões PA10, PA11, PA12 e PA5

void atraso(uint32_t div){
    uint32_t n = 5000000 / div;
    while (n--){
    __asm__("nop"); // No Operation — só para gastar tempo
    }
}

// GPIO_TypeDef é a estrutura que contém MODER, ODR, BSRR, etc.
void blink(GPIO_TypeDef* GPIOx, uint16_t pino) {
    for (uint8_t i = 0; i < 3; i++){
        GPIOx->BSRR = (1 << pino);          // Liga
        atraso(5);
        GPIOx->BSRR = (1 << (pino + 16));   // Desliga
    }
}

int main(void){
    // 1. Clock para GPIOA e GPIOB
    RCC->IOPENR |= (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN);

    // 2. LEDs: PB8 (Verde) e PB9 (Vermelho) como Saída
    GPIOB->MODER &= ~((3 << 2 * 8) | (3 << 2 * 9));
    GPIOB->MODER |=  ((1 << 2 * 8) | (1 << 2 * 9));

    // 3. Botões: PA10, PA11, PA12 (Senhas) e PA5 (Gravar) como Entrada Pull-up
    GPIOA->MODER &= ~((3 << 2 * 10) | (3 << 2 * 11) | (3 << 2 * 12) | (3 << 2 * 5));
    GPIOA->PUPDR |=  ((1 << 2 * 10) | (1 << 2 * 11) | (1 << 2 * 12) | (1 << 2 * 5));

    while(1){
        // --- LÓGICA DO BOTÃO DE GRAVAÇÃO (PA5) ---
        // Se segurar PA5, entra no modo de definir nova senha
        if (!(GPIOA->IDR & (1 << 5))) {
            atraso(5); // Debounce simples
            modo_gravacao = 1;
            indice_senha = 0;
            GPIOB->BSRR = (1 << 8) | (1 << 9); // Ambos acendem para avisar: "GRAVANDO"
        }

        // --- CAPTURA DOS BOTÕES (PA10, PA11, PA12) ---
        uint8_t pinos[3] = {10, 11, 12};
        for (uint8_t i = 0; i < 3; i++) {
            uint8_t estado_atual = (GPIOA->IDR & (1 << pinos[i])) ? 1 : 0;
            
            // Se detectou a borda de descida (clique)
            if (p_last[i] == 1 && estado_atual == 0) { 
                atraso(200); // Debounce inicial
                
                // SALVA O DÍGITO
                if (modo_gravacao) {
                    senha_mestra[indice_senha++] = i + 1;
                } else {
                    entrada_usuario[indice_senha++] = i + 1;
                }

                // --- NOVA TRAVA: Espera o usuário soltar o botão ---
                // Enquanto o pino estiver em 0 (apertado), o código fica "preso" aqui
                while (!(GPIOA->IDR & (1 << pinos[i]))); 
                atraso(200); // Debounce ao soltar
            }
            p_last[i] = estado_atual;
        }

        // --- VERIFICAÇÃO ---
        if (indice_senha == 3) {
            if (modo_gravacao) {
                // Fim da gravação
                modo_gravacao = 0;
                GPIOB->BSRR = (1 << 24) | (1 << 25); // Apaga luzes de gravação
            } else {
                // Checa se a senha bate
                uint8_t acertos = 0;
                for(int j=0; j<3; j++) {
                    if(entrada_usuario[j] == senha_mestra[j]) acertos++;
                }

                if (acertos == 3) {
                    blink(GPIOB, 8); // Verde
                } else {
                    blink(GPIOB, 9); // Vermelho
                }
            }
            indice_senha = 0; // Reinicia para a próxima tentativa
        }
    }
}*/