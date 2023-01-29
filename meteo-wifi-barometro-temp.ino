//Libraires
#include <SFE_BMP180.h>
#include <Wire.h>
#include <DHT.h>
#include <stdlib.h>

// You will need to create an SFE_BMP180 object, here called "pressure":

SFE_BMP180 bmp180;
int Altitude = 170; //current altitude in meters

/*------------------------DHT SENSOR------------------------*/
#define DHTPIN 2         // DHT data pin connected to Arduino pin 2
#define DHTTYPE DHT22     // DHT 22 (or AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
/*----------------------------------------------------------*/

/*-----------------ESP8266 Serial WiFi Module---------------*/
#define SSID "TIM-29103709"     // "SSID-WiFiname" 
#define PASS "IzH9qOGmRQBccdC1u4FtyLA8"       // "password"
#define IP "184.106.153.149"// thingspeak.com ip
String msg = "GET /update?key=95D8ISXLY4VZOTRQ"; //change it with your key...
/*-----------------------------------------------------------*/

//Variables
float temp;
float hum;
String tempC;
float pressione;


int error;

void setup() {
  Serial.begin(115200); //or use default 115200.  
  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }

  //Serial.begin(9600);
  bool success = bmp180.begin();

  if (success) {
    Serial.println("BMP180 init success");
  }

}

void loop() {
  
  //Read temperature and humidity values from DHT sensor:
  start: //label 
  error=0;
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  char buffer[10];
  
  // there is a useful c function called dtostrf() which will convert a float to a char array 
  //so it can then be printed easily.  The format is: dtostrf(floatvar, StringLengthIncDecimalPoint, numVarsAfterDecimal, charbuf);
  tempC = dtostrf(temp, 4, 1, buffer); 



  //PRESSURE 

  char status;
  double T, P;
  bool success = false;

  status = bmp180.startTemperature();

  if (status != 0) {
    delay(1000);
    status = bmp180.getTemperature(T);

    if (status != 0) {
      status = bmp180.startPressure(3);

      if (status != 0) {
        delay(status);
        status = bmp180.getPressure(P, T);

        if (status != 0) {
          pressione = bmp180.sealevel(P, Altitude);
          
          //Serial.print("Pressure: ");
          //Serial.print(comp);
          //Serial.println(" hPa");

          //Serial.print("Temperature: ");
          //Serial.print(T);
          //Serial.println(" C");
        }
      }
    }
  }
  
  //pressione= a;

  updateTemp();
  //Resend if transmission is not completed 
  if (error==1){
    goto start; //go to label "start"
  }
  
  delay(1800000); //Update every 1 minute

}



void updateTemp(){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    return;
  }
  cmd = msg ;
  cmd += "&field1=";     //field 1 for temperature
  cmd += tempC;
  cmd += "&field2=";  //field 2 for humidity
  cmd += String(hum);
  cmd += "&field3=";  //field 3 for Pressure
  cmd += String(pressione);
  cmd += "\r\n\r\n";
  
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")){
    Serial.print(cmd);
  }
  else{
    Serial.println("AT+CIPCLOSE");
    //Resend...
    error=1;
  }
}

 
boolean connectWiFi(){
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  delay(5000);
  if(Serial.find("OK")){
    return true;
  }else{
    return false;
  }
}
