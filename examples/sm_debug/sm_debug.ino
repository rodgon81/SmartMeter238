/*
Library for reading DDS238-4 W Wifi Smart meter (SM).
Reading via Hardware Serial
2020 (development with PlatformIO IDE for VSCode & esp8266 core)

MIT License

Copyright (c) 2020 Rodrigo González Zárate

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

#include <Arduino.h>

#include <SmartMeter238.h>   //import SmartMeter238 library

//-----------------------------------------------------------------------

// Only for debug purposes
HardwareSerial &meter = Serial;
HardwareSerial &debug = Serial1;

//-----------------------------------------------------------------------

#ifdef SM_ENABLE_DEBUG
SmartMeter238 sm(meter, debug);   // config SmartMeter238 with debug
#else
SmartMeter238 sm(meter);   // config SmartMeter238
#endif

// Data storage
SmartMeter238::smartMeterData smData;

void setup() {
    debug.begin(9600);   // Start Serial Debug

    sm.begin();   // initialize SmartMeter238 communication
}

void loop() {
    debug.println();
    debug.println("- START ----------------------------------------------------------------");
    debug.println();
    debug.println("- CALL COMMAND ---------------------------------------------------------");

    debug.println("getPowerCutData:\t\t\t");     (sm.getPowerCutData(&smData, true)                       ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("getMeasurementData:\t\t");    (sm.getMeasurementData(&smData, true)                    ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("getLimitAndPurchaseData:\t"); (sm.getLimitAndPurchaseData(&smData, true)               ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("getPowerCompanyData:\t\t");   (sm.getPowerCompanyData(&smData, true)                   ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setLimitsData:\t\t\t");       (sm.setLimitsData(50.00, 270, 175, &smData)              ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setPurchaseData:\t\t");       (sm.setPurchaseData(9999.99, 999.99, SM_SET_ON, &smData) ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setPowerCutData:\t\t\t");     (sm.setPowerCutData(SM_SET_ON, &smData)                  ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setDelay:\t\t\t");            (sm.setDelay(SM_SET_ON, 99, &smData)                     ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setReset:\t\t\t");            (sm.setReset(&smData)                                    ? debug.println("Ok") : debug.println("Fail")); debug.println();
    debug.println("setPowerCompanyData:\t\t");   (sm.setPowerCompanyData(99999.99, 999.99, &smData)       ? debug.println("Ok") : debug.println("Fail")); debug.println();

    debug.println("- PRINT MEASURENMENT DATA ----------------------------------------------");

    debug.print("time:\t\t\t\t");                debug.print(smData.measurementData.time);                         debug.println();
    debug.print("current:\t\t\t");               debug.print(smData.measurementData.data.current);                 debug.println(" A");
    debug.print("voltage:\t\t\t");               debug.print(smData.measurementData.data.voltage);                 debug.println(" V");
    debug.print("frequency:\t\t\t");             debug.print(smData.measurementData.data.frequency);               debug.println(" Hz");
    debug.print("reactivePower:\t\t\t");         debug.print(smData.measurementData.data.reactivePower);           debug.println(" kVAr");
    debug.print("activePower:\t\t\t");           debug.print(smData.measurementData.data.activePower);             debug.println(" kW");
    debug.print("powerFactor:\t\t\t");           debug.print(smData.measurementData.data.powerFactor);             debug.println(" PF");
    debug.print("lapseOfTimeTotalEnergy:\t\t");  debug.print(smData.measurementData.data.lapseOfTimeTotalEnergy);  debug.println(" kWh");
    debug.print("lapseOfTimeImportEnergy:\t");   debug.print(smData.measurementData.data.lapseOfTimeImportEnergy); debug.println(" kWh");
    debug.print("lapseOfTimeExportEnergy:\t");   debug.print(smData.measurementData.data.lapseOfTimeExportEnergy); debug.println(" kWh");
    debug.print("lapseOfTimePriceEnergy:\t\t");  debug.print(smData.measurementData.data.lapseOfTimePriceEnergy);  debug.println(" $");
    debug.print("totalKWh:\t\t\t");              debug.print(smData.measurementData.data.totalKWh);                debug.println(" kWh");

    debug.println();

    debug.println("- PRINT POWER COMPANY DATA ---------------------------------------------");

    debug.print("time:\t\t\t\t");                debug.print(smData.powerCompanyData.time);                         debug.println();
    debug.print("startingKWh:\t\t\t");           debug.print(smData.powerCompanyData.data.startingKWh);             debug.println(" kWh");
    debug.print("priceKWh:\t\t\t");              debug.print(smData.powerCompanyData.data.priceKWh);                debug.println(" $");

    debug.println();

    debug.println("- PRINT LIMIT AND PURCHASE DATA ----------------------------------------");

    debug.print("time:\t\t\t\t");                debug.print(smData.limitAndPurchaseData.time);                         debug.println();
    debug.print("energyPurchase:\t\t\t");        debug.print(smData.limitAndPurchaseData.data.energyPurchase);          debug.println(" kWh");
    debug.print("energyPurchaseBalance:\t\t");   debug.print(smData.limitAndPurchaseData.data.energyPurchaseBalance);   debug.println(" kWh");
    debug.print("energyPurchaseAlarm:\t\t");     debug.print(smData.limitAndPurchaseData.data.energyPurchaseAlarm);     debug.println(" kWh");
    debug.print("energyPurchaseStatus:\t\t");    debug.print(smData.limitAndPurchaseData.data.energyPurchaseStatus);    debug.println();
    debug.print("maxCurrentLimit:\t\t");         debug.print(smData.limitAndPurchaseData.data.maxCurrentLimit);         debug.println(" A");
    debug.print("maxVoltageLimit:\t\t");         debug.print(smData.limitAndPurchaseData.data.maxVoltageLimit);         debug.println(" V");
    debug.print("minVoltageLimit:\t\t");         debug.print(smData.limitAndPurchaseData.data.minVoltageLimit);         debug.println(" V");

    debug.println();

    debug.println("- PRINT POWER CUT DATA -------------------------------------------------");

    debug.print("time:\t\t\t\t");                debug.print(smData.powerCutData.time);                         debug.println();
    debug.print("powerCut:\t\t\t");              debug.print(smData.powerCutData.data.powerCut);                debug.println();
    debug.print("powerCutDetails:\t\t");         debug.print(smData.powerCutData.data.powerCutDetails);         debug.println();
    debug.print("delay:\t\t\t\t");               debug.print(smData.powerCutData.data.delay);                   debug.println(" min");
    debug.print("delaySetPowerCut:\t\t");        debug.print(smData.powerCutData.data.delaySetPowerCut);        debug.println();

    debug.println();

    debug.println("- END ------------------------------------------------------------------");

    delay(3000);
}