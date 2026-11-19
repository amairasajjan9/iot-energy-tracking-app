#include "Network.h"
#include "time.h"

#include "wear_levelling.h"
#include <Arduino.h>
#include <Preferences.h>
#include <stdio.h>

#include "EmonLib.h" // Include Emon Library

EnergyMonitor emon1; // Create an instance
Preferences preferences;

Network *network;

long double Units = 0;
int wifi_indicator_pin = 27;
int general_indicator_pin = 26;
int updateChecks = 0;
int loops = 0;
int state = 0;
int start;
int temp = 0;

const char *ntpServer = "pool.ntp.org";
int systemLastUpdatedate;
int systemLastUpdatemonth;
int systemLastUpdateyear;
int systemLastUpdatehour;
int systemLastUpdatemin;

const uint32_t NVM_Offset = 0x290000;
uint8_t FLASH_Address = 0;
uint8_t FLASH_Address2 = 100;

template <typename T>
void FlashWrite(uint32_t address, const T &value)
{
  ESP.flashEraseSector((NVM_Offset + address) / 4096);
  ESP.flashWrite(NVM_Offset + address, (uint32_t *)&value, sizeof(value));
}

template <typename T>
void FlashRead(uint32_t address, T &value)
{
  ESP.flashRead(NVM_Offset + address, (uint32_t *)&value, sizeof(value));
}

void config()
{
  configTime(0, 0, ntpServer);
}

void setup()
{

  start = millis();
  Serial.begin(9600);
  preferences.begin("meterReadings");

  network = new Network();
  network->initWifi(wifi_indicator_pin);
  config();

  emon1.voltage(35, 234.26, 1.7); // Voltage: input pin, calibration, phase_shift
  emon1.current(34, 111.1);       // Current: input pin, calibration.
  // emon1.current(34, 66);                  // Current: input pin, calibration.
  pinMode(wifi_indicator_pin, OUTPUT);    // WiFi Indicator
  pinMode(general_indicator_pin, OUTPUT); // Working Indicator

  // double temp;
  // // FlashRead<double>(FLASH_Address, temp);
  // Serial.println();
  // Serial.print("temp ");
  // Serial.println(temp);
  // Serial.printf(

  // );
  preferences.begin("UnitsData", false);

  Units = preferences.getDouble("Units", 0);

  Serial.print(double(Units));

  Serial.println("  this is end of setup");
}

void loop()
{
  // digitalWrite(27, HIGH);
  // digitalWrite(26, LOW);
  // int start = millis();
  emon1.calcVI(20, 50); // Calculate all. No.of half wavelengths (crossings), time-out
  emon1.serialprint();  // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

  float realPower = emon1.realPower;         // extract Real Power into variable
  float apparentPower = emon1.apparentPower; // extract Apparent Power into variable
  float powerFActor = emon1.powerFactor;     // extract Power Factor into Variable
  double supplyVoltage = emon1.Vrms;         // extract Vrms into Variable
  double Irms = emon1.Irms;
  // Serial.print("time = ");
  // Serial.println(millis() - start);

  Units += (Irms * supplyVoltage * 200 * 0.001) / (3.6 * 1000000);

  Serial.println(double(Units));
  if (loops >= 15)
  {
    if (state == 0)
    {
      state = 1;
      digitalWrite(general_indicator_pin, HIGH);
    }
    else
    {
      state = 0;
      digitalWrite(general_indicator_pin, LOW);
    }
    loops = 0;
  }
  loops++;
  
  Serial.println();
  if (updateChecks >= 50)
  {
    network->firestoreDataUpdate(Units);

    updateChecks = 0;
  }
  updateChecks++;
  if (millis() - start >= 9000000)
  // if (millis() - start >= 900)
  { // 15 mins

    preferences.putDouble("Units", Units);
    start = millis();
    Serial.println("Units saved to preferences");
  }
  //   if(temp == 0){
  //   FlashWrite<double>(FLASH_Address, Units);
  // temp = 2;
  //   }
}
