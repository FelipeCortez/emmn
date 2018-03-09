//#include <LiquidCrystal.h>
//Posições de referência
double refAZ = 180.00, refELE = 90.00;

//posições atuais
double AZ = 0.0, ELE = 0.0, AZres = 0.0, ELEres = 0.0;

//Leitura da posição
int bits[16] = {};
double converter[16] = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0, 512.0, 1024.0, 2048.0, 4096.0, 8192.0, 16384.0, 32768.0};

//Status dos relés, inversores e controle manual
unsigned char ka1 = 0, ka2 = 0, daz = 0, del = 0, m = 0, state = 0, p = 0;

//variaveis para auxilio de contagem
int i = 0, j = 0, k = 0, bytes = 0;

//variavel watchdog
long int watchD = 0;
long int timer_display = 0;

//Variaveis do algoritmo de controle
//Azimute
double erroAZ = 0.0, erroaAZ = 0.0, outvAZ = 0.0, IsatAZ = 0.15, Ta = 1.00, kpAZ = 0.142, kiAZ = 0.0193, IAZ = 0.0; //Parametros de Kurios: kp=0.6 ki=0.05
double satAZ = 0.0;
//Elevação
double erroELE = 0.0, erroaELE = 0.0, IsatELE = 0.15, outvELE = 0.0, kpELE = 0.073, kiELE = 0.0099, IELE = 0.0;//kpAZ = 0.09, kiAZ = 0.007
double satELE = 0;

//variaveis da recepção das efemérides
//int stateString[13] = {W,a1,a0,e1,e0,C,cs};
unsigned char stateString[7] = {87, 48, 48, 48, 48, 56, 48};
unsigned char inputString[6] = {0};
//bytes de envio e recepção das mensagens
unsigned char a0 = 0, a1 = 0, e0 = 0, e1 = 0;
//auxiliares para converter double para int
unsigned short temp = 0;
//auxiliar para contagem
int cont = 0;

//variaveis para controle de erro de comunicação
int cont_erro = 0, erro_aknow = 0;
//contador para bytes que irão chegar
int cont_bytes = 0;

//Flags:
boolean rastreio = false; //Se verdadeira, liga potência do sistema
boolean STATE = false; //comando STATE
boolean SET = false; //Comando SET
boolean POWER = false;  //Comando POWER (habilita ou desabilita flag rastreio)
boolean ACK = false;  //Indica que o PC entendeu o ultimo comando enviado a ele
boolean NACK = false; //Indica que o PC não entendeu o ultimo comando enviado a ele
boolean outV = false; // flag que permite calculo e envio do sinal de controle
boolean manual = false; //Controle manual ativo ou não (Quando ativo impede calculo do controle PI)
boolean endMsg = false; //Flag de fim de mensagem

//FUNÇÕES ÚTEIS
//utilizada na recepção das referências

int verifica_checksum() {
  //verifica checksum
    if((inputString[0] ^ inputString[1] ^ inputString[2] ^ inputString[3] ^ inputString[4]) == inputString[5]) {
        return 0;
    } else {
        return 1;
    }
}

void verifica_status(unsigned char *_state) {
    if (digitalRead(12) == LOW) {
        daz = 1;
    } else {
        daz = 0;
    }

    if (digitalRead(11) == LOW) {
        del = 1;
    } else {
        del = 0;
    }

    if (digitalRead(10) == LOW) {
        ka1 = 1;
    }

    else {
        ka1 = 0;
    }

    if (digitalRead(8) == LOW) {
        ka2 = 1;
    } else {
        ka2 = 0;
    }

    if (digitalRead(7) == LOW) {
        m = 1;
    } else {
        m = 0;
    }

    if (rastreio == true) {
        p = 1;
    } else {
        p = 0;
    }

    *_state = p * 128 + ka1 * 16 + ka2 * 8 + daz * 4 + del * 2 + m * 1;
}

double readAngle(int aux_function) { //x=0 ->Ler Azimute; x=1 ->ler Elevação
    double angle = 0.0;
    int cont1 = 0, cont2 = 0;

    if (aux_function == 0) {
        digitalWrite(22, LOW); //libera leitura Azimute
        digitalWrite(24, HIGH); //bloqueia leitura elevação
    } else if(aux_function == 1) {
        digitalWrite(22, HIGH); //bloqueia leitura Azimute
        digitalWrite(24, LOW);  //libera leitura elevação
    }

    cont2 = 53;
    for (cont1 = 0; cont1 < 16; cont1++) { //loop para leitura de posição
        if (digitalRead(cont2) == HIGH) {
            bits[cont1] = 1;
        } else {
            bits[cont1] = 0;
        }

        angle = angle + bits[cont1] * converter[cont1];
        cont2 = cont2 - 2;
    }

    digitalWrite(22, HIGH); //bloqueia leitura azimute
    digitalWrite(24, HIGH); //bloqueia leitura elevação

    angle = angle * (360.0 / 65535.0);

    return angle;
}

void manual_control() {
    int valAZ = 0, valELE = 0;
    double vinAZ = 0.0, vinELE = 0.0;

    valAZ = analogRead(0);
    valELE = analogRead(1);

    //conversão para escala de Volts
    vinAZ = valAZ * (5.0 / 1023.0);
    vinELE = valELE * (5.0 / 1023.0);

    //ZONA MORTA - - - - - - - - - - - - - - - -
    //Verificação de 0 volts de saida AZ (2.5V)
    if ((vinAZ > 2.3) && (vinAZ < 2.7)) {
        vinAZ = 2.5;
    }

    //Verificação de 0 volts de saida ELE (2.5V)
    if ((vinELE > 2.3) && (vinELE < 2.7)) {
        vinELE = 2.5;
    }

    //ZONA DE GRAMPEAMENTO - - - - - - - - - - -
    //limite superior AZ
    if (vinAZ > 3.3) {
        vinAZ = 5.0;
    }

    //limite superior ELE
    if (vinELE > 3.3) {
        vinELE = 5.0;
    }

    //limite inferior AZ
    if (vinAZ < 1.7) {
        vinAZ = 0.0;
    }

    //limite inferior ELE
    if (vinELE < 1.7) {
        vinELE = 0.0;
    }

    //ZONA ATIVA A AZ - - - - - - - - - - -
    if ((vinAZ > 2.7) && (vinAZ < 3.3) ) {
        vinAZ = (vinAZ - 2.7) * (2.5 / 0.6) + 2.5;

    }

    //ZONA ATIVA A ELE - - - - - - - - - - -
    if ((vinELE > 2.7) && (vinELE < 3.3) ) {
        vinELE = (vinELE - 2.7) * (2.5 / 0.6) + 2.5;
    }

    //ZONA ATIVA B AZ - - - - - - - - - - -
    if ((vinAZ > 1.7) && (vinAZ < 2.3) ) {
        vinAZ = (vinAZ - 1.7) * (2.5 / 0.6);
    }

    //ZONA ATIVA B ELE - - - - - - - - - - -
    if ((vinELE > 1.7) && (vinELE < 2.3) ) {
        vinELE = (vinELE - 1.7) * (2.5 / 0.6);
    }

    //Sinal de saida pro inversor
    analogWrite(2, ((5.0 - vinAZ) * (255.0 / 5.0))); //AZ
    analogWrite(3, ((vinELE) * (255.0 / 5.0))); //ELE
}

void display_gaveta(double tempAZ, double tempELE) {
    int dAZ[4], dELE[4], dbits[8][4] = {0}, cont1 = 0, cont2 = 0; //dAZ e dELE -> Vetor dos caracteres das posições atuais
    boolean cELE = false, DELE = false, cAZ = false, DAZ = false; //dbits -> matriz que armazenará os bits referentes a cada caractere

    /*
    //Configuração dos pinos de controle do display da gaveta de comando
    //Azimute
    pinMode(34, OUTPUT);//parte fracionaria AZ
    pinMode(38, OUTPUT);//parte unitaria AZ
    pinMode(44, OUTPUT);//parte das dezenas AZ
    pinMode(52, OUTPUT);//parte das centenas AZ

    //Elevação
    pinMode(30, OUTPUT);//parte fracionaria ELE
    pinMode(36, OUTPUT);//parte unitaria ELE
    pinMode(40, OUTPUT);//parte das dezenas ELE
    pinMode(48, OUTPUT);//parte das centenas ELE

    //Bits
    pinMode(32, OUTPUT);//MSB
    pinMode(46, OUTPUT);
    pinMode(50, OUTPUT);
    pinMode(28, OUTPUT);//LSB

OBS: Configurando os bits de algum display como 0xF (15) ele é apagado
     */

    //transformar em numeros inteiros
    tempAZ = tempAZ * 10;
    tempELE = tempELE * 10;

    //particionamento das posições atuais
    dAZ[0] = tempAZ / 1000;                                        //Centena AZ
    dAZ[1] = (tempAZ - dAZ[0] * 1000) / 100;                       //Dezena AZ
    dAZ[2] = (tempAZ - dAZ[0] * 1000 - dAZ[1] * 100) / 10;         //Unidade AZ
    dAZ[3] = (tempAZ - dAZ[0] * 1000 - dAZ[1] * 100 - dAZ[2] * 10);//Fração AZ

    if (dAZ[0] == 0) { //se centena AZ for 0
        cAZ = true; //flag que permite apagar display centena AZ
    } else {
        cAZ = false;
    }

    if (dAZ[1] == 0) { //se dezena AZ for 0
        DAZ = true;      //desligar display dezena AZ (só é executado se cAZ=true)
    } else {
        DAZ = false;
    }

    //particionamento das posições atuais
    dELE[0] = tempELE / 1000;                                           //Centena ELE
    dELE[1] = (tempELE - dELE[0] * 1000) / 100;                         //Dezena ELE
    dELE[2] = (tempELE - dELE[0] * 1000 - dELE[1] * 100) / 10;          //Unidade ELE
    dELE[3] = (tempELE - dELE[0] * 1000 - dELE[1] * 100 - dELE[2] * 10);//Fração ELE

    if (dELE[0] == 1) { //se centena ELE for 1
        cELE = true; //flag que habilita display centena com digito 1
    } else {
        cELE = false;
    }

    if (dELE[1] == 0) {//se dezena AZ for 0
        DELE = true;     //Flag que permite apagar dezena ELE
    } else {
        DELE = false;
    }

    //Gera bits AZ
    for (cont1 = 0; cont1 < 4; cont1++) {
        for (cont2 = 0; cont2 < 4; cont2++) {
            if (cont2 < 3) {
                dbits[cont1][cont2] = (dAZ[cont1] >> cont2) % 2;
            } else {
                dbits[cont1][cont2] = dAZ[cont1] >> cont2;
            }
        }
    }

    //Gera bits ELE
    for (cont1 = 0; cont1 < 4; cont1++) {
        for (cont2 = 0; cont2 < 4; cont2++) {
            if (cont2 < 3) {
                dbits[cont1 + 4][cont2] = (dELE[cont1] >> cont2) % 2;
            } else {
                dbits[cont1 + 4][cont2] = dELE[cont1] >> cont2;
            }
        }
    }

    //ESCREVER NO DISPLAY
    //Centena Azimute--------------------------------
    //Se centena for 0
    if (cAZ == true) { //desliga display de centena (Configura como 15)
        digitalWrite(28, HIGH);
        digitalWrite(50, HIGH);
        digitalWrite(46, HIGH);
        digitalWrite(32, HIGH);
    } else {
        //bit D(MSB)
        if (dbits[0][3] == 1) {
            digitalWrite(32, HIGH);
        } else {
            digitalWrite(32, LOW);
        }

        //bit C
        if (dbits[0][2] == 1) {
            digitalWrite(46, HIGH);
        } else {
            digitalWrite(46, LOW);
        }

        //bit B
        if (dbits[0][1] == 1) {
            digitalWrite(50, HIGH);
        } else {
            digitalWrite(50, LOW);
        }

        //bit A(LSB)
        if (dbits[0][0] == 1) {
            digitalWrite(28, HIGH);
        } else {
            digitalWrite(28, LOW);
        }
    }

    digitalWrite(52, LOW); //Abilita escrita no pino 52
    delayMicroseconds(10);
    digitalWrite(52, HIGH); //Desabilita escrita no pino 52

    //DEZENA Azimute ------------------------------------------------
    //Se centena e dezena for 0
    if (DAZ == true && cAZ == true) { //desliga display de dezena
        digitalWrite(28, HIGH); //LSB
        digitalWrite(50, HIGH);
        digitalWrite(46, HIGH);
        digitalWrite(32, HIGH); //MSB
    } else {
        //bit D(MSB)
        if (dbits[1][3] == 1) {
            digitalWrite(32, HIGH);
        } else {
            digitalWrite(32, LOW);
        }

        //bit C
        if (dbits[1][2] == 1) {
            digitalWrite(46, HIGH);
        } else {
            digitalWrite(46, LOW);
        }

        //bit B
        if (dbits[1][1] == 1) {
            digitalWrite(50, HIGH);
        } else {
            digitalWrite(50, LOW);
        }

        //bit A(LSB)
        if (dbits[1][0] == 1) {
            digitalWrite(28, HIGH);
        } else {
            digitalWrite(28, LOW);
        }
    }

    digitalWrite(44, LOW); //Abilita escrita no pino 44
    delayMicroseconds(10);
    digitalWrite(44, HIGH); //Desabilita escrita no pino 44

    //UNIDADE Azimute ------------------------------------------------
    //bit D(MSB)

    if (dbits[2][3] == 1) {
        digitalWrite(32, HIGH);
    } else {
        digitalWrite(32, LOW);
    }

    //bit C
    if (dbits[2][2] == 1) {
        digitalWrite(46, HIGH);
    } else {
        digitalWrite(46, LOW);
    }

    //bit B
    if (dbits[2][1] == 1) {
        digitalWrite(50, HIGH);
    } else {
        digitalWrite(50, LOW);
    }

    //bit A(LSB)
    if (dbits[2][0] == 1) {
        digitalWrite(28, HIGH);
    } else {
        digitalWrite(28, LOW);
    }

    digitalWrite(38, LOW); //Abilita escrita no pino 38
    delayMicroseconds(10);
    digitalWrite(38, HIGH); //Desabilita escrita no pino 38

    //FRAÇÃO Azimute ------------------------------------------------

    //bit D(MSB)
    if (dbits[3][3] == 1) {
        digitalWrite(32, HIGH);
    } else {
        digitalWrite(32, LOW);
    }

    //bit C
    if (dbits[3][2] == 1) {
        digitalWrite(46, HIGH);
    } else {
        digitalWrite(46, LOW);
    }

    //bit B
    if (dbits[3][1] == 1) {
        digitalWrite(50, HIGH);
    } else {
        digitalWrite(50, LOW);
    }

    //bit A(LSB)
    if (dbits[3][0] == 1) {
        digitalWrite(28, HIGH);
    } else {
        digitalWrite(28, LOW);
    }

    digitalWrite(34, LOW); //Abilita escrita no pino 34
    delayMicroseconds(10);
    digitalWrite(34, HIGH); //Desabilita escrita no pino 34

    //CENTENA ELEVAÇÃO------------------------------------------------------------------
    //bit D(MSB)
    //Se centena de elevação for 1
    if (cELE == true) { //escreve 1
        digitalWrite(32, LOW);
        digitalWrite(46, HIGH);
        digitalWrite(50, LOW);
        digitalWrite(28, LOW);
    } else { //apaga display
        digitalWrite(32, HIGH);
        digitalWrite(46, HIGH);
        digitalWrite(50, HIGH);
        digitalWrite(28, HIGH);
    }

    digitalWrite(48, LOW); //Abilita escrita no pino 48
    delayMicroseconds(10);
    digitalWrite(48, HIGH); //Desabilita escrita no pino 48

    //DEZENA ELEVAÇÃO ------------------------------------------------
    //Se centena e dezena for 0

    if (cELE == false && DELE == true) {// Se centena ELE não for 1 e dezena ELE for 0
        //escreve o valor 15 para apagar o display de dezena
        digitalWrite(28, HIGH);
        digitalWrite(50, HIGH);
        digitalWrite(46, HIGH);
        digitalWrite(32, HIGH);
    } else {
        //bit D(MSB)
        if (dbits[5][3] == 1) {
            digitalWrite(32, HIGH);
        } else {
            digitalWrite(32, LOW);
        }

        //bit C
        if (dbits[5][2] == 1) {
            digitalWrite(46, HIGH);
        } else {
            digitalWrite(46, LOW);
        }

        //bit B
        if (dbits[5][1] == 1) {
            digitalWrite(50, HIGH);
        } else {
            digitalWrite(50, LOW);
        }

        //bit A(LSB)
        if (dbits[5][0] == 1) {
            digitalWrite(28, HIGH);
        } else {
            digitalWrite(28, LOW);
        }
    }

    digitalWrite(40, LOW); //Abilita escrita no pino 40
    delayMicroseconds(10);
    digitalWrite(40, HIGH); //Desabilita escrita no pino 40

    //UNIDADE ELEVAÇÃO ------------------------------------------------
    //bit D(MSB)
    if (dbits[6][3] == 1) {
        digitalWrite(32, HIGH);
    } else {
        digitalWrite(32, LOW);
    }

    //bit C
    if (dbits[6][2] == 1) {
        digitalWrite(46, HIGH);
    } else {
        digitalWrite(46, LOW);
    }

    //bit B
    if (dbits[6][1] == 1) {
        digitalWrite(50, HIGH);
    } else {
        digitalWrite(50, LOW);
    }

    //bit A(LSB)
    if (dbits[6][0] == 1) {
        digitalWrite(28, HIGH);
    } else {
        digitalWrite(28, LOW);
    }

    digitalWrite(36, LOW); //Abilita escrita no pino 36
    delayMicroseconds(10);
    digitalWrite(36, HIGH); //Desabilita escrita no pino 36

    //FRAÇÃO ELEVAÇÃO ------------------------------------------------

    //bit D(MSB)
    if (dbits[7][3] == 1) {
        digitalWrite(32, HIGH);
    } else {
        digitalWrite(32, LOW);
    }

    //bit C
    if (dbits[7][2] == 1) {
        digitalWrite(46, HIGH);
    } else {
        digitalWrite(46, LOW);
    }

    //bit B
    if (dbits[7][1] == 1) {
        digitalWrite(50, HIGH);
    } else {
        digitalWrite(50, LOW);
    }

    //bit A(LSB)
    if (dbits[7][0] == 1) {
        digitalWrite(28, HIGH);
    } else {
        digitalWrite(28, LOW);
    }

    digitalWrite(30, LOW); //Abilita escrita no pino 30
    delayMicroseconds(10);
    digitalWrite(30, HIGH); //Desabilita escrita no pino 30
    //FIM DA ESCRITA NO DISPLAY -----------------------------------------------------------------------------------
}

void setup() {
    //Iniciar porta serial
    Serial.begin(9600);

    //Configuração de pinos de leitura de posição atual Azimute e Elevação (Conector S8 - Gaveta de pilotage)
    pinMode(53, INPUT); //LSB
    pinMode(51, INPUT);
    pinMode(49, INPUT);
    pinMode(47, INPUT);
    pinMode(45, INPUT);
    pinMode(43, INPUT);
    pinMode(41, INPUT);
    pinMode(39, INPUT);
    pinMode(37, INPUT);
    pinMode(35, INPUT);
    pinMode(33, INPUT);
    pinMode(31, INPUT);
    pinMode(29, INPUT);
    pinMode(27, INPUT);
    pinMode(25, INPUT);
    pinMode(23, INPUT); //MSB


    //Configuração dos pinos de controle do display da gaveta de comando
    //Azimute
    pinMode(34, OUTPUT);//parte fracionaria AZ
    pinMode(38, OUTPUT);//parte unitaria AZ
    pinMode(44, OUTPUT);//parte das dezenas AZ
    pinMode(52, OUTPUT);//parte das centenas AZ

    //Elevação
    pinMode(30, OUTPUT);//parte fracionaria ELE
    pinMode(36, OUTPUT);//parte unitaria ELE
    pinMode(40, OUTPUT);//parte das dezenas ELE
    pinMode(48, OUTPUT);//parte das centenas ELE

    //Bits
    pinMode(32, OUTPUT);//MSB
    pinMode(46, OUTPUT);
    pinMode(50, OUTPUT);
    pinMode(28, OUTPUT);//LSB

    //estado inicial dos pinos do display desabilitados
    for (i = 28; i < 54; i = i + 2) { // OBS: o pino 42 em HIGH desabilita o teste dos
        digitalWrite(i, HIGH);          // leds do display
    }

    //Defeito azimute e elevação(Conector S7 - Gaveta de pilotage)
    pinMode(12, INPUT_PULLUP); //Azimute
    pinMode(11, INPUT_PULLUP); //Elevação

    //Status bobinas KA1, KA2 e KA4 (Conector S7 - Gaveta de pilotage)
    pinMode(10, INPUT_PULLUP); //KA1 -> Botão de emergência acionado (Status: Pino)
    pinMode(8, INPUT_PULLUP); //KA2 -> Fim de curso, Módulo de controle e KA1
    pinMode(9, INPUT_PULLUP); //KA4 -> Teste de LEDs

    //ativação do controle manual
    pinMode(7, INPUT_PULLUP);

    //Configuração dos leds
    pinMode(6, OUTPUT); //LED parada de emergência
    pinMode(4, OUTPUT); //LED potência
    pinMode(14, OUTPUT); //LED falha inversor
    pinMode(15, OUTPUT); //LED rastreio
    pinMode(16, OUTPUT); //LED parada final de curso
    pinMode(17, OUTPUT); //LED manual

    //Estado inicial dos LEDs desligado
    digitalWrite(6, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(14, HIGH);
    digitalWrite(15, HIGH);
    digitalWrite(16, HIGH);
    digitalWrite(17, HIGH);

    //Liberar leitura azimute e elevação (Conector S8 - Gaveta de pilotage)
    pinMode(22, OUTPUT); //azimute
    pinMode(24, OUTPUT); //elevação

    //estado inicial da leitura desabilitada
    digitalWrite(22, HIGH);
    digitalWrite(24, HIGH);

    //Ligar gaveta de potencia (Conector S7 - Gaveta de pilotage)
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW); //estado inicial do sistema desligado

    //Saidas PWM para inversor
    pinMode(2, OUTPUT); //AZ
    pinMode(3, OUTPUT); //ELE

    //Zera sinal de controle inicial
    analogWrite(2, (2.25 * (255.0 / 5.0)));
    analogWrite(3, (2.75 * (255.0 / 5.0)));

    watchD = millis();
    timer_display = millis();
    //45 PORTAS DIGITAIS UTILIZADAS

}

void loop() {
    //--------------------------ALGORITMO DE COMUNICAÇÃO---------------------------------
    //leitura da posição de referência
    while (Serial.available()) {
        // get the new byte:
        unsigned char inChar = (unsigned char) Serial.read();

        if (cont_bytes == 0) {
            SET    = false;
            POWER  = false;
            STATE  = false;
            ACK    = false;
            NACK   = false;
            endMsg = true;

            switch (inChar) {
                case 87://'W'
                    SET    = true;
                    endMsg = false;
                    break;
                case 80: //'P'
                    POWER = true;
                    break;
                case 83: //'S'
                    STATE = true;
                    break;
                case 65: //'A' -> comando state reconhecido pelo PC
                    ACK = true;
                    break;
                case 78: //'N' -> comando state não reconhecido pelo PC, envia novamente o comando
                    NACK = true;
                    STATE = true; // permite reenvio do comando STATE
                    cont_erro += 1; // incrementa contagem de erros de comunicação
                    break;
                default:
                    endMsg = false; //evitar de liberar algum comando aleatório
                    break;
            }
        }

        if (cont_bytes < 6) {
            if (STATE == false && POWER == false && ACK == false && NACK == false) { // feito para evitar erros nas proximas string de mensagem
                inputString[cont_bytes] = inChar;
                cont_bytes = cont_bytes + 1;
            }
        }

        if (cont_bytes >= 6) {
            endMsg = true;
        }

        if (endMsg == true) {
            cont_bytes = 0;
        }
    }

    if(STATE == true && endMsg == true) {
        temp = round((AZ + 20.0) * (65535.0 / 400.0)); //SIMULAÇÃO
        a0 = temp & 255;
        a1 = temp >> 8;

        // ELE + 90 para evitar transição brusca entre leituras (0 -> 359)
        temp = round((ELE + 90.0) * (65535.0 / 360.0)); //SIMULAÇÃO
        e0 = temp & 255;
        e1 = temp >> 8;

        verifica_status(&state);

        stateString[0] = 87;
        stateString[1] = a1;
        stateString[2] = a0;
        stateString[3] = e1;
        stateString[4] = e0;
        stateString[5] = state;

        stateString[6] = stateString[0] ^ stateString[1] ^ stateString[2] ^ stateString[3] ^ stateString[4] ^ stateString[5];

        for (bytes = 0; bytes < 7; bytes++) {
            Serial.write(stateString[bytes]);
        }

        //Atualiza o valor do contador de tempo
        watchD = millis();

        //Limpa string
        for(cont=0;cont<6;cont++){
            inputString[cont] = 0;
        }

        endMsg = false;
        STATE = false;
    }

    if(POWER == true && endMsg == true) {

        rastreio = !rastreio;

        //Atualiza verificador de comunicação
        watchD = millis();

        //envia ACK 'A'
        Serial.write(65);

        //Limpa string
        for(cont=0;cont<6;cont++){
            inputString[cont] = 0;
        }
        POWER = false;
        endMsg = false;
    }

    if(SET == true && endMsg == true) {

        erro_aknow = verifica_checksum();
        cont_erro += erro_aknow;

        //se o checksum estiver correto, executa SET e envia ACK
        if(erro_aknow == 0) {

            //envia ACK
            Serial.write(65);

            a1 = inputString[1];
            a0 = inputString[2];

            e1 = inputString[3];
            e0 = inputString[4];

            temp = a1 << 8;
            temp = temp | a0;

            refAZ = temp * (360.0 / 65535.0);

            if(refAZ>=360.0)
                refAZ-= 360.0;

            temp = e1<<8;
            temp = temp | e0;

            refELE = temp * (360.0 / 65536.0);
            if(refELE>=360.0)
                refELE-= 360.0;

            //Permite envio do sinal de controle
            outV = true;

            //Atualiza o valor do contador de tempo
            watchD = millis();
        } else {
            //Caso checksum esteja errado, envia NACK para receber novamente comando SET
            Serial.write(78);
        }

        //limpa string de entrada e desativa comando SET e fim de mensagem
        //Limpa string
        for(cont=0;cont<6;cont++){
            inputString[cont] = 0;
        }
        SET = false;
        endMsg = false;

    }

    //--------------------------FIM DO ALGORITMO DE COMUNICAÇÃO---------------------------------
    //--------------------------TESTE DE LEDS---------------------------------
    if ( (digitalRead(9) == HIGH) && (rastreio == false) ) { //teste de leds solicitado
        //O teste só pode ser feito quando a antena estiver fora de rastreio
        digitalWrite(6, LOW);
        digitalWrite(4, LOW);
        digitalWrite(14, LOW);
        digitalWrite(15, LOW);
        digitalWrite(17, LOW);
        digitalWrite(16, LOW);
        digitalWrite(42, LOW);
    } else {
        digitalWrite(6, HIGH);
        digitalWrite(4, HIGH);
        digitalWrite(14, HIGH);
        digitalWrite(15, HIGH);
        digitalWrite(16, HIGH);
        digitalWrite(17, HIGH);
        digitalWrite(42, HIGH);
    }

    //se KA1 e KA2 estiverem operando, ligar led potência
    if (digitalRead(10) == LOW && digitalRead(8) == HIGH) {
        digitalWrite(4, LOW); //ligar LED potência
    } else {
        digitalWrite(4, HIGH); //desliga LED potência
    }

    //---------------------LEITURA DA POSIÇÃO DA ANTENA----------------------------------
    //leitura da posição atual da antena e conversão para referência dela
    //leitura do azimute
    AZres = readAngle(0);

    //leitura da elevação
    ELEres = readAngle(1);

    if (ELEres > 360) { //angulo do centro da descontinuidade do resolver
        ELEres = ELEres - 360;
    }

    //calculo da posição atual da antena (mecânica) a partir da posição dos resolvers
    AZ = ((360 - AZres) - 15) * (96.0 / 88.0);
    ELE = ((360 - ELEres) - 34.0) * (80.0 / 130.0);

    //escrever posição atual no display de leds a cada 100mS
    if ((millis() - timer_display) > 200) {
        display_gaveta(AZ, ELE);
        timer_display = millis();
    }
    //------------------------------------------------------------------------------

    // Verificador: se o arduino passar mais que 3 segundos sem receber comandos
    // a potência do sistema será desligada
    if (((millis() - watchD) > 3000) && (rastreio == true) ) {
        rastreio = false;
    }

    //----------------------------CONTROLE MANUAL------------------------------------
    //Verificação de controle manual
    if (digitalRead(7) == LOW) {
        //Se o interruptor de controle manual for acionado
        //Ligar sistema
        digitalWrite(5, HIGH); //liga modulo de potencia
        digitalWrite(17, LOW); //liga led manual
        manual = true;
        manual_control();
    } else {
        manual = false;
        digitalWrite(17, HIGH); //Desliga led manual

        //Desligar sistema e zerar sinal de controle se não estiver em rastreio
        if (rastreio == false) {
            digitalWrite(5, LOW); //desliga modulo de potencia
            //Zerar sinal de saida pro inversor
            analogWrite(2, (2.5 * (255.0 / 5.0)));
            analogWrite(3, (2.5 * (255.0 / 5.0)));
        }
    }
    //-------------------------------------------------------------------------------

    //------------------------CONDIÇÃO PARA RASTREIO---------------------------------
    //Condição para ligar o sistema
    if (rastreio == true) {
        //liga LED rastreio
        digitalWrite(15, LOW);
        //Ligar sistema
        digitalWrite(5, HIGH); //liga modulo de potencia
    } else if (manual == false && rastreio == false) {
        //Desliga LED rastreio
        digitalWrite(15, HIGH);

        //FIM de rastreio - Desligar potência
        digitalWrite(5, LOW); //Desliga modulo de potencia
        outvELE = 0.0;
        outvAZ = 0.0;
        analogWrite(2, (2.5 * (255.0 / 5.0)));
        analogWrite(3, (2.5 * (255.0 / 5.0)));
        IAZ = 0.0;
        IELE = 0.0;
        refAZ = 180.0;
        refELE = 90.0;
    }
    //------------------------------------------------------------------------------

    //--------------------------CALCULO DO SINAL DE CONTROLE---------------------------------
    if ((rastreio == true) && (outV == true)  &&  (manual == false)) {
        //Permite execução do controle se tiver chegado alguma nova referencia (outV == true)
        //Feito para garantir controle a cada 1 sec
        //Se manual == true -> não permite envio do sinal de controle PI caso o controle manual esteja ativo
        //Incluir proteção contra erros muito elevados

        //ALGORITMO DE CONTROLE PARA AZIMUTE - - - - - - - - - - - - - - - - - - - - - -
        erroAZ = refAZ - AZ;
        //Limita erro de controle
        //if((erroAZ > 20) || (erroAZ < -20))erroAZ = erroaAZ;

        IAZ = IAZ + (Ta * kiAZ) * erroaAZ;

        //Tecnica anti-reset windup
        if (IAZ > IsatAZ)IAZ = IsatAZ;
        if (IAZ < -IsatAZ)IAZ = -IsatAZ;

        outvAZ = kpAZ * erroAZ + IAZ;
        //Limitacao do sinal de controle
        if (outvAZ > 2.5) outvAZ = 2.5;
        if (outvAZ < -2.5) outvAZ = -2.5;

        outvAZ = outvAZ * (-1);
        erroaAZ = erroAZ;

        //ALGORITMO DE CONTROLE PARA ELEVAÇÃO - - - - - - - - - - - - - - - - - - - - - - -
        erroELE = refELE - ELE;
        //if((erroELE > 20)|| (erroELE < -20))erroELE = erroaELE;
        IELE = IELE + (Ta * kiELE) * erroaELE;

        //Tecnica anti-reset windup
        if (IELE > IsatELE) IELE = IsatELE;
        if (IELE < -IsatELE)IELE = -IsatELE;

        outvELE = kpELE * erroELE + IELE;
        //limitacao do sinal de controle p/ elevação
        if (outvELE > 2.5) outvELE = 2.5;
        if (outvELE < (-2.5)) outvELE = -2.5;

        erroaELE = erroELE;

        //Sinal de saida pro inversor
        analogWrite(2, ((outvAZ + 2.5) * (255.0 / 5.0))); //AZ
        analogWrite(3, ((outvELE + 2.5) * (255.0 / 5.0))); //ELE

        outV = false;

    }
    //-------------------------------------------------------------------------------------
}//end loop
