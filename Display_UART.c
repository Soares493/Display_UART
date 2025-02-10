#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// Definindo os pinos utilizados para os LEDs RGB e os botões
#define PINO_LED_VERMELHO 13
#define PINO_LED_VERDE 11
#define PINO_LED_AZUL 12
#define PINO_BOTAO_A 5
#define PINO_BOTAO_B 6

// Configuração do barramento I2C
#define BARRAMENTO_I2C i2c1
#define PINO_SDA 14
#define PINO_SCL 15
#define ENDERECO_I2C 0x3C

// Definindo o tempo de debounce (em milissegundos)
#define TEMPO_DEBOUNCE 400

// Configuração da quantidade de LEDs na matriz WS2812 e o pino correspondente
#define QUANTIDADE_LEDS 25
#define PINO_LEDS 7

// Definição do número de pixels na matriz
#define NUMERO_PIXELS QUANTIDADE_LEDS

// Estrutura para representar os pixels da matriz 5x5
struct pixel_t {
    uint8_t componente_verde, componente_vermelho, componente_azul;
};
typedef struct pixel_t npLED_t;
npLED_t leds[QUANTIDADE_LEDS];

// Variáveis de controle da máquina PIO para o WS2812
PIO pio_para_leds;
uint maquina_pio;

// Variáveis globais
ssd1306_t display;
char caractere_digitado = '\0';  // Caractere atualmente digitado
char letra_digitada = '\0';  // Letra atualmente digitada (A-Z)

// Variáveis para gerenciar o tempo de debounce
uint32_t ultimo_tempo_interrupcao_a = 0;
uint32_t ultimo_tempo_interrupcao_b = 0;

// Função para configurar os pinos como entrada ou saída
void configurar_pinos() {
    // Configura pinos para LEDs como saídas
    gpio_init(PINO_LED_VERMELHO);
    gpio_set_dir(PINO_LED_VERMELHO, GPIO_OUT);
    gpio_put(PINO_LED_VERMELHO, 0); // LED vermelho começa desligado

    gpio_init(PINO_LED_VERDE);
    gpio_set_dir(PINO_LED_VERDE, GPIO_OUT);
    gpio_put(PINO_LED_VERDE, 0); // LED verde começa desligado

    gpio_init(PINO_LED_AZUL);
    gpio_set_dir(PINO_LED_AZUL, GPIO_OUT);
    gpio_put(PINO_LED_AZUL, 0); // LED azul começa desligado
    
    // Configura os botões como entradas com resistência de pull-up ativada
    gpio_init(PINO_BOTAO_A);
    gpio_set_dir(PINO_BOTAO_A, GPIO_IN);
    gpio_pull_up(PINO_BOTAO_A);
    
    gpio_init(PINO_BOTAO_B);
    gpio_set_dir(PINO_BOTAO_B, GPIO_IN);
    gpio_pull_up(PINO_BOTAO_B);
}

// Função para configurar o display SSD1306
void configurar_display(ssd1306_t *display) {
    // Inicializa o display SSD1306 com as configurações definidas
    ssd1306_init(display, WIDTH, HEIGHT, false, ENDERECO_I2C, BARRAMENTO_I2C);
    ssd1306_config(display);
    ssd1306_send_data(display);
    ssd1306_fill(display, false);
    ssd1306_send_data(display);
}

// Função para alternar o estado do LED verde
void alternar_led_verde(bool estado) {
    gpio_put(PINO_LED_VERDE, estado);
}

// Função para alternar o estado do LED azul
void alternar_led_azul(bool estado) {
    gpio_put(PINO_LED_AZUL, estado);
}

// Função para exibir uma mensagem no display SSD1306
void exibir_mensagem(ssd1306_t *display, const char *mensagem, int x_pos, int y_pos) {
    ssd1306_fill(display, false);  // Limpa a tela antes de desenhar
    ssd1306_draw_string(display, mensagem, x_pos, y_pos); // Exibe o texto no display
    ssd1306_send_data(display);   // Atualiza o conteúdo do display
}

// Função para inicializar a matriz WS2812
void npInicializar(uint pino) {
    // Carrega o programa WS2818 na máquina PIO
    uint offset = pio_add_program(pio0, &ws2818b_program);
    pio_para_leds = pio0;

    maquina_pio = pio_claim_unused_sm(pio_para_leds, false);
    if (maquina_pio < 0) {
        pio_para_leds = pio1;
        maquina_pio = pio_claim_unused_sm(pio_para_leds, true); // Se não houver máquina livre, emite erro
    }

    // Inicializa o programa WS2818 na máquina PIO
    ws2818b_program_init(pio_para_leds, maquina_pio, offset, pino, 800000.f);

    // Configura os LEDs para estarem apagados inicialmente
    for (uint i = 0; i < QUANTIDADE_LEDS; ++i) {
        leds[i].componente_vermelho = 0;
        leds[i].componente_verde = 0;
        leds[i].componente_azul = 0;
    }
}

// Função para configurar a cor RGB de um LED específico
void npDefinirLED(const uint indice, const uint8_t vermelho, const uint8_t verde, const uint8_t azul) {
    leds[indice].componente_vermelho = vermelho;
    leds[indice].componente_verde = verde;
    leds[indice].componente_azul = azul;
}

// Função para limpar todos os LEDs
void npLimpar() {
    for (uint i = 0; i < QUANTIDADE_LEDS; ++i)
        npDefinirLED(i, 0, 0, 0);
}

// Função para atualizar o estado dos LEDs com as cores armazenadas no buffer
void npAtualizar() {
    for (uint i = 0; i < QUANTIDADE_LEDS; ++i) {
        pio_sm_put_blocking(pio_para_leds, maquina_pio, leds[i].componente_verde);
        pio_sm_put_blocking(pio_para_leds, maquina_pio, leds[i].componente_vermelho);
        pio_sm_put_blocking(pio_para_leds, maquina_pio, leds[i].componente_azul);
    }
    sleep_us(100); // Atraso para garantir que o sinal seja processado corretamente
}

// Função para mapear as coordenadas X, Y para a posição do LED na matriz
int obterIndice(int x, int y) {
    if (y % 2 == 0) {
        return 24 - (y * 5 + x); // Para linhas pares, a ordem vai da esquerda para a direita
    } else {
        return 24 - (y * 5 + (4 - x)); // Para linhas ímpares, a ordem vai da direita para a esquerda
    }
}

// Função para desenhar um número na matriz 5x5
void Numero_Figura(int numero) {
    bool numeros[10][NUMERO_PIXELS] = {
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},  // 0
        {0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0},  // 1
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},  // 2
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},  // 3
        {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},  // 4
        {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},  // 5
        {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},  // 6
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},  // 7
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},  // 8
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0}   // 9
    };

    npLimpar();

    // Desenha o número na matriz de LEDs
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            int posicao = obterIndice(coluna, linha);
            if (numeros[numero][linha * 5 + coluna]) {
                npDefinirLED(posicao, 40, 0, 0); // Define a cor como vermelha
            }
        }
    }

    npAtualizar(); // Atualiza os LEDs com as cores definidas
}

// Função de interrupção para tratar os botões
void handler_interrupcao_gpio(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    char mensagem[20];

    // Processamento do botão A
    if (gpio == PINO_BOTAO_A) {
        if (tempo_atual - ultimo_tempo_interrupcao_a > TEMPO_DEBOUNCE) {
            ultimo_tempo_interrupcao_a = tempo_atual;

            static bool estado_led_verde = false;
            estado_led_verde = !estado_led_verde;
            alternar_led_verde(estado_led_verde);

            // Atualiza a mensagem no display conforme o estado do LED verde
            if (estado_led_verde) {
                snprintf(mensagem, sizeof(mensagem), "Verde Aceso");
            } else {
                snprintf(mensagem, sizeof(mensagem), "Verde Apagado");
            }

            exibir_mensagem(&display, mensagem, 0, 10);
            printf("Botão A. %s\n", mensagem);
        }
    }

    // Processamento do botão B
    if (gpio == PINO_BOTAO_B) {
        if (tempo_atual - ultimo_tempo_interrupcao_b > TEMPO_DEBOUNCE) {
            ultimo_tempo_interrupcao_b = tempo_atual;

            static bool estado_led_azul = false;
            estado_led_azul = !estado_led_azul;
            alternar_led_azul(estado_led_azul);

            // Atualiza a mensagem no display conforme o estado do LED azul
            if (estado_led_azul) {
                snprintf(mensagem, sizeof(mensagem), "Azul Aceso");
            } else {
                snprintf(mensagem, sizeof(mensagem), "Azul Apagado");
            }

            exibir_mensagem(&display, mensagem, 0, 10);
            printf("Botão B. %s\n", mensagem);
        }
    }
}

int main() {
    // Configura as inicializações
    stdio_init_all();
    i2c_init(BARRAMENTO_I2C, 400 * 1000);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SDA);
    gpio_pull_up(PINO_SCL);

    // Configurações de pinos, display e WS2812
    configurar_pinos();
    configurar_display(&display);
    npInicializar(PINO_LEDS);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, handler_interrupcao_gpio);
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, handler_interrupcao_gpio);

    // Loop principal do programa
    while (true) {
        // Aguarda e processa o caractere do Serial Monitor
        if (scanf("%c", &caractere_digitado) == 1) {
            // Exibe o caractere no display SSD1306
            char mensagem[20];
            snprintf(mensagem, sizeof(mensagem), "NUMERO %c", caractere_digitado);
            exibir_mensagem(&display, mensagem, 0, 0);

            // Desenha o caractere na matriz WS2812 se for um número
            if (caractere_digitado >= '0' && caractere_digitado <= '9') {
                Numero_Figura(caractere_digitado - '0');
            }

            // Mantém o caractere visível por 400ms
            sleep_ms(500);  
        }
    }

    return 0;
}
