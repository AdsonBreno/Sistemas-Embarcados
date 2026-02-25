#include <avr/io.h>
#include <util/delay.h>

#define LUX_GAIN_A  50      // Fator de ajuste A_int
#define LUX_SHIFT   12      // Usado para dividir por 1024 (2^10)
#define F_CPU 16000000UL//Frequência de trabalho da CPU
#define BAUD 9600//Taxa da UART
#define MYUBRR F_CPU/16/BAUD-1

// Protótipos das funções ADC (necessário se definidas após main)
void ADC_Init(void);
uint16_t ADC_Read(void);

// Função para TRANSMITIR o valor LUX (ajustado para até 9999)
void UART_Transmit_LUX(uint16_t num) {
    UART_Transmit((num / 1000) % 10 + '0'); // Milhar
    UART_Transmit((num / 100) % 10 + '0');  // Centena
    UART_Transmit((num / 10) % 10 + '0');   // Dezena
    UART_Transmit(num % 10 + '0');          // UnidadeMilhar
}

// === NOVA FUNÇÃO DE CONVERSÃO COM MATEMÁTICA DE INTEIROS ===

uint16_t ADC_to_LUX(uint16_t adc_value){
    uint32_t adc_squared = (uint32_t)adc_value * (uint32_t)adc_value;
    uint32_t numerator;
    uint32_t lux_result;
    
    if(adc_value <= 400){
        numerator = adc_squared * (LUX_GAIN_A-35);     //Aplica o Ganho (A_int)
        lux_result = numerator >> LUX_SHIFT;     //Aplica a Divisão (SHIFT - Bitwise Right Shift)
    } else if(adc_value <= 550){
        numerator = adc_squared * (LUX_GAIN_A-20);     
        lux_result = numerator >> LUX_SHIFT;     
    } else if(adc_value <= 650){
        numerator = adc_squared * (LUX_GAIN_A-10);
        lux_result = numerator >> LUX_SHIFT;    
    } else if(adc_value <= 690){
        numerator = adc_squared * (LUX_GAIN_A-2);
        lux_result = numerator >> LUX_SHIFT;    
    } else if(adc_value <= 730){
        numerator = adc_squared * (LUX_GAIN_A+5);
        lux_result = numerator >> LUX_SHIFT;    
    } else{
        numerator = adc_squared * (LUX_GAIN_A+13);
        lux_result = numerator >> LUX_SHIFT;    
    }
    
    return (uint16_t)lux_result;
}

// ||Função para inicialização da USART||
void UART_Init(uint16_t ubrr){
    UBRR0H = (uint8_t)(ubrr>>8); //Ajusta a taxa de transmissão
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0); //Habilita o transmissor e o receptor
    UCSR0C = (1<<USBS0)|(3<<UCSZ00); //Ajusta o formato do frame: 8 bits de dados e 2 de parada
}

// ||Função para envio de um frame de 5 a 8bits||
void UART_Transmit(uint8_t data)
{
    while(!( UCSR0A & (1<<UDRE0)));//Espera a limpeza do registrador de transmissão
    UDR0 = data; //Coloca o dado no registrador e o envia
}

// Função para envio de uma STRING (necessário para mensagens de debug)
void UART_Transmit_String(const char *s){
    while(*s) {
        UART_Transmit(*s++);
    }
}

// Função para TRANSMITIR o valor ADC (sempre 4 dígitos)
void UART_Transmit_Number(uint16_t num) {
    UART_Transmit((num / 1000) % 10 + '0'); 
    UART_Transmit((num / 100) % 10 + '0');  
    UART_Transmit((num / 10) % 10 + '0');   
    UART_Transmit(num % 10 + '0');          
}

// === FUNÇÕES ADC REVISADAS ===

// Configuração do ADC: Referência AVCC e Prescaler 128
void ADC_Init(void){
    // ADMUX: REFS0=1 (Referência AVCC - 5V) | MUX[3:0]=0000 (Canal ADC0 / Pino C0)
    ADMUX = (1<<REFS0); 
    
    // ADCSRA: ADEN=1 (Habilita ADC) | ADPS[2:0]=111 (Prescaler 128)
    // 16MHz / 128 = 125kHz (ideal para conversão)
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); 
}

// Leitura do ADC (Canal 0)
uint16_t ADC_Read(void){
    ADCSRA |= (1<<ADSC);          // Inicia a conversão (Start Conversion)
    while(ADCSRA & (1<<ADSC));    // Espera a conversão terminar (ADSC = 0)
    return ADCW;                  // Retorna o resultado de 10 bits (0-1023)
}

// === FUNÇÃO PRINCIPAL COM DEBUG ===
int main(void)
{
    UART_Init(MYUBRR);
    UART_Transmit_String("--- Inicializando ---\n");
    _delay_ms(100);

    ADC_Init(); 
//    UART_Transmit_String("Descartando 1a leitura...\n");
    ADC_Read();
    UART_Transmit('\n');
    _delay_ms(100);

    uint16_t leitura_ldr;
    uint16_t lux_estimado;

    while(1)
    {
        leitura_ldr = ADC_Read();
        lux_estimado = ADC_to_LUX(leitura_ldr);

        UART_Transmit_String("ADC: ");
        UART_Transmit_Number(leitura_ldr); 
        
        UART_Transmit_String(" | LUX APROX.: ");
        UART_Transmit_LUX(lux_estimado); // Usa a nova função de transmissão LUX
        
        UART_Transmit('\n');

        _delay_ms(500); 
    }
    return 0;
}