**Controle de LEDs, Botões, Display SSD1306 e Comunicação via UART.**

**Descrição do Projeto**

Este projeto visa o controle de LEDs, botões e um display SSD1306 utilizando a placa BitDogLab. O sistema irá interagir com o usuário por meio de um display de 128x64 e uma matriz 5x5 de LEDs endereçáveis, além de LEDs RGB. O projeto tem como objetivo demonstrar a utilização de diferentes tecnologias, como interrupções, controle de LEDs, protocolo I2C, e comunicação UART para entrada de caracteres via Serial Monitor.

**Componentes Utilizados**

Matriz 5x5 de LEDs WS2812 (conectada à GPIO 7)
LED RGB (conectado às GPIOs 11, 12 e 13)
Botão A (conectado à GPIO 5)
Botão B (conectado à GPIO 6)
Display SSD1306 (conectado via I2C às GPIOs 14 e 15)
PC com VS Code e Serial Monitor para entrada de caracteres

**Funcionalidades**

**1. Entrada de Caracteres via PC**

O sistema deve permitir que o usuário digite caracteres no Serial Monitor do VS Code.
Cada caractere digitado será exibido no display SSD1306.
Quando um número entre 0 e 9 for digitado, um símbolo correspondente ao número será exibido também na matriz 5x5 WS2812.

**2. Interação com o Botão A**

Botão A deve alternar o estado do LED RGB Verde (ligado/desligado).
O estado do LED será registrado de duas formas:
Uma mensagem sobre o estado do LED será exibida no display SSD1306.
Uma mensagem sobre o estado do LED será enviada ao Serial Monitor.

**3. Interação com o Botão B**

Botão B deve alternar o estado do LED RGB Azul (ligado/desligado).
O estado do LED será registrado de duas formas:
Uma mensagem sobre o estado do LED será exibida no display SSD1306.
Uma mensagem sobre o estado do LED será enviada ao Serial Monitor.

**Requisitos do Projeto**

**1. Uso de Interrupções**

Todas as funcionalidades relacionadas aos botões devem ser implementadas utilizando rotinas de interrupção (IRQ) para garantir a resposta rápida e eficiente ao pressionamento dos botões.

**2. Debouncing**

É obrigatório implementar o tratamento do debouncing dos botões via software para evitar múltiplos registros de um único pressionamento.

**3. Controle de LEDs**

O projeto deve demonstrar o controle de LEDs comuns e LEDs WS2812 para exibir diferentes padrões e símbolos. Isso inclui a interação com a matriz 5x5 de LEDs e o controle dos LEDs RGB.

**4. Utilização do Display 128x64**

O projeto utilizará o display SSD1306 de 128x64 para exibir mensagens e símbolos. Será necessário utilizar fontes tanto maiúsculas quanto minúsculas, demonstrando o domínio da biblioteca e do protocolo I2C.

**5. Envio de Informação pela UART**

A comunicação serial via UART será usada para enviar informações sobre o estado dos LEDs e outras mensagens ao Serial Monitor do VS Code.

**Conexões do Projeto**

Matriz 5x5 de LEDs WS2812: GPIO 7
LED RGB: GPIOs 11 (vermelho), 12 (verde), 13 (azul)
Botão A: GPIO 5
Botão B: GPIO 6
Display SSD1306: I2C nas GPIOs 14 (SCL) e 15 (SDA)

**Como Compilar e Executar**

**1. Configuração do Ambiente**

VS Code: Certifique-se de que o VS Code esteja instalado, juntamente com a extensão para PlatformIO ou Arduino.

**Bibliotecas necessárias:**

Adafruit SSD1306 para o display.
Adafruit GFX para gráficos no display.
Adafruit NeoPixel para controle da matriz 5x5 de LEDs WS2812.

**2. Carregar o Código**

Conecte a placa BitDogLab ao seu computador.
Abra o projeto no VS Code.
Compile e faça o upload do código para a placa.

**3. Teste no Serial Monitor**

Abra o Serial Monitor no VS Code.
Digite um caractere no monitor para verificar se ele aparece no display SSD1306 e na matriz 5x5 de LEDs.
Teste a interação com os Botões A e B para alternar o estado dos LEDs RGB.

Segue abaixo o link do vídeo de apresentação da Placa BitDogLab em funcionamento com o código descrito.

https://drive.google.com/file/d/1gEOb1sZVw0_uPzMLag1z4noBJALxDG8G/view?usp=sharing