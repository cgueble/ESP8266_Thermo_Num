#include <Arduino.h>
#line 1 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
#line 1 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Codes_CGU.h>// contient toutes les variables avec les loggins et les codes

//****************Configuration pour Debug****************
const boolean SERIAL_PORT_LOG_ENABLE = true; //true pour avoir la console active et false pour la desactiver ; il faut la desactiver pour l'application car meme pour que "verrou"
const boolean AvecMail = false; //true pour activer les mails
const boolean AvecWifi = false; // true pour activer le wifi

//Declaration des compteurs


String CurentSSID;
String CurentSSIDTry;
//Definition des chaines de characteres pour les mails
String MailContent = "no mail content defined\r\n";
String StringDate = "no date";
String StringTime = "no time";

//Definition des Inputs

//Definition des Outputs
int thermo_Power = 2; //l'alimentation des thermometre se fera sur GPIO2 c'est a dire D2 c'est a dire la pin 3 de la carte ESP8266-E01 ; c'est une output

#define ONE_WIRE_BUS 0 // ONEWIRE is plugged into pin 5 on GPIO_0 the Arduino
#define TEMPERATURE_PRECISION 9
  
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

//Sensor Adresses 
DeviceAddress Sensor1 = { 0x28, 0xFF, 0x89, 0x0F, 0x61, 0x16, 0x03, 0x40 };
DeviceAddress Sensor2 = { 0x28, 0xFF, 0xE2, 0x0F, 0x61, 0x16, 0x03, 0x21 };
DeviceAddress Sensor3 = { 0x28, 0xFF, 0x38, 0xB3, 0x60, 0x16, 0x03, 0xEA };

byte Sensor_addr[8][8];

//Definition var for temperature monitoring
int ThempInt = 90;
int ThempChauf = 90;
int ThempMin = 5;// Temperature min a partir de laquelle 1 mail par jour est envoyé 
int amountOfSensors = 3;
int counterCheckBus = 0;

//Variables de gestion de l'etat de la porte et des mails
char server_email[] = "smtp.orange.fr";
const unsigned int localPort = 2390;


String MailToSend = "";
boolean DailyMailSent = true;
boolean DoorClosedBack = true; //true : porte refermee
boolean Update_needed = false;
long rssi; //pour mesure RSSI

//variables pour le gestion de la date et du temps
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long secsSince1900 = 0;
const unsigned long seventyYears = 2208988800UL;
int Hour;
int Minute;
int Second;
int Day;
int DayofWeek; // Sunday is day 0
int Month;     // Jan is month 0
int Year;      // the Year minus 1900
int WeekDay;

//Variable pour la gestion de thingspeak
const char* server = "mqtt.thingspeak.com";// Define the ThingSpeak MQTT broker
unsigned long lastConnectionTime = 0; // track the last connection time
const unsigned long postingInterval = 1L * 1000L;// post data every 1 seconds
const unsigned long RegularpostingInterval = 10 * 60 * 1L * 1000L;// post data every 10 min
time_t epoch = 0; // contient
IPAddress ip;
IPAddress timeServerIP;

WiFiClient client;  // Initialize the Wifi client library.
WiFiUDP udp; //A UDP instance to let us send and receive packets over UDP
PubSubClient mqttClient(client); // Initialize the PuBSubClient library

//************************Debut de setup*************************
#line 89 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void setup();
#line 168 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void loop();
#line 203 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void UpdateTime();
#line 289 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void GetTimeByUDP();
#line 341 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void sendNTPpacket(IPAddress& address);
#line 367 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void sendDailyMail();
#line 390 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
byte sendEmail(String FcMailFrom, String FcMailTo, String FcMailContent, String FcThingspeakChannelAdress, String FcThingspeakChannelWriteAPIKey);
#line 481 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
byte eRcv();
#line 519 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void efail();
#line 553 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void reconnect();
#line 583 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void mqttpublish();
#line 601 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void mqttpublishtry();
#line 620 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void WifiConnexionManager();
#line 675 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void WifiConnectOwner(char* SSIDowner_fct, char* passwordowner_fct);
#line 707 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void WaitConnexion();
#line 756 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
int getSensorsOnBus();
#line 761 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void discoverOneWireDevices(byte Sensor_addr[8][8]);
#line 89 "C:\\Users\\cgueble\\Documents\\Arduino\\52-Wifi-thermo-num\\Wifi-thermo-num\\Wifi-thermo-num.ino"
void setup() {
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.begin(115200);
    Serial.print("debut de setup. SERIAL_PORT_LOG_ENABLE= ");
    Serial.println(SERIAL_PORT_LOG_ENABLE);
  }
  //Set up PIN in INPUT


//Set up PIN in OUTPUT
  pinMode(thermo_Power, OUTPUT);// on definit comme output le port qui servira a monitorer la temperature
  digitalWrite(thermo_Power, LOW);// on commence par couper l'alim des capteurs
 
  
//Debut de connexion intenet  
  while ((WiFi.status() != WL_CONNECTED) && AvecWifi) {
    WifiConnexionManager();//Recherche reseau wifi + connexion
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print(".");
      delay(1000);
      Serial.print(".");
      delay(1000);
      Serial.print(".\r");
      delay(1000);
      Serial.println("   \r");
    }
  }//Loop connexion until Wifi Connetion is OK

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  }
 if(AvecWifi){
  udp.begin(localPort);// Start UDP port for connexion to NTP server

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Starting UDP");
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
  }

  GetTimeByUDP();
  UpdateTime();
  mqttClient.setServer(server, 1883);// Set the MQTT broker details
}
	//Init des sensors
	//Initialize the Temperature measurement library
	sensors.begin();

	amountOfSensors = getSensorsOnBus();
	Serial.print("number of sensors is: "); 
	Serial.println(amountOfSensors);

	//ici il faut recuprer les adresses des sensors
	
	discoverOneWireDevices(Sensor_addr);
 
 
 
	
	//Set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
	sensors.setResolution(Sensor1, 10);
	sensors.setResolution(Sensor2, 10);
	sensors.setResolution(Sensor3, 10);


	Serial.println("System initialized");  
    
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("End of Setup");
  }
}// fin set up
//******************end of setup*************************************
//*******************************************************************
//******************start of loop************************************
void loop() {

  //LoopLog
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.print(".\r");
  delay(100);
  Serial.print("0\r\n");
  }

  if(AvecWifi){
  WifiConnectOwner((char*)ssid1,(char*)password1);
  UpdateTime();
  rssi = WiFi.RSSI();
  if (Year < 2016) { // si l'heure n'est pas configuree C est a dire si l'annee n'est pas bonne, alors on recupere l'heure sur le serveur NTP
    GetTimeByUDP();
    UpdateTime();
  }

  if (WiFi.status() != WL_CONNECTED) {// si la connexion WIFI est perdue, alors on relance le setup
    WiFi.disconnect();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Connexion lost ");
    }
    setup();
  }
 }
  


//ESP.deepSleep(24*3600*1000000, WAKE_NO_RFCAL);  // Sleep pour 1 jour
//ESP.deepSleep(2*60*1000000, WAKE_NO_RFCAL); // Sleep pour 2 minutes
//Waikup on GPIO16 reset.... witch Pin ???

}//*****************END OF LOOP *******************

void UpdateTime() {
  StringDate = ""; // reset StringDate value
  StringTime = ""; // reset StringTime value
  Hour = hour();
  Minute = minute();
  Second = second();
  Year = year();
  Month = month();
  Day = day();
  WeekDay = weekday();

  if ((Month <= 3) || (Month > 10)) { // on est un mois d'hiver
    if ((Month == 3) && (Day - WeekDay > 24)) { //on est dans le dernier mois d'hiver et aprÃ¨s le dernier dimanche
      Hour = Hour + 2;
      StringTime += " UTC+2: ";
      if (Hour >= 22){
        Day = Day + 1;
      }
    }
    else { // toute la periode hivernale
      Hour = Hour + 1;
      StringTime += " UTC+1: ";
        if (Hour >= 23){
        Day = Day + 1;
      }
    }
  }
  else { // on est un mois d'aout
    if ((Month == 10) && (Day - WeekDay > 24)) { //on est dans le dernier mois d'aout et apres le dernier dimanche
      Hour = Hour + 1;
      StringTime += " UTC+1: ";
      if (Hour >= 23){
        Day = Day + 1;
      }
    }
    else { // toute la periode estivale
      Hour = Hour + 2;
      StringTime += " UTC+2: ";
      if (Hour >= 22){
        Day = Day + 1;
      }
    }
  }

  //StringDate += "Date: ";
  if (Day < 10) {
    StringDate += "0";
  }
  StringDate += Day;
  if (Month < 10) {
    StringDate += "/0";
  }
  else {
    StringDate += "/";
  }
  StringDate += Month;
  StringDate += "/";
  StringDate += Year;

  if (Hour < 10) {
    StringTime += "0";
  }
  StringTime += Hour;
  if (Minute < 10) {
    StringTime += ":0";
  }
  else {
    StringTime += ":";
  }
  StringTime += Minute;
  if (Second < 10) {
    StringTime += ":0";
  }
  else {
    StringTime += ":";
  }
  StringTime += Second;
  if (SERIAL_PORT_LOG_ENABLE) {
    //Serial.print("valeure de StringDate ");
    //Serial.println(StringDate);
    //Serial.print("valeure de StringTime ");
    //Serial.println(StringTime);
  }
}//End of Update Time


void GetTimeByUDP() {
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);


  int cb = udp.parsePacket();

  if (!cb) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("no packet yet");
    }
  }
  else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("packet received, length=");
      Serial.println(cb);
    }

    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);
    }

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    setTime(epoch);

  }
}



// send an NTP request to the time server at the given address
//unsigned long sendNTPpacket(IPAddress& address)
void sendNTPpacket(IPAddress& address)
{
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("sending NTP packet...");
  }
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void sendDailyMail() {    
    GetTimeByUDP();
//    Read_Button(IlsPorte, Verrou);
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Daily mail");
    }
    MailContent = String("Daily report: rue de Vaugirard \r\n");
    if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) { // Mail sent success
      DailyMailSent = true;
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email sent");
      }
    }
    else // Mail sent failure
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email not sent - System frozen for 3 minutes");
      }
      delay(180000); // on bloque l'envoie du Daily mai pour 3 minutes avant prochain envoie
    }
  }//FIN sendDailyMail

byte sendEmail(String FcMailFrom, String FcMailTo, String FcMailContent, String FcThingspeakChannelAdress, String FcThingspeakChannelWriteAPIKey)
{
  if (!AvecMail) return 0; // sort de la procedure sans envoyer le mail si Avec mail=0

  if (client.connect(server_email, 25)) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("connected");
    }
  } else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("connection failed");
    }
    return 0;
  }

  if (!eRcv()) return 0;

  // change to your public ip
  client.println("helo 10.62.66.144");

  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending From");
  }

  // change to your email address (sender)
  client.print("MAIL From: ");
  client.println(FcMailFrom);// contient l'adresse email de l'expediteur "<toto@titi.fr>"

  if (!eRcv()) return 0;

  // change to recipient address
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending To");
  }
  client.print("RCPT To: ");
  client.println(FcMailTo);// contient l'adresse email du destinataire "<toto@titi.fr>"
  if (!eRcv()) return 0;

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending DATA");
  }

  client.println("DATA");
  if (!eRcv()) return 0;


  //*****************Creation du Mail***********************
  client.print("To: You "); // Destinataire
  client.println(FcMailTo); // Destinataire
  client.print("From: Me "); // My address
  client.println(FcMailFrom); // My address
  client.println("Subject: Arduino Report from Maison Paris\r\n"); //sujet du mail
  //Corps du mail
  client.print(FcMailContent); // message specifique de l'application appelante

  
 // }

  client.print(StringDate);
  client.println(StringTime);

  client.println(FcThingspeakChannelAdress);

  client.print("http:///");
  client.println(WiFi.localIP());

  client.print("SW version: xx/01/2018  ;  "); // Date de compilation
  client.println("21-Wifi-thermo-num"); // chemin

  //fin du mail
  client.println(".");
  if (!eRcv()) return 0; //test si mail correctement envoye

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending email");
  }

  client.println("QUIT"); //Deconnection du server mail
  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending QUIT");
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
  return 1;
  UpdateTime();
}// fin sendEmail

byte eRcv(){
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println("QUIT");

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
}

void reconnect()
{
  int connect_counter = 20;
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Connect to the MQTT broker
    if (mqttClient.connect(ThingspeakClientID, ThingspeakUserID, ThingspeakUserPwd))
    {
      Serial.println("connected");
    } else
    {
      connect_counter = connect_counter - 1;
      if ((mqttClient.state() == -3) or (connect_counter < 0))
      {
        Serial.println("-3 : MQTT_CONNECTION_LOST - the network connection was broken ... launch setup");
        setup();
      }
      Serial.print("failed, rc=");
      // Print to know why the connection failed
      // See http://pubsubclient.knolleary.net/api.html#state for the failure code and its reason
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying to connect again
      delay(5000);
    }
  }
}

void mqttpublish() {
  String data = String("field1=");
  //data = String("field1=" + String(Status_Verrou, DEC) + "&field2=" + String(Status_IlsPorte, DEC) + "&field3=" + String(rssi, DEC));

  // Get the data string length
  int length = data.length();
  char msgBuffer[length];
  char* PublishCmd = "channels/9245/publish/SSUT4K9H6PVUIRNT";
  // Convert data string to character buffer
  data.toCharArray(msgBuffer, length + 1);
  Serial.println(msgBuffer);
  // Publish data to ThingSpeak. Replace <YOUR-CHANNEL-ID> with your channel ID and <YOUR-CHANNEL-WRITEAPIKEY> with your write API key
  if(mqttClient.publish(PublishCmd, msgBuffer)){
    // note the last connection time
    lastConnectionTime = millis();
  }
}

void mqttpublishtry() {
  if (!mqttClient.connected())
  {
    reconnect();
  }
  // Call the loop continuously to establish connection to the server
  mqttClient.loop();
  // If interval time has passed since the last connection, Publish data to ThingSpeak
  if (millis() - lastConnectionTime > postingInterval)
  {
    mqttpublish();
    Update_needed = false;
  }
    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("millis() - lastConnectionTime = ");
    Serial.println(millis() - lastConnectionTime );
    }
}

void  WifiConnexionManager() {
  if (SERIAL_PORT_LOG_ENABLE) {
  Serial.println("Begin of WifiConnexionManager");
  }
  int numberOfNetworks = WiFi.scanNetworks();

    if (SERIAL_PORT_LOG_ENABLE) {
      for (int i = 0; i < numberOfNetworks; i++) {
          Serial.print(WiFi.RSSI(i));
          Serial.print("dBm  ");
          Serial.print(" ");
          Serial.print("Network name: ");
          Serial.println(WiFi.SSID(i));
      }
    }
for (int i = 0; i < numberOfNetworks; i++) {    
    if (WiFi.SSID(i) == ssid1) {
      WiFi.begin(ssid1, password1);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid1);
      }
      break;
    }

    if (WiFi.SSID(i) == ssid2) {
      WiFi.begin(ssid2, password2);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid2);
      }
      break;
    }

    if (WiFi.SSID(i) == ssid0) {
      WiFi.begin(ssid0, password0);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid0);
        Serial.print("Wifi.status(): ");
        Serial.println(WiFi.status());
      }
      break;
    }
  }//End FOR
  Serial.println("End of WifiConnexionManager");
}//end WifiConnexionManager


void  WifiConnectOwner(char* SSIDowner_fct, char* passwordowner_fct) {
  if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Begin of WifiConnectOwner");
      Serial.println("Already connected to :");
      Serial.println(WiFi.SSID());
  }
  int numberOfNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numberOfNetworks; i++) {
    if ((WiFi.SSID(i) == SSIDowner_fct) && ((String)SSIDowner_fct != CurentSSID)) {
      WiFi.disconnect();
      WiFi.begin(SSIDowner_fct, passwordowner_fct);
      
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi Owner trouve . temtative de connexion a: ");
        Serial.println(SSIDowner_fct);
      }
      WaitConnexion();
      if ((WiFi.status() == WL_CONNECTED))
      {
        CurentSSID = WiFi.SSID(i);
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.print("connecte a: ");
          Serial.println(SSIDowner_fct);
        }
        
      }
      break;
    }
  }//End FOR
Serial.println("End of WifiConnectOwner");  
}//end WifiConnectOwner

void WaitConnexion(){
      int cpt=10;
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Begin of WaitConnexion");
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      while((cpt > 0) && ((WiFi.status() != WL_CONNECTED))){
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      cpt = cpt -1;
      delay(1000);
      }
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      if ((WiFi.status()== WL_CONNECTED))
      {
          if (SERIAL_PORT_LOG_ENABLE) {
             Serial.print("connecte a: ");
             Serial.println(WiFi.SSID());
            }
      } 
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("End of WaitConnexion");  
      }   
}// end Waitconnexion

int getSensorsOnBus() {
//return return sensors.getDeviceCount();
 return sensors.getDeviceCount();
}

void discoverOneWireDevices(byte Sensor_addr[8][8]) {
 int sensorNumber;
 byte i;
 byte present = 0;
 byte data[12];
 byte addr[8];
 //byte Sensor_addr[8][8];
 
 Serial.print("Looking for 1-Wire devices...\n\r");
 sensorNumber=0;
 while(sensors.search(addr)) {
   Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
   for( i = 0; i < 8; i++) {
     Serial.print("0x");
     if (addr[i] < 16) {
       Serial.print('0');
	   Sensor_addr(sensorNumber,i)='0';
     }
     Serial.print(addr[i], HEX);
	 Sensor_addr(sensorNumber,i) = HEX;
     if (i < 7) {
       Serial.print(", ");
     }
   }
   if ( OneWire::crc8( addr, 7) != addr[7]) {
       Serial.print("CRC is not valid!\n");
       return;
   }
//   copy_array()
   sensorNumber = sensorNumber + 1;
 }
 Serial.print("\n\r\n\rThat's it.\r\n");
 sensors.reset_search();
 return *Sensor_addr;
}

//void copy_array(){

//}

