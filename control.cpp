#include "control.h"

char sat_name[3][20]; //Matriz que armazenará o nome dos satelites dos arquivos carregados
int n_pass[3] = {0}; //Vetor que armazenará o numero de passagens em cada arquivo
int i = 0; //variavel auxiliar de contagem
float max_ele = 0.0; //variaveis para armazenagem dos dados da passagem
float durat = 0.0;
float range = 0.0;
Efem *passagem; //Ponteiro que apontará para as passagens que serão alocadas
time_t start = 0; //Horario de inicio da passagem
time_t stop = 0;  //Horario de término da passagem
time_t time_aux = 0; //Variavel auxiliar
struct tm *horario;
bool find_pass = false;
bool pass_start = false; //Variavel utilizada para saber se a passagem atual já começou

//Variaveis para cronometro
double seconds = 0.0;
int aux_time = 0;
int hor = 0, sec = 0, min = 0;

//Variaveis de testes
char satelite[30];

//VARIAVEIS DO ALGORITMO DE CONTROLE E COMUNICAÇÃO

float refAZ = 0.0, refELE = 0.0; //Posições de referência da antena (valor obtido de efem.get();)
float AZ = 0.0, ELE = 0.0; //Posição atual da antena (recebido do arduino e decodificada)
time_t ef_time; // Horário da proxima efeméride em segundos desde 1 de janeiro de 1970 (Unix time)

unsigned char a1 = 0, a0 = 0, e1 = 0, e0 = 0; //bytes a serem enviados (refAZ e refELE)
int cont_aux = 0;

// Por que não usar 'S', 'P', 'A', 'N', 'X' direto???
unsigned char setString[6] = {87,48,48,48,48,48}; //Array onde será formado o comando SET a ser enviado para o arduino
unsigned char input_state[7] = {87,48,48,48,48,48,0}; //Array de comando STATE a ser recebido do arduino
unsigned char state[1] = { 83 }; // 'S'
unsigned char power[1] = { 80 }; //'P'
unsigned char ack[1] = { 65 }; // 'A'
unsigned char nack[1] = { 78 }; // 'N'
unsigned char input_ack[1] = { 88 };// 'X' //será utilizada para verificar reconhecimento de mensagens enviadas e recebidas
int erro_ack = 0; //Variavel auxiliar para detecção de erros

//arrays que armazenarão rampas de posicionamento inicial e final
float rampa_AZ[30] = { 0 };
float rampa_ELE[30] = { 0 };

//contador de erros de comunicação
int cont_erro = 0;

//Status do sistema
int ka1 = 0, ka2 = 0, daz = 0, del = 0, m = 0, p = 0;

FILE *arq;

//NOME DA PORTA SERIAL UTILIZADA
//const wchar_t port_name[] = L"COM3"; //COLOCAR NOME DA PORTA SERIAL ENTRE AS ASPAS

//Cria objeto da porta serial
CSerial serial;

Control::Control(const wchar_t* port) {
    //INICIA COMUNICAÇÃO SERIAL
    serial.Open(port);
    //Configura comunicação: taxa de 9600 bps, byte com 8 bits, sem bit de paridade, 1 bit de parada
    serial.Setup(CSerial::EBaud9600, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
    // Configura porta serial para na hora de ler bytes esperar até o numero solicitado, passado como argumento da função
    serial.SetupReadTimeouts(serial.EReadTimeoutBlocking);

    if (serial.IsOpen() == true) {
        qDebug() << "Porta serial conectada" << "(" << QString::fromStdWString(port) << ")";
    } else {
        qDebug() << "Erro ao abrir porta" << "(" << QString::fromStdWString(port) << ")";
        return; // TODO: substituir por throw error
    }
}

int Control::cont_passagem(char *arq_name, char *_sat_name)
{
    FILE *_arq = fopen(arq_name, "r");
    int dia1, mes1, ano1, mil1, hora1, min1, sec1;
    int dia2, mes2, ano2, mil2, hora2, min2, sec2;
    float max_ele = 0.0, durat = 0.0;
    int aux = 0, pass = 0;
    char byte;

    //Verifica se o arquivo foi aberto corretamente. Caso contrario retorna -1
    if (_arq == NULL) {
        return -1;
    }

    //pula linha da data
    aux = 0;
    while (aux != 1) {
        fscanf(_arq, "%c", &byte);
        if (byte == '\n')
            aux += 1;
    }

    // Faz a leitura do nome do satelite
    // Acho que os 28 primeiros caracteres são sempre "Place-INPE..."
    for (aux = 0; aux < 28; aux++) {
        fscanf(_arq, "%c", &byte);
    }

    fscanf(_arq, "%s", _sat_name);

    //Limpa caracteres nao pertencentes ao nome do satelite
    for (aux = 7; aux < 20; aux++) {
        _sat_name[aux] = NULL;
    }

    //pula 4 linhas
    aux = 0;
    while (aux != 5) {
        fscanf(_arq, "%c", &byte);
        if (byte == '\n')
            aux += 1;
    }

    //Faz a contagem do numero de passagens
    while (fscanf(_arq, "%d/%d/%d %d:%d:%d.%d    %d/%d/%d %d:%d:%d.%d           %f                 %f\n",
        &dia1, &mes1, &ano1, &hora1, &min1, &sec1, &mil1, &dia2, &mes2, &ano2, &hora2, &min2, &sec2, &mil2, &durat, &max_ele) != 0) {

        //incrementa o numero de passagens se a passagem ainda não tiver ocorrido
        pass += 1;
    }

    fclose(_arq);

    return pass;
}

int Control::load_efem(char *_arq_name, int _n_pass0, int _n_pass1, char *_sat_name)
{
    int dia1, mes1, ano1, mil1, hora1, min1, sec1;
    int dia2, mes2, ano2, mil2, hora2, min2, sec2;
    float max_ele1 = 0.0, durat1 = 0.0;
    float az = 0.0, ele = 0.0, range = 0.0;
    int cont_efem = 0, cont_pass = 0;
    int aux = 0;
    char byte;

    //struct tm start2, stop2;
    time_t start1, stop1;
    struct tm *_time, start_t, stop_t;

    time(&start1);
    time(&stop1);

    _time = localtime(&start1);
    start_t = *_time;
    stop_t = *_time;

    FILE *_arq = fopen(_arq_name, "r");

    //Verifica se o arquivo foi aberto corretamente. Caso contrario retorna -1
    if (_arq == NULL) {
        return -1;
    }

    //pula cabeçalho (6 linhas)
    aux = 0;
    while (aux != 6) {
        fscanf(_arq, "%c", &byte);
        if (byte == '\n')
            aux += 1;
    }

    //LEITURA DA PRIMEIRA SEÇÃO DO ARQUIVO (Horario de inicio e fim, duração e máxima elevação)
    //É feita a leitura e em seguida os dados de cada passagem são armazenados no respectivo
    //objeto "passagem". Ex: o arquivo contém 10 passagens. É feita a leitura e armazenado em
    //passagem[0], passagem[1], ..., passagem[9].
    aux = _n_pass0;
    while (fscanf(_arq, "%d/%d/%d %d:%d:%d.%d    %d/%d/%d %d:%d:%d.%d           %f                 %f\n",
                         &dia1, &mes1, &ano1, &hora1, &min1, &sec1, &mil1,
                         &dia2, &mes2, &ano2, &hora2, &min2, &sec2, &mil2,
                         &durat1, &max_ele1) != 0) {

        // atribui o horario da passagem para o ponteiro _time
        _time->tm_year = ano1 - 1900;
        _time->tm_mon = mes1 - 1;
        _time->tm_mday = dia1;
        _time->tm_hour = hora1;
        _time->tm_min = min1;
        _time->tm_sec = sec1;
        _time->tm_isdst = 0;

        //Copia o conteudo do ponteiro _time para o struct start_t
        start_t = *_time;
        //transforma o struct do horario da passagem em um time_t (start1)
        //Remove 10800 segundos (60s * 60min * 3h) devido ao horario no arquivo ser no formato UTC
        start1 = mktime(&start_t) - 10800;

        //atribui o horario de fim da passagem do satelite para stop_t
        _time->tm_year = ano2 - 1900;
        _time->tm_mon = mes2 - 1;
        _time->tm_mday = dia2;
        _time->tm_hour = hora2;
        _time->tm_min = min2;
        _time->tm_sec = sec2;
        _time->tm_isdst = 0;

        //Copia o conteudo do ponteiro _time para o struct stop_t
        stop_t = *_time;

        //transforma o struct do horario da passagem em um time_t (stop1)
        //Remove 10800 segundos (60s * 60min * 3h) devido ao horario no arquivo ser no formato UTC
        stop1 = mktime(&stop_t) - 10800;

        //printf("funcao1  -----> %d   %d", start1, stop1);
        //printf("    %f    %f\n", durat1, max_ele1);
        _time = &start_t;
        //printf("struct start: %d/%d/%d %d:%d:%d\n", _time->tm_mday, _time->tm_mon + 1, _time->tm_year + 1900, _time->tm_hour, _time->tm_min, _time->tm_sec);
        _time = &stop_t;
        //printf("struct stop: %d/%d/%d %d:%d:%d\n", _time->tm_mday, _time->tm_mon + 1, _time->tm_year + 1900, _time->tm_hour, _time->tm_min, _time->tm_sec);

        //Passa as informações da sessão 1 para um objeto Efem se a passagem ainda não tiver ocorrido
        passagem[aux].set_data(start1, stop1, durat1, max_ele1);
        passagem[aux].set_name(_sat_name);
        aux += 1;
    }

    //LEITURA DA SEGUNDA SEÇÃO DO ARQUIVO (Efemérides)
    for (cont_pass = _n_pass0; cont_pass < _n_pass1; cont_pass++) {
        //Pula duas linhas
        aux = 0;
        while (aux != 2) {
            fscanf(_arq, "%c", &byte);
            if (byte == '\n')
                aux += 1;
        }

        cont_efem = 0;
        if (cont_pass < _n_pass1 - 1) {
            while (fscanf(_arq, "%d/%d/%d %d:%d:%d.%d          %f              %f    %f\n",
                &dia1, &mes1, &ano1, &hora1, &min1, &sec1, &mil1, &az, &ele, &range) != 0) {

                // atribui o horario da efemeride para o ponteiro _time
                _time->tm_year = ano1 - 1900;
                _time->tm_mon = mes1 - 1;
                _time->tm_mday = dia1;
                _time->tm_hour = hora1;
                _time->tm_min = min1;
                _time->tm_sec = sec1;
                _time->tm_isdst = 0;

                //Copia o conteudo do ponteiro _time para o struct start_t
                start_t = *_time;
                //transforma o struct do horario da efemeride em um time_t (start1)
                //Remove 10800 segundos devido ao horario no arquivo ser no formato UTC
                start1 = mktime(&start_t) - 10800;

                //envia informações da efemeride atual para a referida passagem
                passagem[cont_pass].set_efem(start1, az, ele, range, cont_efem);
                cont_efem += 1;

            }//End while
        }//End if
        else {
            while (fscanf(_arq, "%d/%d/%d %d:%d:%d.%d          %f              %f    %f\n",
                &dia1, &mes1, &ano1, &hora1, &min1, &sec1, &mil1, &az, &ele, &range) != EOF) {

                // atribui o horario da efemeride para o ponteiro _time
                _time->tm_year = ano1 - 1900;
                _time->tm_mon = mes1 - 1;
                _time->tm_mday = dia1;
                _time->tm_hour = hora1;
                _time->tm_min = min1;
                _time->tm_sec = sec1;
                _time->tm_isdst = 0;

                //Copia o conteudo do ponteiro _time para o struct start_t
                start_t = *_time;
                //transforma o struct do horario da efemeride em um time_t (start1)
                //Remove 10800 segundos devido ao horario no arquivo ser no formato UTC
                start1 = mktime(&start_t) - 10800;

                //envia informações da efemeride atual para a referida passagem
                passagem[cont_pass].set_efem(start1, az, ele, range, cont_efem);
                cont_efem += 1;

            }//end while
        }//end if
    }//end for

    fclose(_arq);

    return 0;
}

void Control::tratar_efem(int _n_pass)
{
    int cont = 0;

    for (cont = 0; cont < _n_pass; cont++) {
        passagem[cont].change_efem();

    }
}

void Control::ordenar_passagens(int _n_pass)
{
    int cont1 = 0, cont2 = 0;
    Efem temp;

    // organiza as passagens de acordo com a ordem cronológica crescente
    // (bubble sort)
    for (cont1 = 0; cont1 < _n_pass; cont1++) {
        for (cont2 = cont1 + 1; cont2 < _n_pass; cont2++) {
            if (passagem[cont1] > passagem[cont2]) {
                temp = passagem[cont2];
                passagem[cont2] = passagem[cont1];
                passagem[cont1] = temp;
            }
        }
    }
}

void Control::send_set(float _refAZ, float _refELE, unsigned char *_setString, int *_cont_erro)
{
    //auxiliar para conversão de float para int
    unsigned short temp = 0;

    printf("\nrefAZ:%f\nrefELE: %f\n\n", _refAZ, _refELE);

    //Gerar bytes A1, A0, E1, E0
    _refAZ = _refAZ *(65535.0 / 360.0);
    temp = round(_refAZ);

    a0 = temp & 255;
    a1 = temp >> 8;

    _refELE = _refELE * (65535.0 / 360.0);
    temp = round(_refELE);

    e0 = temp & 255;
    e1 = temp >> 8;

    //Gera string
    _setString[0] = 87;
    _setString[1] = a1;
    _setString[2] = a0;
    _setString[3] = e1;
    _setString[4] = e0;

    //Calcula checksum
    _setString[5] = _setString[0] ^ _setString[1] ^ _setString[2] ^ _setString[3] ^ _setString[4];

    printf("setString: {%d, %d, %d, %d, %d, %d} \n\n", setString[0], setString[1], setString[2], setString[3], setString[4], setString[5]);

    //loop para envio do comando SET e recepção da resposta do arduino
    //A resposta será ACK ou NACK, caso o arduino reconheça ou não, respectivamente
    //Caso o PC receba NACK, ele reenviará o comando SET
    erro_ack = 1;
    while (erro_ack != 0) {
        //envia comando set
        serial.Write(setString, 6);

        //Recebe comando ACK ou NACK vindo do arduino e armazena código de erro
        erro_ack = reconhecimento_arduino(input_ack);

        //Incrementa erro (se houver)
        *_cont_erro += erro_ack;
    }

}

void Control::send_state(unsigned char *_input_state, float *_AZ, float *_ELE, int *_cont_erro)
{
    //variavel auxiliar de 16 bits
    unsigned short temp = 0;

    //envia comando STATE
    serial.Write(state, 1);
    erro_ack = 1;

    //Loop para recepção e verificação de erro na mensagem
    //O programa irá receber a mensagem e verificar se contém erro
    //Caso não contenha eviará ACK para o arduino e continuará
    //Caso contenha erro, enviará NACK e esperará pela mensagem novamente
    while (erro_ack != 0) {
        //recebe comando STATE
        serial.Read(_input_state, 7);

        //verifica checksum: retorna 0 se estiver OK
        erro_ack = verifica_checksum();

        //envia ack ou nack dependendo do erro do checksum
        envia_reconhecimento(erro_ack);

        //incrementa contagem de erros (se houver)
        *_cont_erro = *_cont_erro + erro_ack;
    }

    printf("Input String: {%d, %d, %d, %d, %d, %d, %d}\n\n", _input_state[0], _input_state[1], _input_state[2],
        _input_state[3], _input_state[4], _input_state[5], _input_state[6]);

    //Decodifica posições AZ e ELE
    a0 = _input_state[2];
    a1 = _input_state[1];

    temp = a1 << 8;
    temp = temp | a0;

    *_AZ = temp * (360.0 / 65535.0);

    e0 = _input_state[4];
    e1 = _input_state[3];

    temp = e1 << 8;
    temp = temp | e0;

    *_ELE = temp * (360.0 / 65535.0);

    printf("AZ_antena = %f\nELE_antena = %f\n\n", *_AZ, *_ELE);

    //Decodificação do STATUS do sistema contido no byte 5 da mensagem
    m = (_input_state[5]) % 2;
    del = ((_input_state[5]) >> 1) % 2;
    daz = ((_input_state[5]) >> 2) % 2;
    ka2 = ((_input_state[5]) >> 3) % 2;
    ka1 = ((_input_state[5]) >> 4) % 2;
    p = ((_input_state[5]) >> 7);

    printf("\nP:%d  KA1: %d KA2: %d DAZ: %d  DEL: %d  M: %d  Erros: %d\n\n", p, ka1, ka2, daz, del, m, erro_ack);
    printf("Erros detectados: %d\n\n", cont_erro);

}

void Control::send_power(void)
{
    erro_ack = 1;
    while (erro_ack != 0) {
        //Envia POWER
        serial.Write(power, 1);

        serial.Read(input_ack, 1);

        if (input_ack[0] == 'A') {
            erro_ack = 0;
        }
        else {
            erro_ack = 1;
        }

    }// end While
} //end function

int Control::verifica_checksum()
{
    //verifica checksum
    if ((input_state[0] ^ input_state[1] ^ input_state[2] ^ input_state[3] ^ input_state[4] ^ input_state[5]) == input_state[6]) {
        return 0;
    } else {
        return 1;
    }

}

void Control::envia_reconhecimento(int _erro) {

    if (_erro == 0) {
        serial.Write(ack, 1);

    } else {
        serial.Write(nack, 1);

    }

}

int Control::reconhecimento_arduino(unsigned char *_input_ack)
{
    serial.Read(_input_ack, 1);
    if (_input_ack[0] == 'A') { //reconhecido
        return 0;
    } else {
        return 1;
    }

}

void Control::gerar_rampa(float *_rampa_AZ, float *_rampa_ELE)
{
    float mAZ = 0, mELE = 0;
    int k = 0;

    mAZ = (refAZ - AZ) / 30.0;
    mELE = (refELE - ELE) / 30.0;

    for (k = 1; k <= 30; k++) {
        _rampa_AZ[k - 1] = AZ + k*mAZ;
        _rampa_ELE[k - 1] = ELE + k*mELE;
    }
}

void Control::runProgram() {
    /*
    //INICIA COMUNICAÇÃO SERIAL
    serial.Open(port_name);
    //Configura comunicação: taxa de 9600 bps, byte com 8 bits, sem bit de paridade, 1 bit de parada
    serial.Setup(CSerial::EBaud9600, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
    // Configura porta serial para na hora de ler bytes esperar até o numero solicitado, passado como argumento da função
    serial.SetupReadTimeouts(serial.EReadTimeoutBlocking);

    if (serial.IsOpen() == true) {
        printf("Porta serial conectada\n\n");
    } else {
        printf("Erro ao abrir porta serial\n");
        return; // TODO: substituir por throw error
    }

    //Envia comando STATE para saber STATUS atual do sistema
    // Ou seja, "get" state
    send_state(input_state, &AZ, &ELE, &cont_erro);

    //Recebe a primeira efeméride da passagem
    //passagem[cont_pass].get_efem(&ef_time, &refAZ, &refELE, &range);

    //liga sistema e recebe estado atual da antena
    send_power();
    send_state(input_state, &AZ, &ELE, &cont_erro);

    //gera rampa de posicionamento inicial
    gerar_rampa(rampa_AZ, rampa_ELE);

    //Envia sinal de controle com a rampa inicial
    for (cont_aux = 1; cont_aux < 30; cont_aux++) {
        //while (time(NULL) < (ef_time - 30 + cont_aux));
        //system("cls");
        //printf("Informacoes da passagem atual:\nData: %d/%d/%d Hora: %d:%d:%d  duracao: %f  max_ele: %f  Sat_name: %s\n\n",
        //    horario->tm_mday, horario->tm_mon + 1, horario->tm_year + 1900, horario->tm_hour, horario->tm_min, horario->tm_sec, durat, max_ele, satelite);
        //printf("RAMPA DE POSICIONAMENTO INICIAL\n\n");
        send_set(rampa_AZ[cont_aux], rampa_ELE[cont_aux], setString, &cont_erro);
        //Sleep(20);
        send_state(input_state, &AZ, &ELE, &cont_erro);
    }

    // Enquanto tiver passagem
    passagem[cont_pass].get_efem(&ef_time, &refAZ, &refELE, &range);
    send_set(refAZ, refELE, setString, &cont_erro);
    send_state(input_state, &AZ, &ELE, &cont_erro); // para verificar erro
    gerar_rampa(180, 90); // posicionamento final

    // No fim
    send_power();
    Sleep(20);
    send_state(input_state, &AZ, &ELE, &cont_erro);


    printf("Fim das passagens carregadas!!\n");
    delete [] passagem;
    */
}
