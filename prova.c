#include <REGX52.H>

// === Barramento ===
#define GlcdDataBus  P3
sbit RS  = P2^0;
sbit RW  = P2^1;
sbit EN  = P2^2;
sbit CS1 = P2^3;
sbit CS2 = P2^4;
sbit RST = P2^5;

// === Botões ===
sbit BTN_LEFT  = P2^6;
sbit BTN_RIGHT = P2^7;

// === Caracteres personalizados ===
code unsigned char GLYPHS[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0: SPACE
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // 1: PIXEL
    {0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00}, // 2: LADO
    {0x0A, 0x1F, 0x12, 0x12, 0x1F, 0x0A}, // 3: CARRINHO
    {0x0A, 0x1F, 0x12, 0x12, 0x1F, 0x0A}, // 4: CARRINHOMAL
    {0xF5, 0xE0, 0xED, 0xED, 0xE0, 0xF5}, // 5: CARRINHONOITE
    {0xEA, 0xF1, 0xE0, 0xF1, 0xEA, 0xFF}, // 6: LADONOITE
    {0x18, 0x10, 0x00, 0x00, 0x10, 0x18}, // 7: FAROL
};

code unsigned char LETRAS[13][6] = {
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00},	// 0 - 'O'
    {0x7F, 0x02, 0x0C, 0x30, 0x7F, 0x00},	// 1 - 'N'
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00},	// 2 - 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00},	// 3 - 'E'
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00},	// 4 - 'V'
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00},	// 5 - 'C'
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00},	// 6 - 'T'
    {0x41, 0x41, 0x7F, 0x41, 0x41, 0x00},	// 7 - 'I'
    {0x02, 0x01, 0x59, 0x09, 0x06, 0x00},	// 8 - '?'
    {0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00},	// 9 - 'A'
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00},	// 10 - '!' 
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 11 - ' '
		{0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // 12 - 'U'
};

code unsigned char NUMEROS[13][6] = {
    {0x3E, 0x45, 0x49, 0x51, 0x3E, 0x00}, // 0
    {0x00, 0x21, 0x7F, 0x01, 0x00, 0x00}, // 1
    {0x23, 0x45, 0x49, 0x51, 0x21, 0x00}, // 2
    {0x42, 0x41, 0x51, 0x69, 0x46, 0x00}, // 3
    {0x0C, 0x14, 0x24, 0x7F, 0x04, 0x00}, // 4
    {0x72, 0x51, 0x51, 0x51, 0x4E, 0x00}, // 5
    {0x1E, 0x29, 0x49, 0x49, 0x06, 0x00}, // 6
    {0x40, 0x47, 0x48, 0x50, 0x60, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x30, 0x49, 0x49, 0x4A, 0x3C, 0x00}, // 9
    {0x00, 0x00, 0x06, 0x06, 0x00, 0x00}, // '.'
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // 'k'
    {0xFE, 0x60, 0x18, 0x60, 0xFE, 0x00}, // 'M'
};

// === Linhas ===
idata unsigned char linha1[10], linha2[10], linha3[10], linha4[10];
idata unsigned char linha5[10], linha6[10], linha7[10], linha8[10];

// === Carrinho jogador ===
unsigned char carrinhoPos = 4;

// === Paredes para curvas ===
unsigned char leftWall = 0;
unsigned char rightWall = 9;

// === Variáveis auxiliares ===
unsigned char frameCount = 0;  // para controlar espaçamento dos inimigos
unsigned char seed = 3;        // semente simples para pseudo-random
unsigned char morte = 0;       // 0 = parede, 1 = batida
unsigned int distancia = 0;    // em décimos de km (ex: 12 = 1.2km)

// === Túnel ===
bit modoTunel = 0;

// === Delay ===
void delay(int t) {
    int i, j;
    for(i = 0; i < t; i++)
        for(j = 0; j < 10; j++);
}

// === GLCD ===
void Glcd_SelectPage0() { CS1 = 0; CS2 = 1; }
void Glcd_SelectPage1() { CS1 = 1; CS2 = 0; }

void Glcd_CmdWrite(char cmd) {
    GlcdDataBus = cmd;
    RS = 0; RW = 0;
    EN = 1; delay(1); EN = 0; delay(1);
}

void Glcd_DataWrite(char dat) {
    GlcdDataBus = dat;
    RS = 1; RW = 0;
    EN = 1; delay(1); EN = 0; delay(1);
}

void Glcd_DisplayChar(unsigned char *symbol) {
    int i;
    for(i = 0; i < 6; i++)
        Glcd_DataWrite(symbol[i]);
}

void Glcd_PrintLine(unsigned char *line, int length) {
    int i;
    for(i = 0; i < length; i++)
        Glcd_DisplayChar(GLYPHS[line[i]]);
}

// === Inverte alguns caracteres que estavam sendo mostrados de ponta cabeça ===
unsigned char reverseBits(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// === Desenha a pontuação ===
void drawScore() {
    unsigned char digits[6];  // "12.3km"
    unsigned int d = distancia, i, j;

    digits[0] = d / 100;
    digits[1] = (d / 10) % 10;
    digits[2] = 10; // '.'
    digits[3] = d % 10;
    digits[4] = 11; // 'k'
    digits[5] = 12; // 'm'

    Glcd_SelectPage1();
    Glcd_CmdWrite(0x40);  // coluna 0
    Glcd_CmdWrite(0xBA);  // linha 0

    for(i = 0; i < 6; i++) {
        for(j = 0; j < 6; j++) {
            Glcd_DataWrite(reverseBits(NUMEROS[digits[i]][j]));
        }
    }
}

// === Move linhas e insere nova no topo ===
void shiftDownAndAddNewLine(unsigned char *newLine) {
    int i;
    for(i = 0; i < 10; i++) {
        linha8[i] = linha7[i];
        linha7[i] = linha6[i];
        linha6[i] = linha5[i];
        linha5[i] = linha4[i];
        linha4[i] = linha3[i];
        linha3[i] = linha2[i];
        linha2[i] = linha1[i];
        linha1[i] = newLine[i];
    }
}

// === Atualiza carrinho com botões ===
void updateCarPosition() {
    static bit btnLeftPrev = 1;
    static bit btnRightPrev = 1;
		
		// === Precisa esperar o botão soltar para evitar debounce ===
		
    // Botão esquerda:
    if (BTN_LEFT == 0 && btnLeftPrev == 1) {
        carrinhoPos--;
    }
    btnLeftPrev = BTN_LEFT;

    // Botão direita:
    if (BTN_RIGHT == 0 && btnRightPrev == 1) {
        carrinhoPos++;
    }
    btnRightPrev = BTN_RIGHT;
}

// === Cria nova linha com carrinho inimigo de tempos em tempos ===
void generateNewLine(unsigned char *newLine) {
    int i;
    unsigned char pos;

    for(i = 0; i < 10; i++) newLine[i] = 0;

    // Seed para criar aleatoriedade
    seed = (seed * 17 + carrinhoPos * 31 + frameCount * 23 + linha2[4]) % 251;
    frameCount++;

    // Movimento das paredes
    if ((seed % 5) < 2 && leftWall < 1) leftWall++;
    else if ((seed % 5) > 3 && leftWall > 0) leftWall--;

    if ((seed % 7) < 2 && rightWall < 9) rightWall++;
    else if ((seed % 7) > 4 && rightWall > 8) rightWall--;

    newLine[leftWall]  = 2;
    newLine[rightWall] = 2;

    // Cria carrinho inimigo nas colunas de 3 a 8, para nenhum nascer dentro da parede
    if(frameCount >= (seed % 3) + 5) {
        frameCount = 0;
        pos = 2 + (seed % 6);
        newLine[pos] = 4; 
    }

    distancia++;  // 0.1km por linha

    // Entra em modo túnel a partir de 15.0 km
    if(distancia >= 150) {
        modoTunel = 1;
    }
}

// === Redesenha a pista ===
void redrawTrack() {
    int i, j;
    unsigned char tempLinha8[10];	// O uso de array temporário é necessário para mudanças de fundo entre
																	// pista e túnel sem gerar erros como deletar o carrinho do jogador ou
																	// outros problemas de atualização.

    Glcd_SelectPage0();

    // Primeiras linhas (1 a 7)
    for(j = 0; j < 7; j++) {
        unsigned char *linha;
        unsigned char tempLine[10];
        switch(j) {
            case 0: linha = linha1; break;
            case 1: linha = linha2; break;
            case 2: linha = linha3; break;
            case 3: linha = linha4; break;
            case 4: linha = linha5; break;
            case 5: linha = linha6; break;
            case 6: linha = linha7; break;
        }

        for(i = 0; i < 10; i++) {
            tempLine[i] = linha[i];

            // === Modo túnel ===
            if (modoTunel) {
                // Escurecer fundo (0 vira 1)
                if (tempLine[i] == 0) tempLine[i] = 1;

                // Carrinho inimigo invisível, exceto se estiver no cone do farol
                if (linha[i] == 4) {
                    int visivel = 0;
                    int dist = 7 - j; // distância até o carrinho (linha 8)

                    if (dist == 1 && i == carrinhoPos) visivel = 1;
                    else if (dist == 2 && i >= carrinhoPos - 1 && i <= carrinhoPos + 1) visivel = 1;
                    else if (dist == 3 && i >= carrinhoPos - 2 && i <= carrinhoPos + 2) visivel = 1;

                    if (!visivel) {
                        tempLine[i] = 1;  // escurece
                    }
                }

                // Farol desenhado na linha 7
                if(j == 6) {
                    if(i == carrinhoPos)
                        tempLine[i] = 7;
                    else if(tempLine[i] == 7)
                        tempLine[i] = 1;
                }
            } else {
                // Fora do túnel: remove farol
                if(tempLine[i] == 7)
                    tempLine[i] = 0;
            }
        }

        Glcd_CmdWrite(0x40);
        Glcd_CmdWrite(0xB8 + j);
        Glcd_PrintLine(tempLine, 10);
    }

    // === Linha 8 (jogador) ===
    for(i = 0; i < 10; i++) {
        tempLinha8[i] = linha8[i];

        if(modoTunel) {
            // Escurecer fundo
            if(tempLinha8[i] == 0) tempLinha8[i] = 1;

            // Esconder carrinhos inimigos
            if(tempLinha8[i] == 4) tempLinha8[i] = 1;
        }
    }

    // Carrinho do jogador
    tempLinha8[carrinhoPos] = modoTunel ? 5 : 3;
    tempLinha8[0] = 2;
    tempLinha8[9] = 2;

    Glcd_CmdWrite(0x40);
    Glcd_CmdWrite(0xBF);  // linha 8
    Glcd_PrintLine(tempLinha8, 10);

    drawScore();
}

// === Verifica colisão ===
bit checkCollision() {
    if (linha8[carrinhoPos] == 2) {
        morte = 0;  // parede
        return 1;
    }
    if (linha8[carrinhoPos] == 4) {
        morte = 1;  // carrinho inimigo
        return 1;
    }
    return 0;
}

void vitoriaScreen() {
    code unsigned char vitoriaMsg[10] = {4,5,11,4,3,1,5,3,12,10}; // "VC VENCEU!"
		int j;
    
    Glcd_SelectPage1();
    Glcd_CmdWrite(0x40);
    Glcd_CmdWrite(0xBC);

    for(j = 0; j < 10; j++) {
        Glcd_DisplayChar(LETRAS[vitoriaMsg[j]]);
    }

    while(1);  // trava o jogo após vitória
}


// === Tela de Game Over ===
void gameOverScreen() {
    int j;
    code unsigned char msg0[10] = {0,1,2,3,4,5,4,9,7,8}; // ONDEVCVAI?
    code unsigned char msg1[10] = {9,5,7,2,3,1,6,3,10,10}; // ACIDENTE!!

    Glcd_SelectPage1();
    Glcd_CmdWrite(0x40);
    Glcd_CmdWrite(0xBC);

    for(j = 0; j < 10; j++) {
        if (morte == 0)
            Glcd_DisplayChar(LETRAS[msg0[j]]);  // mensagem "parede"
        else
            Glcd_DisplayChar(LETRAS[msg1[j]]);  // mensagem "inimigo"
    }

    while(1); // trava o jogo
}

// === MAIN ===
void main() {
    idata unsigned char novaLinha[10];
		RST = 1;

    generateNewLine(novaLinha);
    shiftDownAndAddNewLine(novaLinha);
    redrawTrack();

    while (1) {
        delay(20);
        updateCarPosition();
        generateNewLine(novaLinha);

        if (distancia >= 201) {
            vitoriaScreen();
        }

        shiftDownAndAddNewLine(novaLinha);

        if (checkCollision()) {
            gameOverScreen();
        }

        redrawTrack();
    }
}