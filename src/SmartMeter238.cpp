/*
Library for reading DDS238-4 W Wifi Smart meter (SM).
Reading via Hardware Serial
2020 (development with PlatformIO IDE for VSCode & esp8266 core)

MIT License

Copyright (c) 2020 Rodrigo González

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//------------------------------------------------------------------------------
#include "SmartMeter238.h"
//------------------------------------------------------------------------------

#ifdef SM_ENABLE_DEBUG

#ifdef SM_USE_REMOTE_DEBUG
SmartMeter238::SmartMeter238(HardwareSerial &serial, RemoteDebug &debug) : smSerial(serial), smDebug(debug) {}
#else
SmartMeter238::SmartMeter238(HardwareSerial &serial, HardwareSerial &debug) : smSerial(serial), smDebug(debug) {}
#endif   // SM_USE_REMOTE_DEBUG

#else    // SM_ENABLE_DEBUG
SmartMeter238::SmartMeter238(HardwareSerial &serial) : smSerial(serial) {}
#endif   // SM_ENABLE_DEBUG

SmartMeter238::~SmartMeter238() {}

void SmartMeter238::begin(void) {
    this->smSerial.begin(SM_UART_BAUD, SM_UART_CONFIG);
}

bool SmartMeter238::transmitSerialData(uint8_t *array, uint8_t size) {
    SM_PRINT_I(F("* Message send: "));
    SM_PRINT_MESSAGE(array, size);

    while (this->smSerial.available() > 0) {
        this->smSerial.read();

        delay(2);
    }

    this->smSerial.write(array, size);

    this->smSerial.flush();

    SM_PRINT_I_LN(F("* Waiting confirmation:"));

    if (this->receiveSerialData(array, size, array[1], array[4], SM_FRAME_3B_TYPE_SEND)) {
        SM_PRINT_I_LN(F("* Successful Confirmation"));

        return true;
    }

    SM_PRINT_I_LN(F("* Confirmation Failed"));

    return false;
}

bool SmartMeter238::preTransmitSerialData(smCommandTransmit cmd, uint8_t frameSize, uint8_t *array) {
    uint8_t sendArr[frameSize];

    sendArr[0] = SM_FRAME_1B_START;

    sendArr[2] = SM_FRAME_3B_TYPE_SEND;
    sendArr[3] = 0x01;

    switch (cmd) {
        case SM_CMD_GET_POWERCUT: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_GETDATA;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_GETDATA_POWERCUT;

            break;
        }
        case SM_CMD_GET_MEASUREMENTDATA: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_GETDATA;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_GETDATA_MEASUREMENTDATA;

            break;
        }
        case SM_CMD_GET_LIMITANDPURCHASEDATA: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_GETDATA;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_GETDATA_LIMITANDPURCHASEDATA;

            break;
        }
        case SM_CMD_SET_LIMITDATA: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_LIMITDATA;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_LIMITDATA;

            sendArr[5] = array[0];
            sendArr[6] = array[1];

            sendArr[7] = array[2];
            sendArr[8] = array[3];

            sendArr[9] = array[4];
            sendArr[10] = array[5];

            break;
        }
        case SM_CMD_SET_PURCHASEDATA: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_PURCHASEDATA;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_PURCHASEDATA;

            sendArr[5] = array[0];
            sendArr[6] = array[1];
            sendArr[7] = array[2];
            sendArr[8] = array[3];

            sendArr[9] = array[4];
            sendArr[10] = array[5];
            sendArr[11] = array[6];
            sendArr[12] = array[7];

            sendArr[13] = array[8];

            break;
        }
        case SM_CMD_SET_POWERCUT: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_POWERCUT;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_POWERCUT;

            sendArr[5] = array[0];

            break;
        }
        case SM_CMD_SET_DELAY: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_DELAY;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_DELAY;

            sendArr[5] = array[0];
            sendArr[6] = array[1];

            sendArr[7] = array[2];

            break;
        }
        case SM_CMD_SET_RESET: {
            sendArr[1] = SM_FRAME_2B_COMD_SEND_RESET;
            sendArr[4] = SM_FRAME_5B_SUBCOMD_SEND_RESET;

            sendArr[5] = array[0];
            sendArr[6] = array[1];
            sendArr[7] = array[2];
            sendArr[8] = array[3];
            sendArr[9] = array[4];
            sendArr[10] = array[5];
            sendArr[11] = array[6];
            sendArr[12] = array[7];
            sendArr[13] = array[8];
            sendArr[14] = array[9];
            sendArr[15] = array[10];
            sendArr[16] = array[11];

            break;
        }
    }

    sendArr[frameSize - 1] = this->calculateCRC(sendArr, frameSize);

    return this->transmitSerialData(sendArr, frameSize);
}

bool SmartMeter238::receiveSerialData(uint8_t *array, uint8_t size, uint8_t command, uint8_t subCommand, uint8_t typeMessage) {
    unsigned long resptime;
    smErrorCode readErr = SM_ERR_NO_ERROR;

    resptime = millis() + SM_MAX_MILLIS_TO_RESPONSE;

    while (this->smSerial.available() < size) {
        if (resptime < millis()) {
            if (this->smSerial.available() > 0) {
                readErr = SM_ERR_NOT_ENOUGHT_BYTES;
            } else {
                readErr = SM_ERR_TIMEOUT;
            }

            break;
        }

        yield();
    }

    delay(2);

    if (readErr == SM_ERR_NO_ERROR) {
        if (this->smSerial.available() == size) {
            for (int n = 0; n < size; n++) {
                array[n] = this->smSerial.read();
            }

            SM_PRINT_I(F("* Message received: "));
            SM_PRINT_MESSAGE(array, size);

            if (array[0] == SM_FRAME_1B_START && array[1] == command && array[2] == typeMessage && array[4] == subCommand) {
                if (this->calculateCRC(array, size) != array[size - 1]) {
                    readErr = SM_ERR_CRC_ERROR;
                }
            } else {
                readErr = SM_ERR_WRONG_BYTES;
            }
        } else {
            readErr = SM_ERR_EXCEEDS_BYTES;
        }
    }

    if (readErr != SM_ERR_NO_ERROR) {
        this->errType = SM_TYPE_COMMUNICATION_ERROR;
        this->errCode = readErr;

        this->readingErrCount++;
    } else {
        this->readingSuccessCount++;
    }

    delay(2);

    while (this->smSerial.available() > 0) {
        this->smSerial.read();

        delay(2);
    }

    return (this->errCode == SM_ERR_NO_ERROR);
}

bool SmartMeter238::preReceiveSerialData(smCommandReceive cmd, uint8_t frameSize, smartMeterData *dataObject) {
    uint8_t receiveArr[frameSize];

    SM_PRINT_I_LN(F("* Waiting answer:"));

    switch (cmd) {
        case SM_CMD_RESP_POWERCUT: {
            if (this->receiveSerialData(receiveArr, frameSize, SM_FRAME_2B_COMD_RESPONSE_POWERCUT, SM_FRAME_5B_SUBCOMD_RESPONSE_POWERCUT, SM_FRAME_3B_TYPE_RESPONSE)) {
                dataObject->powerCutData.time = millis();

                dataObject->powerCutData.data.powerCut = !receiveArr[6];

                if (dataObject->powerCutData.data.powerCut) {
                    if (receiveArr[11] == 1) {
                        dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_OVER_VOLTAGE;
                    } else if (receiveArr[11] == 2) {
                        dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_UNDER_VOLTAGE;
                    } else if (receiveArr[15] == 1) {
                        dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_OVER_CURRENT;
                    } else if (receiveArr[19] == 1) {
                        dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_END_PURCHASE;
                    } else {
                        dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_UNKNOWN;
                    }
                } else {
                    dataObject->powerCutData.data.powerCutDetails = SM_STR_POWERCUT_DETAILS_NO_POWER_CUT;
                }

                dataObject->powerCutData.data.delay = (receiveArr[16] << 8) | receiveArr[17];
                dataObject->powerCutData.data.delaySetPowerCut = receiveArr[18];

                SM_PRINT_I_LN(F("* Successful answer"));

                return true;
            }

            break;
        }
        case SM_CMD_RESP_MEASUREMENTDATA: {
            if (this->receiveSerialData(receiveArr, frameSize, SM_FRAME_2B_COMD_RESPONSE_MEASUREMENTDATA, SM_FRAME_5B_SUBCOMD_RESPONSE_MEASUREMENTDATA, SM_FRAME_3B_TYPE_RESPONSE)) {
                dataObject->measurementData.time = millis();

                dataObject->measurementData.data.current = ((receiveArr[5] << 16) | (receiveArr[6] << 8) | receiveArr[7]) * 0.001;
                dataObject->measurementData.data.voltage = ((receiveArr[14] << 8) | receiveArr[15]) * 0.1;
                dataObject->measurementData.data.frequency = ((receiveArr[52] << 8) | receiveArr[53]) * 0.01;

                dataObject->measurementData.data.reactivePower = receiveArr[20] + (((receiveArr[21] << 8) | receiveArr[22]) * 0.0001);
                dataObject->measurementData.data.activePower = receiveArr[32] + (((receiveArr[33] << 8) | receiveArr[34]) * 0.0001);
                dataObject->measurementData.data.powerFactor = ((receiveArr[44] << 8) | receiveArr[45]) * 0.001;

                dataObject->measurementData.data.lapseOfTimeTotalEnergy = (((receiveArr[54] << 24) | (receiveArr[55] << 16) | (receiveArr[56] << 8) | receiveArr[57]) * 0.01);
                dataObject->measurementData.data.lapseOfTimeImportEnergy = (((receiveArr[58] << 24) | (receiveArr[59] << 16) | (receiveArr[60] << 8) | receiveArr[61]) * 0.01);
                dataObject->measurementData.data.lapseOfTimeExportEnergy = (((receiveArr[62] << 24) | (receiveArr[63] << 16) | (receiveArr[64] << 8) | receiveArr[65]) * 0.01);
                dataObject->measurementData.data.lapseOfTimePriceEnergy = dataObject->measurementData.data.lapseOfTimeTotalEnergy * dataObject->powerCompanyData.data.priceKWh;

                dataObject->measurementData.data.totalKWh = dataObject->measurementData.data.lapseOfTimeTotalEnergy + dataObject->powerCompanyData.data.startingKWh;

                SM_PRINT_I_LN(F("* Successful answer"));

                return true;
            }

            break;
        }
        case SM_CMD_RESP_LIMITANDPURCHASEDATA: {
            if (this->receiveSerialData(receiveArr, frameSize, SM_FRAME_2B_COMD_RESPONSE_LIMITANDPURCHASEDATA, SM_FRAME_5B_SUBCOMD_RESPONSE_LIMITANDPURCHASEDATA, SM_FRAME_3B_TYPE_RESPONSE)) {
                dataObject->limitAndPurchaseData.time = millis();

                dataObject->limitAndPurchaseData.data.energyPurchase = (((receiveArr[11] << 24) | (receiveArr[12] << 16) | (receiveArr[13] << 8) | receiveArr[14]) * 0.01);
                dataObject->limitAndPurchaseData.data.energyPurchaseBalance = (((receiveArr[15] << 24) | (receiveArr[16] << 16) | (receiveArr[17] << 8) | receiveArr[18]) * 0.01);
                dataObject->limitAndPurchaseData.data.energyPurchaseAlarm = (((receiveArr[19] << 24) | (receiveArr[20] << 16) | (receiveArr[21] << 8) | receiveArr[22]) * 0.01);
                dataObject->limitAndPurchaseData.data.energyPurchaseStatus = receiveArr[13];

                dataObject->limitAndPurchaseData.data.maxCurrentLimit = ((receiveArr[9] << 8) | receiveArr[10]) * 0.01;
                dataObject->limitAndPurchaseData.data.maxVoltageLimit = (receiveArr[5] << 8) | receiveArr[6];
                dataObject->limitAndPurchaseData.data.minVoltageLimit = (receiveArr[7] << 8) | receiveArr[8];

                SM_PRINT_I_LN(F("* Successful answer"));

                return true;
            }

            break;
        }
    }

    SM_PRINT_I_LN(F("* Failed answer"));

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

bool SmartMeter238::getPowerCutData(smartMeterData *dataObject, bool forceUpdate) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (getPowerCutData)"));

    SM_PRINT_V_LN(F("* No input Data"));

    if (!forceUpdate) {
        if (!((millis() - dataObject->powerCutData.time) >= this->minIntervalUpdate) && dataObject->powerCutData.time > 0) {
            SM_PRINT_I_LN(F("* Not necessary to update the data:"));

            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getPowerCutData)"));

            return true;
        }
    }

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (this->preTransmitSerialData(SM_CMD_GET_POWERCUT, SM_FRAMESIZE_MSG_GET_POWERCUT, nullptr)) {
        if (this->preReceiveSerialData(SM_CMD_RESP_POWERCUT, SM_FRAMESIZE_MSG_RESP_POWERCUT, dataObject)) {
            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getPowerCutData)"));

            return true;
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getPowerCutData)"));

    return false;
}

bool SmartMeter238::getMeasurementData(smartMeterData *dataObject, bool forceUpdate) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (getMeasurementData)"));

    SM_PRINT_V_LN(F("* No input Data"));

    if (!forceUpdate) {
        if (!((millis() - dataObject->measurementData.time) >= this->minIntervalUpdate) && dataObject->measurementData.time > 0) {
            SM_PRINT_I_LN(F("* Not necessary to update the data:"));

            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getMeasurementData)"));

            return true;
        }
    }

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (this->preTransmitSerialData(SM_CMD_GET_MEASUREMENTDATA, SM_FRAMESIZE_MSG_GET_MEASUREMENTDATA, nullptr)) {
        if (this->preReceiveSerialData(SM_CMD_RESP_MEASUREMENTDATA, SM_FRAMESIZE_MSG_RESP_MEASUREMENTDATA, dataObject)) {
            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getMeasurementData)"));

            return true;
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getMeasurementData)"));

    return false;
}

bool SmartMeter238::getLimitAndPurchaseData(smartMeterData *dataObject, bool forceUpdate) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (getLimitAndPurchaseData)"));

    SM_PRINT_V_LN(F("* No input Data"));

    if (!forceUpdate) {
        if (!((millis() - dataObject->limitAndPurchaseData.time) >= this->minIntervalUpdate) && dataObject->limitAndPurchaseData.time > 0) {
            SM_PRINT_I_LN(F("* Not necessary to update the data:"));

            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getLimitAndPurchaseData)"));

            return true;
        }
    }

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (this->preTransmitSerialData(SM_CMD_GET_LIMITANDPURCHASEDATA, SM_FRAMESIZE_MSG_GET_LIMITANDPURCHASEDATA, nullptr)) {
        if (this->preReceiveSerialData(SM_CMD_RESP_LIMITANDPURCHASEDATA, SM_FRAMESIZE_MSG_RESP_LIMITANDPURCHASEDATA, dataObject)) {
            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getLimitAndPurchaseData)"));

            return true;
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getLimitAndPurchaseData)"));

    return false;
}

bool SmartMeter238::getPowerCompanyData(smartMeterData *dataObject, bool forceUpdate) {
    //Dummy function for now

    SM_PRINT_I_LN(F("In to SmartMeter238 Library (getPowerCompanyData)"));

    SM_PRINT_V_LN(F("* No input Data"));

    if (!forceUpdate) {
        if (!((millis() - dataObject->powerCompanyData.time) >= this->minIntervalUpdate) && dataObject->powerCompanyData.time > 0) {
            SM_PRINT_I_LN(F("* Not necessary to update the data:"));

            SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getPowerCompanyData)"));

            return true;
        }
    }

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    dataObject->powerCompanyData.time = millis();

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (getPowerCompanyData)"));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

bool SmartMeter238::setLimitsData(float maxCurrentLimit, uint16_t maxVoltageLimit, uint16_t minVoltageLimit, smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setLimitsData)"));

    SM_PRINT_V(F("* Input Data:"));
    SM_PRINT_V(F(" maxCurrentLimit = "));
    SM_PRINT_V(maxCurrentLimit);
    SM_PRINT_V(F(" - maxVoltageLimit = "));
    SM_PRINT_V(maxVoltageLimit);
    SM_PRINT_V(F(" - minVoltageLimit = "));
    SM_PRINT_V_LN(minVoltageLimit);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (maxCurrentLimit < SM_MIN_CURRENT_LIMIT || maxCurrentLimit > SM_MAX_CURRENT_LIMIT) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_1P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (maxVoltageLimit > SM_MAX_VOLTAGE_LIMIT) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_2P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (minVoltageLimit < SM_MIN_VOLTAGE_LIMIT) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_3P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (minVoltageLimit > minVoltageLimit) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_3P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (this->errCode == SM_ERR_NO_ERROR) {
        uint8_t sendArr[6];

        uint16_t tmpCurrentLimit = maxCurrentLimit * 100;

        sendArr[0] = (tmpCurrentLimit >> 8);
        sendArr[1] = (tmpCurrentLimit & SM_GET_ONE_BYTE);

        sendArr[2] = (maxVoltageLimit >> 8);
        sendArr[3] = (maxVoltageLimit & SM_GET_ONE_BYTE);

        sendArr[4] = (minVoltageLimit >> 8);
        sendArr[5] = (minVoltageLimit & SM_GET_ONE_BYTE);

        if (this->preTransmitSerialData(SM_CMD_SET_LIMITDATA, SM_FRAMESIZE_MSG_SET_LIMITDATA, sendArr)) {
            if (this->preReceiveSerialData(SM_CMD_RESP_LIMITANDPURCHASEDATA, SM_FRAMESIZE_MSG_RESP_LIMITANDPURCHASEDATA, dataObject)) {
                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setLimitsData)"));

                return true;
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setLimitsData)"));

    return false;
}

bool SmartMeter238::setPurchaseData(float energyPurchase, float energyPurchaseAlarm, bool energyPurchaseStatus, smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setPurchaseData)"));

    SM_PRINT_V(F("* Input Data: "));
    SM_PRINT_V(F("energyPurchase = "));
    SM_PRINT_V(energyPurchase);
    SM_PRINT_V(F(" - energyPurchaseAlarm = "));
    SM_PRINT_V(energyPurchaseAlarm);
    SM_PRINT_V(F(" - energyPurchaseStatus = "));
    SM_PRINT_V_LN(energyPurchaseStatus);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (energyPurchase < SM_MIN_ENERGY_PURCHASE || energyPurchase > SM_MAX_ENERGY_PURCHASE) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_1P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (energyPurchaseAlarm < SM_MIN_ENERGY_ALARM || energyPurchaseAlarm > SM_MAX_ENERGY_ALARM) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_2P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (this->errCode == SM_ERR_NO_ERROR) {
        uint8_t sendArr[9];

        uint32_t tmpEnergyPurchase = floor(energyPurchase * 100);
        uint32_t tmpEnergyPurchaseAlarm = floor(energyPurchaseAlarm * 100);

        sendArr[0] = (tmpEnergyPurchase >> 24);
        sendArr[1] = (tmpEnergyPurchase >> 16);
        sendArr[2] = (tmpEnergyPurchase >> 8);
        sendArr[3] = (tmpEnergyPurchase & SM_GET_ONE_BYTE);

        sendArr[4] = (tmpEnergyPurchaseAlarm >> 24);
        sendArr[5] = (tmpEnergyPurchaseAlarm >> 16);
        sendArr[6] = (tmpEnergyPurchaseAlarm >> 8);
        sendArr[7] = (tmpEnergyPurchaseAlarm & SM_GET_ONE_BYTE);

        sendArr[8] = energyPurchaseStatus;

        if (this->preTransmitSerialData(SM_CMD_SET_PURCHASEDATA, SM_FRAMESIZE_MSG_SET_PURCHASEDATA, sendArr)) {
            if (this->preReceiveSerialData(SM_CMD_RESP_LIMITANDPURCHASEDATA, SM_FRAMESIZE_MSG_RESP_LIMITANDPURCHASEDATA, dataObject)) {
                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPurchaseData)"));

                return true;
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPurchaseData)"));

    return false;
}

bool SmartMeter238::setPowerCutData(bool powerCut, smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setPowerCutData)"));

    SM_PRINT_V(F("* Input Data: powerCut = "));
    SM_PRINT_V_LN(powerCut);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    // VALIDATE DATA (NOT)

    if (this->errCode == SM_ERR_NO_ERROR) {
        uint8_t sendArr[1];

        sendArr[0] = !powerCut;

        if (this->preTransmitSerialData(SM_CMD_SET_POWERCUT, SM_FRAMESIZE_MSG_SET_POWERCUT, sendArr)) {
            if (this->preReceiveSerialData(SM_CMD_RESP_POWERCUT, SM_FRAMESIZE_MSG_RESP_POWERCUT, dataObject)) {
                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPowerCutData)"));

                return true;
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPowerCutData)"));

    return false;
}

bool SmartMeter238::setDelay(bool delaySetPowerCut, uint16_t delay, smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setDelay)"));

    SM_PRINT_V(F("* Input Data: "));
    SM_PRINT_V(F("delaySetPowerCut = "));
    SM_PRINT_V(delaySetPowerCut);
    SM_PRINT_V(F(" - delay = "));
    SM_PRINT_V_LN(delay);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (delay < SM_MIN_DELAY || delay > SM_MAX_DELAY) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_2P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (this->errCode == SM_ERR_NO_ERROR) {
        uint8_t sendArr[3];

        sendArr[0] = (delay >> 8);
        sendArr[1] = (delay & SM_GET_ONE_BYTE);

        sendArr[2] = delaySetPowerCut;

        if (this->preTransmitSerialData(SM_CMD_SET_DELAY, SM_FRAMESIZE_MSG_SET_DELAY, sendArr)) {
            if (this->preReceiveSerialData(SM_CMD_RESP_POWERCUT, SM_FRAMESIZE_MSG_RESP_POWERCUT, dataObject)) {
                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setDelay)"));

                return true;
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setDelay)"));

    return false;
}

bool SmartMeter238::setReset(smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setReset)"));

    SM_PRINT_V_LN(F("* No input Data"));

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    // VALIDATE DATA (NOT)

    if (this->errCode == SM_ERR_NO_ERROR) {
        uint8_t sendArr[12];

        sendArr[0] = 0x00;
        sendArr[1] = 0x00;
        sendArr[2] = 0x00;
        sendArr[3] = 0x00;
        sendArr[4] = 0x00;
        sendArr[5] = 0x00;
        sendArr[6] = 0x00;
        sendArr[7] = 0x00;
        sendArr[8] = 0x00;
        sendArr[9] = 0x00;
        sendArr[10] = 0x00;
        sendArr[11] = 0x00;

        float tmpStartingKWh = dataObject->powerCompanyData.data.startingKWh + dataObject->measurementData.data.lapseOfTimeTotalEnergy;

        if (this->preTransmitSerialData(SM_CMD_SET_RESET, SM_FRAMESIZE_MSG_SET_RESET, sendArr)) {
            if (this->preReceiveSerialData(SM_CMD_RESP_MEASUREMENTDATA, SM_FRAMESIZE_MSG_RESP_MEASUREMENTDATA, dataObject)) {
                dataObject->powerCompanyData.data.startingKWh = tmpStartingKWh;
                dataObject->measurementData.data.totalKWh = tmpStartingKWh;

                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setReset)"));

                return true;
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setReset)"));

    return false;
}

bool SmartMeter238::setPowerCompanyData(float startingKWh, float priceKWh, smartMeterData *dataObject) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (setPowerCompanyData)"));

    SM_PRINT_V(F("* Input Data: "));
    SM_PRINT_V(F("startingKWh = "));
    SM_PRINT_V(startingKWh);
    SM_PRINT_V(F(" - priceKWh = "));
    SM_PRINT_V_LN(priceKWh);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    if (startingKWh < SM_MIN_ENERGY_STARTING || startingKWh > SM_MAX_ENERGY_STARTING) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_1P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (priceKWh < SM_MIN_ENERGY_PRICE || priceKWh > SM_MAX_ENERGY_PRICE) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_2P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (this->errCode == SM_ERR_NO_ERROR) {
        dataObject->powerCompanyData.data.startingKWh = startingKWh;
        dataObject->powerCompanyData.data.priceKWh = priceKWh;

        // Update counters
        dataObject->measurementData.data.lapseOfTimePriceEnergy = dataObject->measurementData.data.lapseOfTimeTotalEnergy * priceKWh;
        dataObject->measurementData.data.totalKWh = dataObject->measurementData.data.lapseOfTimeTotalEnergy + startingKWh;

        SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPowerCompanyData)"));

        return true;
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (setPowerCompanyData)"));

    return false;
}

#ifdef SM_ENABLE_RAW_TEST_MSG
bool SmartMeter238::sendHexMessage(const char *msg) {
    SM_PRINT_I_LN(F("In to SmartMeter238 Library (sendHexMessage)"));

    SM_PRINT_V(F("* Input Data: "));
    SM_PRINT_V(F("hex = "));
    SM_PRINT_V_LN(msg);

    this->errType = SM_TYPE_NO_ERROR;
    this->errCode = SM_ERR_NO_ERROR;

    uint8_t lengthMsg = strlen(msg);

    if (lengthMsg == 0 || lengthMsg > SM_MAX_HEX_MSG_LENGTH) {
        this->errType = SM_TYPE_INPUT_DATA_ERROR;
        this->errCode = SM_ERR_1P_INPUT_DATA_OUT_OF_RANGE;
    }

    if (this->errCode == SM_ERR_NO_ERROR) {
        char tmpMsg[SM_MAX_HEX_MSG_LENGTH_PARSE];   // Max char without ":"

        uint8_t size = 0;

        uint8_t c = 0;

        for (uint8_t i = 0; i < lengthMsg; i++) {
            if (msg[i] == ':') {
                if (c == 2) {
                    c = 0;
                } else {
                    this->errType = SM_TYPE_INPUT_DATA_ERROR;
                    this->errCode = SM_ERR_WRONG_MSG;

                    break;
                }
            } else {
                if (c == 2) {
                    this->errType = SM_TYPE_INPUT_DATA_ERROR;
                    this->errCode = SM_ERR_WRONG_MSG;

                    break;
                } else {
                    tmpMsg[size] = msg[i];
                    size++;

                    c++;
                }
            }
        }

        if (this->errCode == SM_ERR_NO_ERROR) {
            if ((size % 2) != 0) {
                this->errType = SM_TYPE_INPUT_DATA_ERROR;
                this->errCode = SM_ERR_WRONG_MSG;
            }

            if (this->errCode == SM_ERR_NO_ERROR) {
                uint8_t lengthArray = size / 2;

                uint8_t sendArr[lengthArray];

                for (uint8_t i = 0; i < lengthArray; i++) {
                    int a = this->char2int(tmpMsg[2 * i]);
                    int b = this->char2int(tmpMsg[(2 * i) + 1]);

                    if (a == -1 || b == -1) {
                        this->errType = SM_TYPE_INPUT_DATA_ERROR;
                        this->errCode = SM_ERR_WRONG_MSG;

                        break;
                    }

                    sendArr[i] = (a << 4) | b;
                }

                if (this->errCode == SM_ERR_NO_ERROR) {
                    sendArr[lengthArray - 1] = this->calculateCRC(sendArr, lengthArray);

                    if (this->transmitSerialData(sendArr, lengthArray)) {
                        SM_PRINT_I_LN(F("* Waiting answer:"));

                        unsigned long resptime = millis() + SM_MAX_MILLIS_TO_RESPONSE;

                        while (this->smSerial.available() == 0) {
                            if (resptime < millis()) {
                                this->errCode = SM_ERR_TIMEOUT;

                                break;
                            }

                            yield();
                        }

                        if (this->errCode == SM_ERR_NO_ERROR) {
                            if (this->processIncomingMessages()) {
                                SM_PRINT_I_LN(F("* Successful answer"));

                                SM_PRINT_I_LN(F("Out from SmartMeter238 Library (sendHexMessage)"));

                                return true;
                            }
                        }

                        SM_PRINT_I_LN(F("* Failed answer"));
                    }
                }
            }
        }
    }

    SM_PRINT_ERROR(true);

    SM_PRINT_I_LN(F("Out from SmartMeter238 Library (sendHexMessage)"));

    return false;
}

bool SmartMeter238::processIncomingMessages() {
    uint8_t index = 0;

    strlcpy(this->incomingHexMessage, "", SM_MAX_HEX_MSG_LENGTH);   // clean hex message buffer

    while (this->smSerial.available() > 0) {
        this->incomingByteMessage[index] = this->smSerial.read();

        delay(2);   // ESP is more quickly

        index++;

        if (index == SM_MAX_BYTE_MSG_BUFFER) {
            break;
        }
    }

    if (index == 0) {
        return true;   // 0 is not error
    } else {
        SM_PRINT_W(F("Incoming message arrived: "));

        SM_PRINT_MESSAGE(this->incomingByteMessage, index);

        if (this->incomingByteMessage[0] == SM_FRAME_1B_START) {
            if (this->calculateCRC(this->incomingByteMessage, index) != this->incomingByteMessage[index - 1]) {
                SM_PRINT_W_LN(F("* Error CRC"));

                this->errCode = SM_ERR_CRC_ERROR;
            } else {
                SM_PRINT_W_LN(F("* Valid message"));

                char hexstr[index * 3];

                uint8_t i;

                for (i = 0; i < index; i++) {
                    sprintf(hexstr + (i * 3), "%02x:", this->incomingByteMessage[i]);
                }

                hexstr[(index * 3) - 1] = 0;

                for (i = 0; i < strlen(hexstr); i++) {
                    hexstr[i] = toupper(hexstr[i]);
                }

                strlcpy(this->incomingHexMessage, hexstr, SM_MAX_HEX_MSG_LENGTH);

                return true;
            }
        } else {
            SM_PRINT_W_LN(F("* Wrong Bytes"));

            this->errCode = SM_ERR_WRONG_BYTES;
        }
    }

    return false;
}

char *SmartMeter238::getIncomingHexMessage() {
    return this->incomingHexMessage;
}

int SmartMeter238::char2int(char input) {
    if (input >= '0' && input <= '9') {
        return input - 48;
    } else if (input >= 'A' && input <= 'F') {
        return input - 55;
    } else if (input >= 'a' && input <= 'f') {
        return input - 87;
    }

    return -1;
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

SmartMeter238::smErrorType SmartMeter238::getErrType(bool clear) {
    smErrorType tmp = this->errType;

    if (clear) {
        this->clearErrType();
    }

    return tmp;
}

SmartMeter238::smErrorCode SmartMeter238::getErrCode(bool clear) {
    smErrorCode tmp = this->errCode;

    if (clear) {
        this->clearErrCode();
    }

    return tmp;
}

uint16_t SmartMeter238::getErrCount(bool clear) {
    uint16_t tmp = this->readingErrCount;

    if (clear) {
        this->clearErrCount();
    }

    return tmp;
}

uint16_t SmartMeter238::getSuccCount(bool clear) {
    uint16_t tmp = this->readingSuccessCount;

    if (clear) {
        this->clearSuccCount();
    }

    return tmp;
}

void SmartMeter238::clearErrType() {
    this->errType = SM_TYPE_NO_ERROR;
}

void SmartMeter238::clearErrCode() {
    this->errCode = SM_ERR_NO_ERROR;
}

void SmartMeter238::clearErrCount() {
    this->readingErrCount = 0;
}

void SmartMeter238::clearSuccCount() {
    this->readingSuccessCount = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

uint8_t SmartMeter238::calculateCRC(uint8_t *array, uint8_t size) {
    uint16_t tmpCRC = 0;
    uint8_t crc;

    uint8_t tmpSize = size - 1;   // we discard the last byte that corresponds to the CRC from the sum

    for (uint8_t n = 0; n < tmpSize; n++) {
        tmpCRC = tmpCRC + array[n];
    }

    crc = tmpCRC & SM_GET_ONE_BYTE;

    return crc;
}

char *SmartMeter238::getTypeStr(bool clear) {
    const char *textTable = smStrTypeTable[this->getErrType(clear)];

    uint8_t sizeString = strlen_P(textTable);

    for (uint8_t i = 0; i < sizeString; i++) {
        prtStrType[i] = pgm_read_byte_near(textTable + i);
    }

    prtStrType[sizeString] = 0;   // '\0'

    return prtStrType;
}

char *SmartMeter238::getErrorStr(bool clear) {
    const char *textTable = smStrErrTable[this->getErrCode(clear)];

    uint8_t sizeString = strlen_P(textTable);

    for (uint8_t i = 0; i < sizeString; i++) {
        prtStrError[i] = pgm_read_byte_near(textTable + i);
    }

    prtStrError[sizeString] = 0;   // '\0'

    return prtStrError;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

#ifdef SM_ENABLE_DEBUG
void SmartMeter238::printMessage(uint8_t *array, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        this->printByte(array[i], false);

        if (i < (size - 1)) {
            SM_PRINT_I(":");
        } else {
            SM_PRINT_I_LN();
        }
    }
}

void SmartMeter238::printByte(uint8_t byte, bool prefix) {
    if (prefix) {
        SM_PRINT_I(F("0x"));
    }

    if ((byte & 0xF0) == 0) {
        SM_PRINT_I(F("0"));
    }

    SM_PRINT_I(byte, HEX);
}

void SmartMeter238::printError(bool clear) {
    SM_PRINT_I(F("* Error Reading Count: "));
    SM_PRINT_I_LN(this->getErrCount(clear));

    SM_PRINT_I(F("* Success Reading Count: "));
    SM_PRINT_I_LN(this->getSuccCount(clear));

    SM_PRINT_I(F("* Error Type: "));
    SM_PRINT_I_LN(this->getTypeStr(false));

    SM_PRINT_I(F("* Error Description: "));
    SM_PRINT_I_LN(this->getErrorStr(false));
}
#endif
