// This #include statement was automatically added by the Particle IDE.
#include "ThingSpeak/ThingSpeak.h"

// This #include statement was automatically added by the Particle IDE.
#include "DS18B20/DS18B20.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunBME280/SparkFunBME280.h"

// This #include statement was automatically added by the Particle IDE.
#include "CE_BME280/CE_BME280.h"

int relay1 = D3;
int relay2 = D2;
int relay3 = D5;
int relay4 = D4;

BME280 mySensor;

DS18B20 ds18b20 = DS18B20(D6); //Sets Pin D6 for Water Temp Sensor
int led = D7;
char szInfo[64];
float pubTemp;
double celsius;
double fahrenheit;
unsigned int Metric_Publish_Rate = 30000;
unsigned int MetricnextPublishTime;
int DS18B20nextSampleTime;
int DS18B20_SAMPLE_INTERVAL = 2500;
int dsAttempts = 0;

unsigned int sampleNumber = 0;

//Thingspeak examples
TCPClient client;
unsigned long myChannelNumber = 138882;
const char * myWriteAPIKey = "RRJCMTYMGY48V9H4";

void setup() {

    Time.zone(+1);

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(relay3, OUTPUT);
    pinMode(relay4, OUTPUT);

    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);

	//***Driver settings********************************//
	mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x76;


	//***Operation settings*****************************//
	mySensor.settings.runMode = 3; //  3, Normal mode
	mySensor.settings.tStandby = 0; //  0, 0.5ms
	mySensor.settings.filter = 0; //  0, filter off
	mySensor.settings.tempOverSample = 1;
    mySensor.settings.pressOverSample = 1;
	mySensor.settings.humidOverSample = 1;

	Serial.begin(57600);
	Serial.print("Program Started\n");
	Serial.print("Starting BME280... result of .begin(): 0x");
	delay(100);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	//Calling .begin() causes the settings to be loaded
	Serial.println(mySensor.begin(), HEX);

	//Build a first-row of column headers
	Serial.print("\n\n");
	Serial.print("Sample,");
	Serial.print("T(deg C),");
	Serial.print("T(deg F),");
	Serial.print("P(Pa),");
	Serial.print("Alt(m),");
	Serial.print("Alt(ft),");
	Serial.print("%RH");
	Serial.println("");

	//Water temp sensor setup
	Time.zone(-5);
    Particle.syncTime();
    pinMode(D6, INPUT);
    Particle.variable("tempHotWater", &fahrenheit, DOUBLE);
    Serial.begin(115200);

    //ThingSpeak setup
    //Ethernet.begin(mac);
    ThingSpeak.begin(client);

}


void loop() {

    Time.zone(IsDST(Time.day(), Time.month(), Time.weekday())? +2 : +1);

    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    //Grow Light is on from 06:00 to 0:00 and of from midnight to six in the morning
    if(Time.hour() > 6)
    {
        digitalWrite(relay1, LOW);
        ThingSpeak.setField(5,1);
        //Need to log this
    }

    if(Time.hour() < 6)
    {
        digitalWrite(relay1, HIGH);
        ThingSpeak.setField(5,0);
        //Need to log this
    }

    float sensorReading = mySensor.readTempC();
    ThingSpeak.setField(1,sensorReading);

    sensorReading = mySensor.readFloatPressure();
    ThingSpeak.setField(2,sensorReading);

    sensorReading = mySensor.readFloatHumidity();
    ThingSpeak.setField(3,sensorReading);


    if (millis() > DS18B20nextSampleTime){
       if(!ds18b20.search()){
            ds18b20.resetsearch();
            sensorReading = ds18b20.getTemperature();
      while (!ds18b20.crcCheck() && dsAttempts < 4){
        Serial.println("Caught bad value.");
        dsAttempts++;
        Serial.print("Attempts to Read: ");
        Serial.println(dsAttempts);
        if (dsAttempts == 3){
          delay(1500);
        }
        ds18b20.resetsearch();
        sensorReading = ds18b20.getTemperature();
        continue;
      }
    }

    ThingSpeak.setField(4,sensorReading);

    }

    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    //delay(5000); //5 second delay
	delay(600000); // 10min delay

}

bool IsDST(int dayOfMonth, int month, int dayOfWeek)
{
  if (month < 3 || month > 11)
  {
    return false;
  }
  if (month > 3 && month < 11)
  {
    return true;
  }
  int previousSunday = dayOfMonth - dayOfWeek;
  //In march, we are DST if our previous sunday was on or after the 8th.
  if (month == 3)
  {
    return previousSunday >= 8;
  }
  //In november we must be before the first sunday to be dst.
  //That means the previous sunday must be before the 1st.
  return previousSunday <= 0;
}

void publishData(){
  if(!ds18b20.crcCheck()){
    return;
  }
  sprintf(szInfo, "%2.2f", fahrenheit);
  Particle.publish("dsTmp", szInfo, PRIVATE);
  MetricnextPublishTime = millis() + Metric_Publish_Rate;
}

/*void getTemp(){
    if(!ds18b20.search()){
      ds18b20.resetsearch();
      celsius = ds18b20.getTemperature();
      //Serial.print("Celsius: ");
      //Serial.println(celsius);
      while (!ds18b20.crcCheck() && dsAttempts < 4){
        Serial.println("Caught bad value.");
        dsAttempts++;
        Serial.print("Attempts to Read: ");
        Serial.println(dsAttempts);
        if (dsAttempts == 3){
          delay(1000);
        }
        ds18b20.resetsearch();
        celsius = ds18b20.getTemperature();
        continue;
      }
      dsAttempts = 0;
      fahrenheit = ds18b20.convertToFahrenheit(celsius);
      DS18B20nextSampleTime = millis() + DS18B20_SAMPLE_INTERVAL;
      Serial.print("Fahrenheit: ");
      Serial.println(fahrenheit);
    }
}*/
