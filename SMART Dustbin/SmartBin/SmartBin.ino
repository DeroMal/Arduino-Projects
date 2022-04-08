#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

// WiFi and for ThingSpeak connection
String apiKey = "P63NLGDIDL34WGNM";
const char* ssid = "JohnBV";
const char* password = "12345678910";
const char* server = "api.thingspeak.com";
WiFiClient client;

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(D2, D1); //SIM800L Tx & Rx is connected to Arduino #3 & #2
String message;

const int TRIG_PIN = D7;
const int ECHO_PIN = D6;
int distance;

const unsigned int BAUD_RATE = 115200;

u_int32_t lastsms = 0;
const u_int32_t interval = 1L * 60L * 1000L; //Time interval of 1 hour
int c1 = 0, c2 = 0;

//----------------------------SETUP FUNCTION--------------------------------------------
void setup() {
  Serial.begin(BAUD_RATE);
  mySerial.begin(BAUD_RATE);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);



  //----------------------WiFi Set-up------------------------
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

//-----------------------LOOP FUNCTION---------------------------------------------
void loop() {

  Sensor();
  inet();
  
//------------------------sends sms------------------------------------------  
  int elapse = (millis() / 60000);
  Serial.print("elapse = ");
  Serial.println(elapse);
  if (distance <= 5 && (millis() - lastsms) >= interval && c1 <= 3) {
    message = "Hello Derrick!\nThe dustbin is full";
    sms();
    lastsms = millis();
    c1++;
    Serial.println(c1);
  }
  else if (distance >= 6 && distance <= 10 && (millis() - lastsms) >= interval && c2 <= 2) {
    message = "Hello Derrick!\nThe dustbin is almost getting full. Prepare to empty";
    sms();
    lastsms = millis();
    c2++;
    Serial.println(c2);
  }
  else if (distance > 10) {
    c1 = 0;
    c2 = 0;
  }
//--------------------------------------------------------------------------------------------------

}

//-----------------FUNCTION TO READ SENSOR VALUE-------------------
void Sensor() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration / 29 / 2;
  if (distance >= 450){
    distance = 450;
  }
  if (duration == 0) {
    Serial.println("Warning: no pulse from sensor");
  }
  else {
    Serial.print("distance to nearest object:");
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(2000);
}

//-----------------FUNCTION TO SEND SMS-----------------------------------
void sms() {
  Serial.println("Initializing...");
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+256774546556\"");//phone number SMS to be sent to
  updateSerial();
  mySerial.print(message); //text content
  updateSerial();
  mySerial.write(26);

}
//-------------------------FUNCTION FOR SENDING TO THINGSPEAK----------------------
void inet(){
  Sensor();
  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(distance);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("Sending data to Thingspeak");
    Serial.println("--------------------------------------------------------");
  }
  client.stop();

  Serial.println("Waiting 20 secs");
  // thingspeak needs at least a 15 sec delay between updates
  // 20 seconds to be safe
  delay(20000);
}

//----------------------FUNCTION TO READ DATA RECEIVED THORGH GSM FROM SERIAL PORT-------------------------------------
void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
