/**
 * @mainpage Documentação do Jogo Baja Hill para Nokia 5110
 *
 * @section intro Introdução
 * Este projeto implementa um jogo de rolagem horizontal (side-scrolling)
 * com terreno procedural para um LCD Nokia 5110 em um microcontrolador AVR.
 *
 * @section features Funcionalidades
 * - Geração de terreno procedural usando tabelas de cossenos.
 * - Controle de aceleração e desaceleração.
 * - Desenho de bitmap do baja pixelado.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "nokia5110.h"

/// Frequência principal do clock do microcontrolador (16 MHz).
#define F_CPU 16000000UL

/// @name Parâmetros do Terreno
/// Definição de parâmetros para a criação e renderização do terreno procedural.
/// @{
#define AMPLITUDE 20 ///< Altura máxima do morro (em pixels).
#define OFFSET    35 ///< Base (nível do mar) do terreno na tela.
#define FREQUENCY 4  ///< Frequência do padrão do terreno (menor valor = mais denso).
/// @}

/// @brief Tabela de Cossenos escalonada.
///
/// Tabela precalculada (armazenada em PROGMEM/Flash) para gerar o terreno
/// de forma procedural. Os valores são escalonados de 0 a 255.
const uint8_t COS_TABLE[36] PROGMEM = {
255, 253, 248, 240, 230, 217, 201, 183, 164, 142, 120, 98, 77, 57, 40, 25, 13, 5,
0, 2, 8, 18, 32, 49, 68, 89, 112, 135, 157, 178, 196, 212, 225, 236, 244, 250
};

/// @name Variáveis de Estado Global
/// @{
uint8_t terreno = 0; ///< Posição horizontal atual do terreno (usada para rolagem).
uint8_t move = 0;    ///< Velocidade de movimento atual do jogador (aceleração).
/// @}


/**
 * @brief Calcula a coordenada Y (altura) do terreno para uma dada posição X.
 *
 * Usa a tabela de cossenos precalculada para gerar a altura de forma
 * procedural e aplica a aceleração e o offset vertical (OFFSET).
 *
 * @param x_pos Posição X na tela (coordenada de 0 a 83).
 * @return Retorna a coordenada Y na tela onde o terreno deve começar a ser desenhado.
 */
uint8_t Get_Terrain_Y(uint16_t x_pos) {
    // Acesso correto ao array (não mais 360)
    uint16_t absolute_x = x_pos + terreno; // A coordenada absoluta de cada pixel

    uint8_t table_index = (absolute_x / FREQUENCY) % 36;
    
    // Usando pgm_read_byte para acessar Flash (PROGMEM)
    uint16_t cos_val = pgm_read_byte(&(COS_TABLE[table_index])); 
    
    uint32_t height_scaled = (uint32_t)cos_val * (uint32_t)AMPLITUDE * move / 8;
    // Escala (divide por 256) e aplica a altura
    uint8_t height_pixel = (uint8_t)(height_scaled >> 8); 
    
    return OFFSET - height_pixel;
}

/// @brief Pinos do PORTD mapeados para os botões. 7 para acelerador e 6 para freiar
const uint8_t BUTTON[] = {6,7}; 

/// @brief Bitmap para textura de Lama. Ainda não foi aplicado
const uint16_t lama[] = {
    0b0110100001101000, // Coluna 0
    0b1111110001101000, // Coluna 1
    0b0111101001101000, // ...
    0b0001111101101000,
    0b0001100101101000,
    0b0111101001101000,
    0b1111110001101000,
    0b0110100001101000
};

/// @brief Bitmap do baja
const uint16_t baja[] = {
    0b0000011111000001,
    0b0011101111100011, 
    0b0111110111110001, 
    0b0111110111111010,
    0b0011101111111100,
    0b0000011111111110,
    0b0000011110000010,
    0b0000011110000010,
    0b0000011110000010,
    0b0000011110000010,
    0b0000011110000110,
    0b0011101111111100,
    0b0111110110011000,
    0b0111110110110000,
    0b0011101111100000,
    0b0000011111000000
};

/**
 * @brief Atualiza a velocidade de movimento e o deslocamento do terreno.
 *
 * Lida com a aceleração/desaceleração baseada na leitura do botão de entrada
 * mapeado no pino BUTTON[1]. Segurar o acelerador aumenta a velocidade até o limite definido abaixo. Soltar o acelerador desacelera.
 */
void mover(){
    if(!(PIND & (1 << BUTTON[1]))) {
        move += 1; 
        if (move > 8) move = 10;
    } else {
        if (move > 0) move--;
    }   
    terreno -= move;

}

/**
 * @brief Função principal do programa.
 *
 * Inicializa o LCD Nokia 5110 e entra no loop principal do jogo,
 * lidando com o desenho do terreno, movimento e atualização do score.
 *
 * @note Este código é destinado a microcontroladores AVR.
 * @return Retorna 0 (nunca alcançado no loop infinito de um MCU).
 */
int main(void){
    DDRD &= ~((1 << BUTTON[0]) | (1 << BUTTON[1]));
    PORTD |= ((1 << BUTTON[0]) | (1 << BUTTON[1])); 

    uint8_t u = 3;
    uint8_t j = 1;
    uint8_t rep = 0;
    uint16_t metros = 0;

    const char *text = "Baja Hill";

    nokia_lcd_init();
    
    while(1){
        if(u > 26) j=-1;
        if(u < 3){ 
            j = 1;
            rep++;
        }

        mover();

        if(rep < 4){
            if (rep == 3){
                u = 16;
            }
            nokia_lcd_clear();
            nokia_lcd_set_cursor(u, 0);
            nokia_lcd_write_string(text, 1, 1);
        }
        uint8_t baja_height = 16; // Altura do bitmap 'baja'
        uint8_t baja_in = Get_Terrain_Y(18 + move);

        /// 1. DESENHO PROCEDURAL DO TERRENO (ADICIONADO)
        for(uint8_t x = 0; x < 84; x++) {
            /// Calcula a altura Y para cada coluna, deslocada pelo offset
            uint8_t y_coord = Get_Terrain_Y(x + move);
            
            /// Desenha pixels do y_coord até o final da tela (47)
            for(uint8_t y = y_coord; y < 48; y++) {
                nokia_lcd_set_pixel(x, y, 1); // 1 = Pixel ligado (Montanha)
            uint8_t y_coord = Get_Terrain_Y(x + terreno);

                for(uint8_t y = y_coord; y < 48; y++) {
                    if ((y % 4 == 0) && (x % 3 == 0)) { // Exemplo: padrão x, y
                        nokia_lcd_set_pixel(x, y, 0); /// Apaga (Buraco / Textura)
                    } else {
                        nokia_lcd_set_pixel(x, y, 1); // Liga
                    }
                }                      
            }
        }

        metros += abs(move);
        nokia_lcd_draw_bitmap(10, baja_in - baja_height, baja, 16, 16);
        /// Escreve o valor do score
        nokia_lcd_set_cursor(25, 40);
        nokia_lcd_write_score(metros);
        nokia_lcd_set_cursor(45, 40);
        nokia_lcd_write_string("m", 1, 1);
        nokia_lcd_render();
        u = u + j;
        _delay_ms(75);

    }
}