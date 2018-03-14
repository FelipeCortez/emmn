#include "control.h"

Control::Control(QString port,
                 TrackerListModel *trackerListModel,
                 QObject *parent)
    : QObject(parent)
    , azOffset(0.0f)
    , eleOffset(0.0f)
    , validPort(false)
    , controlMode(ControlMode::None)
    , controllerTimer(this)
    , speedAz(0)
    , accelerationAz(0)
    , speedEle(0)
    , accelerationEle(0)
    , maxSpeed(7.0)
    , maxAcceleration(0.3)
    , powerStatus(false)
    //, timeOffset(0.0f)
{
    changePort(port);

    this->trackerListModel = trackerListModel;
    //this->logger = new Logger();

    connect(&controllerTimer,
            SIGNAL(timeout()),
            this,
            SLOT(updateSlot()));
}

Control::~Control() {
    if (validPort) {
        sendPower();
    }

    serial.Close();
}

void Control::sendPosition(float az, float ele) {
    //auxiliar para conversão de float para int
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
    int erro_ack = 1;
    while (erro_ack != 0) {
        //envia comando set
        serial.Write(setString, 6);

        //Recebe comando ACK ou NACK vindo do arduino e armazena código de erro
        erro_ack = acknowledge(inputAck);

        //Incrementa erro (se houver)
        //errors += erro_ack;
    }
}

AzEle Control::getState() {
    //envia comando STATE
    serial.Write(state, 1);
    int erro_ack = 1;

    //Loop para recepção e verificação de erro na mensagem
    //O programa irá receber a mensagem e verificar se contém erro
    //Caso não contenha eviará ACK para o arduino e continuará
    //Caso contenha erro, enviará NACK e esperará pela mensagem novamente
    while (erro_ack != 0) {
        //recebe comando STATE
        serial.Read(inputState, 7);

        //verifica checksum: retorna 0 se estiver OK
        erro_ack = verifyChecksum();

        //envia ack ou nack dependendo do erro do checksum
        sendAck(erro_ack);
    }

    //Decodifica posições AZ e ELE
    a0 = inputState[2];
    a1 = inputState[1];

    // unsigned mesmo
    unsigned short azBits = 0;
    azBits = a1 << 8;
    azBits = azBits | a0;

    float az = azBits * (400.0 / 65535.0);
    az -= 20.0f;

    e0 = inputState[4];
    e1 = inputState[3];

    unsigned short eleBits = 0;
    eleBits = e1 << 8;
    eleBits = eleBits | e0;

    float ele = eleBits * (360.0 / 65535.0);
    ele -= 90.0f;

    // @todo corrigir no Arduino
    //ele = (ele < 350.0f) ? ele : ele - 360.0f;

    lastAzEle.azimuth = az;
    lastAzEle.elevation = ele;

    //Decodificação do STATUS do sistema contido no byte 5 da mensagem
    m =    (inputState[5]) % 2;
    del = ((inputState[5]) >> 1) % 2;
    daz = ((inputState[5]) >> 2) % 2;
    ka2 = ((inputState[5]) >> 3) % 2;
    ka1 = ((inputState[5]) >> 4) % 2;
    p =   ((inputState[5]) >> 7);

    powerStatus = ka1 && ka2;

    return lastAzEle;
}

void Control::changePort(QString port) {
    if (serial.IsOpen()) {
        serial.Close();
    }

    wchar_t* portWchar = new wchar_t[port.length() + 1];
    port.toWCharArray(portWchar);
    portWchar[port.length()] = '\0';
    if (serial.CheckPort(portWchar) == 0) {
        serial.Open(portWchar);
        serial.Setup(CSerial::EBaud9600, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
        serial.SetupReadTimeouts(serial.EReadTimeoutBlocking);
    }

    if (!serial.IsOpen()) {
        qDebug() << "Erro ao abrir porta" << "(" << port << ")";
        validPort = false;
    } else {
        validPort = sendPower();
        qDebug() << "valid: " << validPort;

        if (validPort) {
            controllerTimer.start(100);
            setTarget(180, 90);
        } else {
            controllerTimer.stop();
        }
    }
}

void Control::setDeltas(float deltaAz, float deltaEle) {
    const float minEle = 5.0f;
    const float maxEle = 175.0f;

    if (fabs(deltaAz) > 1e-3 || fabs(deltaEle) > 1e-3) {
        if (lastAzEle.azimuth + deltaAz < 0 || lastAzEle.azimuth + deltaAz >= 360) { deltaAz = 0; }
        if (lastAzEle.elevation + deltaEle < minEle || lastAzEle.elevation + deltaEle >= maxEle) { deltaEle = 0; }

        setTarget(lastAzEle.azimuth + deltaAz, lastAzEle.elevation + deltaEle);
    }
}

void Control::setTarget(float az, float ele) {
    targetAz = az;
    targetEle = ele;
}

void Control::setControlMode(ControlMode controlMode) {
    this->controlMode = controlMode;
}

void Control::moveToTarget() {
    lastAzEle = getState();
    //logger->addLog(lastAzEle);

    double incrementAz = (targetAz + azOffset) - lastAzEle.azimuth;
    double incrementEle = (targetEle + eleOffset) - lastAzEle.elevation;

    if (fabs(incrementAz) > maxSpeed) {
        accelerationAz = Helpers::clip(incrementAz, maxAcceleration);
        speedAz = Helpers::clip(speedAz + accelerationAz, maxSpeed);
    } else {
        speedAz = Helpers::clip(incrementAz, maxSpeed);
    }

    if (fabs(incrementEle) > maxSpeed) {
        accelerationEle = Helpers::clip(incrementEle, maxAcceleration);
        speedEle = Helpers::clip(speedEle + accelerationEle, maxSpeed);
    } else {
        speedEle = Helpers::clip(incrementEle, maxSpeed);
    }

    sendPosition(lastAzEle.azimuth + speedAz,
                 lastAzEle.elevation + speedEle);

    /*
    // restrição de intensidade de movimento
    float incrementAz = targetAz - lastAzEle.azimuth;
    incrementAz = Helpers::clip(incrementAz, maxSpeed);

    float incrementEle = targetEle - lastAzEle.elevation;
    incrementEle = Helpers::clip(incrementEle, maxSpeed);

    sendPosition(lastAzEle.azimuth + incrementAz, lastAzEle.elevation + incrementEle);
    */
}

bool Control::sendPower() {
    int count = 0;

    while (true) {
        serial.Write(power, 1);
        // Timeout de leitura de 50 ms
        char data[50];
        serial.Read(data, 1, 0, 0, 50);

        if (data[0] == 'A') {
            return true;
        } else {
            if (count > 10) { return false; }
            ++count;
        }
    }

    return false;
}

int Control::verifyChecksum() {
    //verifica checksum
    if ((inputState[0] ^ inputState[1] ^ inputState[2] ^ inputState[3] ^ inputState[4] ^ inputState[5]) == inputState[6]) {
        return 0;
    } else {
        return 1;
    }

}

void Control::sendAck(int _erro) {
    if (_erro == 0) {
        serial.Write(ack, 1);
    } else {
        serial.Write(nack, 1);
    }
}

int Control::acknowledge(unsigned char *_input_ack) {
    serial.Read(_input_ack, 1);
    if (_input_ack[0] == 'A') { //reconhecido
        return 0;
    } else {
        return 1;
    }
}

bool Control::isPortValid() {
    return validPort;
}

void Control::updateSlot() {
    if (controlMode == ControlMode::Schedule) {
        if (trackerListModel->getAllPasses().empty()) {
            setTarget(180, 90);
        } else {
            auto nextPass = trackerListModel->getAllPasses().at(0);
            TimeSpan remaining = nextPass.passDetails.aos - DateTime::Now(true);
            float secondsRemaining = remaining.Ticks() / (1e6f); // microseconds to seconds
            float positioningTime = 60;

            if (nextPass.tracker->getElevationForObserver() >= 0.0f) {
                double az = nextPass.tracker->getAzimuthForObserver();
                double ele = nextPass.tracker->getElevationForObserver();
                if (nextPass.passDetails.reverse) {
                    az = fmod(az + 180, 360);
                    ele = 180 - ele;
                }

                setTarget(az, ele);
            } else if (secondsRemaining <= positioningTime &&
                       secondsRemaining > 0) {
                double az = nextPass.tracker->getAzimuthForObserver();
                double ele = 0.0f;
                if (nextPass.passDetails.reverse) {
                    az = fmod(az + 180, 360);
                    ele = 180 - ele;
                }

                setTarget(az, ele);
            } else {
                setTarget(180, 90);
            }
        }
    } else if (controlMode == ControlMode::None) {
        setTarget(180, 90);
    }

    moveToTarget();
}

bool Control::getPowerStatus() {
    return powerStatus;
}
