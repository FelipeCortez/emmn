#include "control.h"

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

    send_power();
    setTarget(180, 90);
}

void Control::send_set(float az, float ele) {
    //auxiliar para conversão de float para int (??????)
    unsigned short temp = 0;

    //Gerar bytes A1, A0, E1, E0
    az = az *(65535.0 / 360.0);
    temp = round(az);

    a0 = temp & 255;
    a1 = temp >> 8;

    ele = ele * (65535.0 / 360.0);
    temp = round(ele);

    e0 = temp & 255;
    e1 = temp >> 8;

    //Gera string
    setString[0] = 87;
    setString[1] = a1;
    setString[2] = a0;
    setString[3] = e1;
    setString[4] = e0;

    //Calcula checksum
    setString[5] = setString[0] ^ setString[1] ^ setString[2] ^ setString[3] ^ setString[4];

    /*
    printf("setString: {%d, %d, %d, %d, %d, %d} \n\n",
           setString[0],
            setString[1],
            setString[2],
            setString[3],
            setString[4],
            setString[5]);
    */

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
        //errors += erro_ack;
    }
}

QMap<QString, float> Control::send_state()
{
    QMap<QString, float> answer;
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
        serial.Read(input_state, 7);

        //verifica checksum: retorna 0 se estiver OK
        erro_ack = verifica_checksum();

        //envia ack ou nack dependendo do erro do checksum
        envia_reconhecimento(erro_ack);

        //incrementa contagem de erros (se houver)
        cont_erro = cont_erro + erro_ack;
    }

    /*
    printf("Input String: {%d, %d, %d, %d, %d, %d, %d}\n\n",
           input_state[0],
           input_state[1],
           input_state[2],
           input_state[3],
           input_state[4],
           input_state[5],
           input_state[6]);
           */

    //Decodifica posições AZ e ELE
    a0 = input_state[2];
    a1 = input_state[1];

    temp = a1 << 8;
    temp = temp | a0;

    float az = temp * (360.0 / 65535.0);

    e0 = input_state[4];
    e1 = input_state[3];

    temp = e1 << 8;
    temp = temp | e0;

    float ele = temp * (360.0 / 65535.0);

    answer["az"] = az;
    answer["ele"] = ele;

    //Decodificação do STATUS do sistema contido no byte 5 da mensagem
    m =    (input_state[5]) % 2;
    del = ((input_state[5]) >> 1) % 2;
    daz = ((input_state[5]) >> 2) % 2;
    ka2 = ((input_state[5]) >> 3) % 2;
    ka1 = ((input_state[5]) >> 4) % 2;
    p =   ((input_state[5]) >> 7);

    //printf("\nP:%d  KA1: %d KA2: %d DAZ: %d  DEL: %d  M: %d  Erros: %d\n\n", p, ka1, ka2, daz, del, m, erro_ack);
    //printf("Erros detectados: %d\n\n", cont_erro);

    return answer;
}

void Control::setTarget(float az, float ele) {
    targetAz = az;
    targetEle = ele;
}

float clip(float val, float max) {
    if(fabs(val) > max) {
        return max * (fabs(val) / val);
    } else {
        return val;
    }
}

void Control::moveToTarget() {
    QMap<QString, float> answerMap = send_state();
    az = answerMap.value("az");
    ele = answerMap.value("ele");

    float az_increment = targetAz - az;
    az_increment = clip(az_increment, 10);
    float ele_increment = targetEle - ele;
    ele_increment = clip(ele_increment, 10);
    send_set(az + az_increment, ele + ele_increment);
    //qDebug() << "az:" << az << "|" << "increment:" << az_increment;
    //qDebug() << "ele:" << ele << "|" << "increment:" << ele_increment;
    //qDebug() << "--";
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
        } else {
            erro_ack = 1;
        }
    }
}

int Control::verifica_checksum() {
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

/*
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
*/

/*
void Control::runProgram() {
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
}
    */
