#include "control.h"

FILE *arq;

//NOME DA PORTA SERIAL UTILIZADA
//const wchar_t port_name[] = L"COM3"; //COLOCAR NOME DA PORTA SERIAL ENTRE AS ASPAS

//Cria objeto da porta serial
CSerial serial;

Control::Control(const wchar_t* port, TrackerListModel *trackerListModel, QObject *parent)
    : QObject(parent)
    , controller(Controller::Schedule)
    , antennaTimer(this)
{
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
    antennaTimer.start(1000);
    setTarget(180, 90);

    this->trackerListModel = trackerListModel;

    connect(&antennaTimer,
            SIGNAL(timeout()),
            this,
            SLOT(updateSlot()));
}

Control::~Control() {
    send_power();
    serial.Close();
}

void Control::send_set(float az, float ele) {
    //auxiliar para conversão de float para int (??????)
    unsigned short temp = 0;

    //Gerar bytes A1, A0, E1, E0
    az = az * (65535.0 / 360.0);
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

AzEle Control::send_state()
{
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

    //Decodifica posições AZ e ELE
    a0 = input_state[2];
    a1 = input_state[1];

    unsigned short temp = 0;
    temp = a1 << 8;
    temp = temp | a0;

    float az = temp * (360.0 / 65535.0);

    e0 = input_state[4];
    e1 = input_state[3];

    temp = e1 << 8;
    temp = temp | e0;

    float ele = temp * (360.0 / 65535.0);

    AzEle antennaInfo;
    antennaInfo.azimuth = az;
    antennaInfo.elevation = ele;

    //Decodificação do STATUS do sistema contido no byte 5 da mensagem
    m =    (input_state[5]) % 2;
    del = ((input_state[5]) >> 1) % 2;
    daz = ((input_state[5]) >> 2) % 2;
    ka2 = ((input_state[5]) >> 3) % 2;
    ka1 = ((input_state[5]) >> 4) % 2;
    p =   ((input_state[5]) >> 7);

    //printf("\nP:%d  KA1: %d KA2: %d DAZ: %d  DEL: %d  M: %d  Erros: %d\n\n", p, ka1, ka2, daz, del, m, erro_ack);
    //printf("Erros detectados: %d\n\n", cont_erro);

    return antennaInfo;
}

void Control::setDeltas(float deltaAz, float deltaEle) {
    AzEle antennaInfo = send_state();
    az = antennaInfo.azimuth;
    ele = antennaInfo.elevation;

    if(az + deltaAz < 0 || az + deltaAz > 360) { deltaAz = 0; }
    if(ele + deltaEle < 0 || ele + deltaEle > 360) { deltaEle = 0; }

    setTarget(az + deltaAz, ele + deltaEle);
}

void Control::setTarget(float az, float ele) {
    targetAz = az;
    targetEle = ele;
}

void Control::setController(Controller controller) {
    this->controller = controller;
}

void Control::moveToTarget() {
    AzEle antennaInfo = send_state();
    az = antennaInfo.azimuth;
    ele = antennaInfo.elevation;

    float incrementAz = targetAz - az;
    incrementAz = Helpers::clip(incrementAz, 10);
    float incrementEle = targetEle - ele;
    incrementEle = Helpers::clip(incrementEle, 10);
    send_set(az + incrementAz, ele + incrementEle);
}

void Control::send_power(void) {
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

void Control::updateSlot() {
    if(controller == Controller::Schedule) {
        TimeSpan remaining = trackerListModel->allPasses.at(0).passDetails.aos - DateTime::Now(true);
        auto nextPass = trackerListModel->allPasses.at(0);
        float secondsRemaining = remaining.Ticks() / (1e6f); // microseconds to seconds
        float positioningTime = 60;

        bool qd = false;

        if(nextPass.tracker->getElevationForObserver() >= 0) {
            if(qd) { qDebug() << "Passando"; }
            double az = nextPass.tracker->getAzimuthForObserver();
            double ele = nextPass.tracker->getElevationForObserver();
            if(nextPass.passDetails.reverse) {
                az = fmod(az + 180, 360);
                ele = 180 - ele;
            }

            setTarget(az, ele);
        } else if(secondsRemaining <= positioningTime &&
                  secondsRemaining > 0) {
            if(qd) { qDebug() << "Interpolando para azimute inicial e elevação zero"; }
            double az = nextPass.tracker->getAzimuthForObserver();
            double ele = 0;
            if(nextPass.passDetails.reverse) {
                az = fmod(az + 180, 360);
                ele = 180 - ele;
            }

            setTarget(az, ele);
        } else {
            setTarget(180, 90);
        }
    }

    moveToTarget();
}
