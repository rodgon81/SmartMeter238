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
#ifndef SmartMeter238_h
#define SmartMeter238_h
//------------------------------------------------------------------------------

#define SM_VERSION "1.0.0-beta1"

#include <Arduino.h>
#include <HardwareSerial.h>

#ifdef SM_ENABLE_DEBUG

#define SM_PRINT_ERROR(x) this->printError(x);
#define SM_PRINT_MESSAGE(x, y) this->printMessage(x, y);

#ifdef SM_USE_REMOTE_DEBUG

//#include <RemoteDebug.h>   // https://github.com/JoaoLopesF/RemoteDebug

#define SM_PRINT_V(x, ...) \
    if (this->smDebug.isActive(this->smDebug.VERBOSE)) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_D(x, ...) \
    if (this->smDebug.isActive(this->smDebug.DEBUG)) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_I(x, ...) \
    if (this->smDebug.isActive(this->smDebug.INFO)) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_W(x, ...) \
    if (this->smDebug.isActive(this->smDebug.WARNING)) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_E(x, ...) \
    if (this->smDebug.isActive(this->smDebug.ERROR)) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_A(x, ...) \
    if (this->smDebug.isActive(this->smDebug.ANY)) this->smDebug.print(x, ##__VA_ARGS__)

#define SM_PRINT_V_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.VERBOSE)) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_D_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.DEBUG)) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_I_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.INFO)) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_W_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.WARNING)) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_E_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.ERROR)) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_A_LN(x, ...) \
    if (this->smDebug.isActive(this->smDebug.ANY)) this->smDebug.println(x, ##__VA_ARGS__)

#else   // SM_USE_REMOTE_DEBUG

#define SM_PRINT_V(x, ...) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_D(x, ...) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_I(x, ...) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_W(x, ...) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_E(x, ...) this->smDebug.print(x, ##__VA_ARGS__)
#define SM_PRINT_A(x, ...) this->smDebug.print(x, ##__VA_ARGS__)

#define SM_PRINT_V_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_D_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_I_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_W_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_E_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)
#define SM_PRINT_A_LN(x, ...) this->smDebug.println(x, ##__VA_ARGS__)

#endif   // SM_USE_REMOTE_DEBUG

#else   // SM_ENABLE_DEBUG

#define SM_PRINT_ERROR(x)
#define SM_PRINT_MESSAGE(x, y)

#define SM_PRINT_V(x, ...)
#define SM_PRINT_D(x, ...)
#define SM_PRINT_I(x, ...)
#define SM_PRINT_W(x, ...)
#define SM_PRINT_E(x, ...)
#define SM_PRINT_A(x, ...)

#define SM_PRINT_V_LN(x, ...)
#define SM_PRINT_D_LN(x, ...)
#define SM_PRINT_I_LN(x, ...)
#define SM_PRINT_W_LN(x, ...)
#define SM_PRINT_E_LN(x, ...)
#define SM_PRINT_A_LN(x, ...)

#endif   // SM_ENABLE_DEBUG

//------------------------------------------------------------------------------
// DEFAULTS
//------------------------------------------------------------------------------

#define SM_MIN_INTERVAL_TO_GET_DATA 500   // millis

#ifndef SM_MAX_MILLIS_TO_CONFIRM
#define SM_MAX_MILLIS_TO_CONFIRM 200   // default max time to wait for confirm from DDS2384W
#endif

#ifndef SM_MAX_MILLIS_TO_RESPONSE
#define SM_MAX_MILLIS_TO_RESPONSE 1000   // default max time to wait for response from DDS2384W
#endif

// Min Max data
#define SM_MIN_VOLTAGE_LIMIT 80         // V
#define SM_MAX_VOLTAGE_LIMIT 300        // V
#define SM_MIN_CURRENT_LIMIT 0          // A
#define SM_MAX_CURRENT_LIMIT 60         // A
#define SM_MIN_DELAY 0                  // minutes
#define SM_MAX_DELAY 1440               // minutes
#define SM_MIN_ENERGY_ALARM 0           // kWh
#define SM_MAX_ENERGY_ALARM 100000      // kWh
#define SM_MIN_ENERGY_PURCHASE 0        // kWh
#define SM_MAX_ENERGY_PURCHASE 100000   // kWh
#define SM_MIN_ENERGY_STARTING 0        // kWh
#define SM_MAX_ENERGY_STARTING 100000   // kWh
#define SM_MIN_ENERGY_PRICE 0           //$
#define SM_MAX_ENERGY_PRICE 1000        //$

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

#define SM_UART_BAUD 9600   // default baudrate

#define SM_UART_CONFIG SERIAL_8N1   // default hardware uart config

#define SM_SET_ON true
#define SM_SET_OFF false

#define SM_MAX_BYTE_MSG_BUFFER 96
#define SM_MAX_HEX_MSG_LENGTH 256
#define SM_MAX_HEX_MSG_LENGTH_PARSE 176

#define SM_MAX_STR_LENGTH_TYPE 16
#define SM_MAX_STR_LENGTH_ERROR 48

//------------------------------------------------------------------------------

#define SM_FRAMESIZE_MSG_RESP_POWERCUT 0x15
#define SM_FRAMESIZE_MSG_RESP_MEASUREMENTDATA 0x43
#define SM_FRAMESIZE_MSG_RESP_LIMITANDPURCHASEDATA 0x19

#define SM_FRAMESIZE_MSG_GET_POWERCUT 0x06
#define SM_FRAMESIZE_MSG_GET_MEASUREMENTDATA 0x06
#define SM_FRAMESIZE_MSG_GET_LIMITANDPURCHASEDATA 0x06

#define SM_FRAMESIZE_MSG_SET_LIMITDATA 0x0C
#define SM_FRAMESIZE_MSG_SET_PURCHASEDATA 0x0F
#define SM_FRAMESIZE_MSG_SET_POWERCUT 0x07
#define SM_FRAMESIZE_MSG_SET_DELAY 0x09
#define SM_FRAMESIZE_MSG_SET_RESET 0x12

//------------------------------------------------------------------------------

// Start Frame
#define SM_FRAME_1B_START 0x48

// Command Response
#define SM_FRAME_2B_COMD_RESPONSE_POWERCUT 0x15
#define SM_FRAME_2B_COMD_RESPONSE_MEASUREMENTDATA 0x43
#define SM_FRAME_2B_COMD_RESPONSE_LIMITANDPURCHASEDATA 0x19

// Command Send
#define SM_FRAME_2B_COMD_SEND_LIMITDATA 0x0C
#define SM_FRAME_2B_COMD_SEND_PURCHASEDATA 0x0F
#define SM_FRAME_2B_COMD_SEND_POWERCUT 0x07
#define SM_FRAME_2B_COMD_SEND_DELAY 0x09
#define SM_FRAME_2B_COMD_SEND_RESET 0x12
#define SM_FRAME_2B_COMD_SEND_GETDATA 0x06

// Type message
#define SM_FRAME_3B_TYPE_RESPONSE 0x01
#define SM_FRAME_3B_TYPE_SEND 0x02

// Subcommand Response
#define SM_FRAME_5B_SUBCOMD_RESPONSE_POWERCUT 0x01
#define SM_FRAME_5B_SUBCOMD_RESPONSE_MEASUREMENTDATA 0x0B
#define SM_FRAME_5B_SUBCOMD_RESPONSE_LIMITANDPURCHASEDATA 0x08

// Subcommand Send
#define SM_FRAME_5B_SUBCOMD_SEND_LIMITDATA 0x03
#define SM_FRAME_5B_SUBCOMD_SEND_PURCHASEDATA 0x0D
#define SM_FRAME_5B_SUBCOMD_SEND_POWERCUT 0x09
#define SM_FRAME_5B_SUBCOMD_SEND_DELAY 0x0C
#define SM_FRAME_5B_SUBCOMD_SEND_RESET 0x05
#define SM_FRAME_5B_SUBCOMD_SEND_GETDATA_POWERCUT 0x00
#define SM_FRAME_5B_SUBCOMD_SEND_GETDATA_MEASUREMENTDATA 0x0A
#define SM_FRAME_5B_SUBCOMD_SEND_GETDATA_LIMITANDPURCHASEDATA 0x02

//------------------------------------------------------------------------------

#define SM_GET_ONE_BYTE 0xFF

//------------------------------------------------------------------------------

#define SM_STR_POWERCUT_DETAILS_OVER_VOLTAGE "Off by Over Voltage"
#define SM_STR_POWERCUT_DETAILS_UNDER_VOLTAGE "Off by Under Voltage"
#define SM_STR_POWERCUT_DETAILS_OVER_CURRENT "Off by Over Current"
#define SM_STR_POWERCUT_DETAILS_END_PURCHASE "Off by End Purchase"
#define SM_STR_POWERCUT_DETAILS_UNKNOWN "Off by Unknown"
#define SM_STR_POWERCUT_DETAILS_NO_POWER_CUT "No Power Cut"

// Type error text
const char smStrTypeNoError[] PROGMEM = {"No error type"};
const char smStrTypeCommunicationError[] PROGMEM = {"Communication"};
const char smStrTypeInputDataError[] PROGMEM = {"Input data"};
const char smStrTypeLimitsExceededError[] PROGMEM = {"Limits exceeded"};

const char *const smStrTypeTable[] PROGMEM = {
    smStrTypeNoError,
    smStrTypeCommunicationError,
    smStrTypeInputDataError,
    smStrTypeLimitsExceededError
};

// Error text
const char smStrErrNoError[] PROGMEM = {"No Errors"};
const char smStrErrCrcError[] PROGMEM = {"CRC check failed"};
const char smStrErrWrongBytes[] PROGMEM = {"The bytes was received but are not correct"};
const char smStrErrNotEnoughtBytes[] PROGMEM = {"Not enough byes were received"};
const char smStrErrExceedsBytes[] PROGMEM = {"Expected amount of bytes exceeded"};
const char smStrErrTimeout[] PROGMEM = {"Timed out"};
const char smStrErrWrongMsg[] PROGMEM = {"No bytes received"};
const char smStrErr1PInputDataOutOfRange[] PROGMEM = {"Data outside ranges, first parameter"};
const char smStrErr2PInputDataOutOfRange[] PROGMEM = {"Data outside ranges, second parameter"};
const char smStrErr3PInputDataOutOfRange[] PROGMEM = {"Data outside ranges, third parameter"};

const char *const smStrErrTable[] PROGMEM = {
    smStrErrNoError,
    smStrErrCrcError,
    smStrErrWrongBytes,
    smStrErrNotEnoughtBytes,
    smStrErrExceedsBytes,
    smStrErrTimeout,
    smStrErrWrongMsg,
    smStrErr1PInputDataOutOfRange,
    smStrErr2PInputDataOutOfRange,
    smStrErr3PInputDataOutOfRange
};

class SmartMeter238 {
   public:
    enum smCommandTransmit {
        SM_CMD_GET_POWERCUT,
        SM_CMD_GET_MEASUREMENTDATA,
        SM_CMD_GET_LIMITANDPURCHASEDATA,

        SM_CMD_SET_LIMITDATA,
        SM_CMD_SET_PURCHASEDATA,
        SM_CMD_SET_POWERCUT,
        SM_CMD_SET_DELAY,
        SM_CMD_SET_RESET
    };

    enum smCommandReceive {
        SM_CMD_RESP_POWERCUT,
        SM_CMD_RESP_MEASUREMENTDATA,
        SM_CMD_RESP_LIMITANDPURCHASEDATA
    };

    enum smErrorType {
        SM_TYPE_NO_ERROR,
        SM_TYPE_COMMUNICATION_ERROR,
        SM_TYPE_INPUT_DATA_ERROR,
        SM_TYPE_LIMITS_EXCEEDED_ERROR
    };

    enum smErrorCode {
        SM_ERR_NO_ERROR,                     // no error
        SM_ERR_CRC_ERROR,                    // crc error
        SM_ERR_WRONG_BYTES,                  // bytes wrong from DDS2384W
        SM_ERR_NOT_ENOUGHT_BYTES,            // not enough bytes from DDS2384W
        SM_ERR_EXCEEDS_BYTES,                // exceeds bytes from DDS2384W
        SM_ERR_TIMEOUT,                      // timeout from DDS2384W
        SM_ERR_WRONG_MSG,                    // message is not valid
        SM_ERR_1P_INPUT_DATA_OUT_OF_RANGE,   // out of range first parameter
        SM_ERR_2P_INPUT_DATA_OUT_OF_RANGE,   // out of range second parameter
        SM_ERR_3P_INPUT_DATA_OUT_OF_RANGE    // out of range third parameter
    };

    typedef struct {
        struct {
            unsigned long time = 0;

            struct {
                float startingKWh = 0;
                float priceKWh = 0;
            } data;
        } powerCompanyData;

        struct {
            unsigned long time = 0;

            struct {
                bool powerCut = false;
                const char *powerCutDetails = SM_STR_POWERCUT_DETAILS_NO_POWER_CUT;

                uint16_t delay = 0;
                bool delaySetPowerCut = false;
            } data;
        } powerCutData;

        struct {
            unsigned long time = 0;

            struct {
                float current = 0;
                float voltage = 0;
                float frequency = 0;

                float reactivePower = 0;
                float activePower = 0;
                float powerFactor = 0;

                float lapseOfTimeTotalEnergy = 0;
                float lapseOfTimeImportEnergy = 0;
                float lapseOfTimeExportEnergy = 0;
                float lapseOfTimePriceEnergy = 0;

                float totalKWh = 0;
            } data;
        } measurementData;

        struct {
            unsigned long time = 0;

            struct {
                float energyPurchase = 0;
                float energyPurchaseBalance = 0;
                float energyPurchaseAlarm = 0;
                bool energyPurchaseStatus = false;

                float maxCurrentLimit = 0;
                uint16_t maxVoltageLimit = 0;
                uint16_t minVoltageLimit = 0;
            } data;
        } limitAndPurchaseData;
    } smartMeterData;

#ifdef SM_ENABLE_DEBUG
#ifdef SM_USE_REMOTE_DEBUG
    SmartMeter238(HardwareSerial &serial, RemoteDebug &debug);
#else
    SmartMeter238(HardwareSerial &serial, HardwareSerial &debug);
#endif   // SM_USE_REMOTE_DEBUG
#else
    SmartMeter238(HardwareSerial &serial);
#endif   // SM_ENABLE_DEBUG

    virtual ~SmartMeter238();

    void begin(void);

    bool getPowerCutData(smartMeterData *dataObject, bool forceUpdate = false);
    bool getMeasurementData(smartMeterData *dataObject, bool forceUpdate = false);
    bool getLimitAndPurchaseData(smartMeterData *dataObject, bool forceUpdate = false);
    bool getPowerCompanyData(smartMeterData *dataObject, bool forceUpdate = false);

    bool setLimitsData(float maxCurrentLimit, uint16_t maxVoltageLimit, uint16_t minVoltageLimit, smartMeterData *dataObject);
    bool setPurchaseData(float energyPurchase, float energyPurchaseAlarm, bool energyPurchaseStatus, smartMeterData *dataObject);
    bool setPowerCutData(bool powerCut, smartMeterData *dataObject);
    bool setDelay(bool delaySetPowerCut, uint16_t delay, smartMeterData *dataObject);
    bool setReset(smartMeterData *dataObject);

    bool setPowerCompanyData(float startingKWh, float priceKWh, smartMeterData *dataObject);

#ifdef SM_ENABLE_RAW_TEST_MSG
    bool sendHexMessage(const char *msg);
    bool processIncomingMessages();
    char *getIncomingHexMessage();
#endif

    smErrorType getErrType(bool clear = false);
    smErrorCode getErrCode(bool clear = false);

    uint16_t getErrCount(bool clear = false);
    uint16_t getSuccCount(bool clear = false);

    void clearErrType();
    void clearErrCode();
    void clearErrCount();
    void clearSuccCount();

    char prtStrType[SM_MAX_STR_LENGTH_TYPE];
    char prtStrError[SM_MAX_STR_LENGTH_ERROR];

    char *getTypeStr(bool clear = false);
    char *getErrorStr(bool clear = false);

   private:
    smErrorType errType = SM_TYPE_NO_ERROR;
    smErrorCode errCode = SM_ERR_NO_ERROR;

    uint16_t readingErrCount = 0;
    uint32_t readingSuccessCount = 0;

    HardwareSerial &smSerial;

#ifdef SM_ENABLE_DEBUG
#ifdef SM_USE_REMOTE_DEBUG
    RemoteDebug &smDebug;
#else
    HardwareSerial &smDebug;
#endif   // SM_USE_REMOTE_DEBUG
#endif   // SM_ENABLE_DEBUG

    const unsigned long minIntervalUpdate = SM_MIN_INTERVAL_TO_GET_DATA;

    bool transmitSerialData(uint8_t *array, uint8_t size);
    bool preTransmitSerialData(smCommandTransmit cmd, uint8_t frameSize, uint8_t *array = nullptr);

    bool receiveSerialData(uint8_t *array, uint8_t size, uint8_t command, uint8_t subCommand, uint8_t typeMessage);
    bool preReceiveSerialData(smCommandReceive cmd, uint8_t frameSize, smartMeterData *dataObject);

    uint8_t calculateCRC(uint8_t *array, uint8_t size);

#ifdef SM_ENABLE_RAW_TEST_MSG
    char incomingHexMessage[SM_MAX_HEX_MSG_LENGTH];
    uint8_t incomingByteMessage[SM_MAX_BYTE_MSG_BUFFER];
    int char2int(char input);
#endif

#ifdef SM_ENABLE_DEBUG
    void printError(bool clear);
    void printMessage(uint8_t *array, uint8_t size);
    void printByte(uint8_t byte, bool prefix);
#endif   // SM_ENABLE_DEBUG
};
#endif   // SmartMeter238_h
