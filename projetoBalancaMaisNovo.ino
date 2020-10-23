/*
   Trabalho de Graduação - Gestão da Tecnologia da Informação
   Integrantes: Carlos E. Miranda e Thiago T. Yara
   Instituição de ensino: Fatec-Itu.
   Última revisão do código: 22/10/2020
*/

// inclusão das bibliotecas
#include <Wire.h>
#include <Ethernet.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// definição das constantes de ligação do LCD
#define LCD_ADDR 0x27
#define COL_NUM  16
#define ROW_NUM  2

// definição das constantes de ligação da balança 1
#define DOUT_B1  2
#define SCK_B1   3

// definição das constantes de ligação da balança 2
#define DOUT_B2  5
#define SCK_B2   6

// definição das constantes dos botões
#define BOTAO_DIREITA  16 // A2
#define BOTAO_ESQUERDA 14 // A0
#define BOTAO_ENTRAR   15 // A1
#define BOTAO_TARA     17 // A3

// definição do número da camada de menus da tela LCD
#define NUM_MAX_MENU 3

// criação dos objetos para as classes
LiquidCrystal_I2C lcd(LCD_ADDR, COL_NUM, ROW_NUM); 
EthernetClient client;                             
HX711 balanca1, balanca2;                          

float fatorCalibracaoB1 = 455670, fatorCalibracaoB2 = 455670; // variáveis de calibração

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // endereço MAC do módulo ethernet shield
char servidor[] = "prj1.xyz";                        // servidor onde serão armazenados os dados

byte numMenu = 1,      // referência à camada do menu do LCD
     subMenu = 1,      // referência à camada do submenu do LCD
     posicaoVetorAB1 = 0, posicaoVetorAB2 = 0, posicaoVetorEMB1 = 0, posicaoVetorEMB2 = 0; // verredura de vetores

// vetor de alimentos da balança 1
String alimentosB1[] = {"Nenhum", "Arroz", "Feijao", "Sal", "Acucar", "Cafe"};

// vetor de alimentos da balança 2
String alimentosB2[] = {"Nenhum", "Arroz", "Feijao", "Sal", "Acucar", "Cafe"};

// vetor de valores para o estoque mínimo da balança 1
float estoqueMinimoB1[] = {0.250, 0.500, 0.750, 1.000, 1.250};

// vetor de valores para o estoque mínimo da balança 2
float estoqueMinimoB2[] = {0.250, 0.500, 0.750, 1.000, 1.250};

String AB1, AB2;  // armazena os alimentos das balanças 1 e 2
float EMB1, EMB2, MAB1, MAB2; // armazena os estoques mínimos e as pesos atuais das balanças 1 e 2

String dadosPost; // armazena para ser enviado ao servidor

String baixoEstoqueB1 = "Baixo!", baixoEstoqueB2 = "Baixo!", // variáveis para mostrar "Baixo Estoque"
Div = "="; // variável para fazer a divisão da string de saída

// elementos para serem enviados ao servidor
String alimentoB1 = "alimentoB1", massaAtualB1 = "massaAtualB1", alimentoB2 = "alimentoB2", massaAtualB2 = "massaAtualB2";

void setup() {

  Serial.begin(9600);

  // configura os botões como pull-up para funcionamento com resistência interna
  pinMode(BOTAO_DIREITA,  INPUT_PULLUP);
  pinMode(BOTAO_ESQUERDA, INPUT_PULLUP);
  pinMode(BOTAO_ENTRAR,   INPUT_PULLUP);
  pinMode(BOTAO_TARA,     INPUT_PULLUP);

  // inicia as balanças
  balanca1.begin(DOUT_B1, SCK_B1);
  balanca2.begin(DOUT_B2, SCK_B2);

  // calibra e tara as balanças para o fator de calibração previamente calculado e definido
  balanca1.set_scale(fatorCalibracaoB1);
  balanca1.tare();
  balanca2.set_scale(fatorCalibracaoB2);
  balanca2.tare();

  // liga o LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  alocaIP();           // chama função de alocação de IP para módulo Shield
  
  dadosPost = alimentoB1 + AB1 + alimentoB2 + AB2; // inicializa os alimentos para serem postados
}

void loop() {

  // variáveis globais para -> Alimento (AB1 e AB2); Estoque Mínimo (EMB1 e EMB2) e Massa Atual (MAB1 e MAB2) 
  AB1 = posicaoVetorAB1[alimentosB1];
  AB2 = posicaoVetorAB2[alimentosB2];

  EMB1 = posicaoVetorEMB1[estoqueMinimoB1], 3;
  EMB2 = posicaoVetorEMB2[estoqueMinimoB2], 3;

  MAB1 = balanca1.get_units(), 3;
  MAB2 = balanca2.get_units(), 3;

  if ((MAB1 <= EMB1) && (MAB2 <= EMB2)) {

    dadosPost = alimentoB1 + Div + AB1  + Div + baixoEstoqueB1 + Div + AB2 + Div + baixoEstoqueB2;
    String dadosPost = "alimentoB1" + Div + AB1 + Div + baixoEstoqueB1 + Div + AB2 + Div + baixoEstoqueB2; // aloca os elementos para serem enviados ao servidor
    enviaDados(); // envia os dados para o servidor
  }

  else if (MAB1 <= EMB1) {

    dadosPost = alimentoB1 + Div + AB1 + Div + baixoEstoqueB1 + Div + AB2 + Div + MAB2;
    String dadosPost = "alimentoB1" + Div + AB1 + Div + baixoEstoqueB1 + Div + AB2 + Div + String(MAB2, 3); 
    enviaDados(); 
  }

  else if (MAB2 <= EMB2) {

    dadosPost = alimentoB1 + Div + AB1 + Div + MAB1 + Div + AB2 + Div + baixoEstoqueB2;
    String dadosPost = "alimentoB1" + Div + AB1 + Div + String(MAB1, 3) + Div + AB2 + Div + baixoEstoqueB2;
    enviaDados();
  }

  else if ((MAB1 > EMB1) && (MAB2 > EMB2)) {

    dadosPost = alimentoB1 + Div + AB1 + Div + MAB1 + Div + AB2 + Div + MAB2;
    String dadosPost = "alimentoB1" + Div + AB1 + Div + String(MAB1, 3) + Div + AB2 + Div + String(MAB2, 3);
    enviaDados();
  }

  teclado(); // chama a função dos botões

  switch (numMenu) {

    // chama as funções dos menus
    case 1: menuBalancaUm();   break;
    case 2: menuBalancaDois(); break;
    case 3: menuInfosGerais(); break;
  }
}

// recolhe as respostas dos botões
void teclado() {

  if (!digitalRead(BOTAO_DIREITA) && subMenu == 1) {

    delay(150);
    if (numMenu <= NUM_MAX_MENU) numMenu++;
  }

  if (!digitalRead(BOTAO_DIREITA) && subMenu == 1) {

    delay(150);
    if (numMenu >= NUM_MAX_MENU) numMenu = 1;
  }

  if (!digitalRead(BOTAO_ESQUERDA) && subMenu == 1) {

    delay(150);
    if (numMenu > 0) numMenu--;
  }

  if (!digitalRead(BOTAO_ESQUERDA) && subMenu == 1) {

    delay(150);
    if (numMenu <= NUM_MAX_MENU) numMenu = 3;
  }

  if (!digitalRead(BOTAO_ENTRAR)) {

    delay(150);
    if (subMenu <= 2) subMenu++;
  }

  if (!digitalRead(BOTAO_ENTRAR)) {

    delay(150);
    if (subMenu >= 2) subMenu = 1;
  }

  if (!digitalRead(BOTAO_TARA)) {

    delay(150);
    zeraBalanca(); // zera a balança
  }
}

// disponibiliza na tela os menus e submenus da balança 1
void menuBalancaUm() {

  switch (subMenu) {

    // chama os submenus do menu da balança 1
    case 1:

      lcd.setCursor(0, 0);
      lcd.print("< Def. Balanca >");
      lcd.setCursor(0, 1);
      lcd.print("        1       ");
      break;

    case 2:

      alimentoBalancaUm();
      break;

    case 3:

      estoqueMinimoBalancaUm();
      break;
  }
}

// disponibiliza na tela os menus e submenus da balança 2
void menuBalancaDois() {

  switch (subMenu) {

    // chama os submenus do menu da balança 2
    case 1:

      lcd.setCursor(0, 0);
      lcd.print("< Def. Balanca >");
      lcd.setCursor(0, 1);
      lcd.print("        2       ");
      break;

    case 2:

      alimentoBalancaDois();
      break;

    case 3:

      estoqueMinimoBalancaDois();
      break;
  }
}

// disponibiliza na tela os pesos recolhidos das balanças
void menuInfosGerais() {

  switch (subMenu) {

    // chama o submenu com as pesos sobre as balanças 1 e 2
    case 1:

      lcd.setCursor(0, 0);
      lcd.print("<     Pesos    >");
      lcd.setCursor(0, 1);
      lcd.print("      Atuais    ");
      break;

    case 2:

      dadosBalancas(); // mostra os dados com as pesos sobre as balanças 1 e 2
      break;
  }
}

// recebe o alimento da balança 1 informado pelo usuário
void alimentoBalancaUm() {

  if (!digitalRead(BOTAO_DIREITA)) {

    posicaoVetorAB1[alimentosB1];
    posicaoVetorAB1++;

    if (posicaoVetorAB1 == 6)

      posicaoVetorAB1 = 0;
  }

  if (!digitalRead(BOTAO_ESQUERDA)) {

    posicaoVetorAB1[alimentosB1];
    posicaoVetorAB1--;

    if (posicaoVetorAB1 == 255)

      posicaoVetorAB1 = 5;
  }

  AB1 = posicaoVetorAB1[alimentosB1]; // variável local para o alimento da balança 1

  lcd.setCursor(0, 0);
  lcd.print("Alimento B1:    ");
  lcd.setCursor(0, 1);
  lcd.print(AB1);
  lcd.print("                ");
}

// recebe o alimento da balança 2 informado pelo usuário
void alimentoBalancaDois() {

  if (!digitalRead(BOTAO_DIREITA)) {

    posicaoVetorAB2[alimentosB2];
    posicaoVetorAB2++;

    if (posicaoVetorAB2 == 6)

      posicaoVetorAB2 = 0;
  }

  if (!digitalRead(BOTAO_ESQUERDA)) {

    posicaoVetorAB2[alimentosB2];
    posicaoVetorAB2--;

    if (posicaoVetorAB2 == 255)

      posicaoVetorAB2 = 5;
  }

  AB2 = posicaoVetorAB2[alimentosB2]; // variável local para o alimento da balança 2

  lcd.setCursor(0, 0);
  lcd.print("Alimento B2:    ");
  lcd.setCursor(0, 1);
  lcd.print(AB2);
  lcd.print("                ");
}

// recebe o estoque mínimo da balança 1 informado pelo usuário
void estoqueMinimoBalancaUm() {

  if (!digitalRead(BOTAO_DIREITA)) {

    posicaoVetorEMB1[estoqueMinimoB1];
    posicaoVetorEMB1++;

    if (posicaoVetorEMB1 == 5)

      posicaoVetorEMB1 = 0;
  }

  if (!digitalRead(BOTAO_ESQUERDA)) {

    posicaoVetorEMB1[estoqueMinimoB1];
    posicaoVetorEMB1--;

    if (posicaoVetorEMB1 == 255)

      posicaoVetorEMB1 = 4;
  }

  EMB1 = posicaoVetorEMB1[estoqueMinimoB1], 3; // variável local para o estoque mínimo da balança 1

  lcd.setCursor(0, 0);
  lcd.print("Est. Min. B1:   ");
  lcd.setCursor(0, 1);
  lcd.print(EMB1, 3);
  lcd.print(" kg             ");
}

// recebe o estoque mínimo da balança 2 informado pelo usuário
void estoqueMinimoBalancaDois() {

  if (!digitalRead(BOTAO_DIREITA)) {

    posicaoVetorEMB2[estoqueMinimoB2];
    posicaoVetorEMB2++;

    if (posicaoVetorEMB2 == 5)

      posicaoVetorEMB2 = 0;
  }

  if (!digitalRead(BOTAO_ESQUERDA)) {

    posicaoVetorEMB2[estoqueMinimoB2];
    posicaoVetorEMB2--;

    if (posicaoVetorEMB2 == 255)

      posicaoVetorEMB2 = 4;
  }

  EMB2 = posicaoVetorEMB2[estoqueMinimoB2], 3; // variável local para o estoque mínimo da balança 2

  lcd.setCursor(0, 0);
  lcd.print("Est. Min. B2:   ");
  lcd.setCursor(0, 1);
  lcd.print(EMB2, 3);
  lcd.print(" kg             ");
}

// mostra as pesos dos produtos das balanças
void dadosBalancas() {

  MAB1 = balanca1.get_units(), 3;
  MAB2 = balanca2.get_units(), 3;

  lcd.setCursor(0, 0);
  lcd.print("PB1 =  ");
  lcd.setCursor(7, 0);
  lcd.print(MAB1, 2);
  lcd.setCursor(14, 0);
  lcd.print("kg");

  lcd.setCursor(0, 1);
  lcd.print("PB2 =  ");
  lcd.setCursor(7, 1);
  lcd.print(MAB2, 2);
  lcd.setCursor(14, 1);
  lcd.print("kg");
}

// zera(tara) a balança
void zeraBalanca() {

  balanca1.tare();
  balanca2.tare();
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Balancas");
  lcd.setCursor(4, 1);
  lcd.print("Zeradas!");
  delay(1500);
}

// aloca um endereço de IP disponível na rede via DHCP para o módulo ethernet shield
void alocaIP() {

  if (Ethernet.begin(mac) == 0) {

    Serial.println("Falha ao configurar Ethernet Shield utilizando DHCP!");
  }

  Serial.print("IP Ethernet Shield: ");
  Serial.println(Ethernet.localIP());
  Serial.println();
}

// envia os dados para o servidor
void enviaDados() {

  if (client.connect(servidor, 80)) {

    client.println("POST /main.php HTTP/1.1");
    client.println("Host: prj1.xyz");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(dadosPost.length());
    client.println();
    client.print(dadosPost);

    Serial.println(dadosPost);
  }

  if (client.connected()) {

    client.stop();
  }
}
