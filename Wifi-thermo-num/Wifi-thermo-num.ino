#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Codes_Pouilly.h>// contient toutes les variables avec les loggins et les codes

//****************Configuration pour Debug****************
const boolean SERIAL_PORT_LOG_ENABLE = true; //true pour avoir la console active et false pour la desactiver ; il faut la desactiver pour l'application car meme pour que "verrou"
const boolean AvecMail = true; //true pour activer les mails
const boolean AvecWifi = true; // true pour activer le wifi

//Declaration des compteurs


String CurentSSID;
String CurentSSIDTry;
//Definition des chaines de characteres pour les mails
String MailContent = "no mail content defined\r\n";
String StringDate = "no date";
String StringTime = "no time";
String Release_Date = "12-02-2018";

//Definition des Inputs

//Definition des Outputs
int thermo_Power = 0; //ONEWIRE is plugged into pin 5 on GPIO_0 the Arduinol'alimentation des thermometre se fera sur GPIO2 c'est a dire D2 c'est a dire la pin 3 de la carte ESP8266-E01 ; c'est une output

#define ONE_WIRE_BUS 2 // l'alimentation des thermometre se fera sur GPIO2 c'est a dire D2 c'est a dire la pin 3 de la carte ESP8266-E01 ; c'est une output
#define TEMPERATURE_PRECISION 10
  
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
#define NumberOfDevices 3
byte totalDevices;
//Sensor Adresses 
byte allAddress [NumberOfDevices][8];//Permet d'explorer tous les sensors
DeviceAddress SensorBoard = { 0x28, 0xFF, 0x0F, 0xEA, 0x00, 0x17, 0x04, 0x96 };//Sensor soldered on board
DeviceAddress SensorExt; //external sensor
DeviceAddress SensorChauf; // Sensor circuit de chauffage
byte onBoardDeviceNumber=0;
byte chaufDeviceNumber=0;
byte extDeviceNumber=0;

byte Sensor_addr[8][8];

//Definition var for temperature monitoring
int ThempBoard = 150;
int ThempExt = 150;
int ThempChauf = 150;
int ThempMin = 5;// Temperature min a partir de laquelle 1 mail par jour est envoyé 


//Variables de gestion de l'etat de la porte et des mails
char server_email[] = "smtp.orange.fr";
const unsigned int localPort = 2390;


String MailToSend = "";
boolean mailToSend = true;
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
const unsigned long RegularpostingInterval = 10 * 60 * 1000L;// post data every 1O min
const unsigned long whatchDogValue = 60 * 60 * 1000L;// WhatchDog Value 60min
time_t epoch = 0; // contient
IPAddress ip;
IPAddress timeServerIP;

WiFiClient client;  // Initialize the Wifi client library.
WiFiUDP udp; //A UDP instance to let us send and receive packets over UDP
PubSubClient mqttClient(client); // Initialize the PuBSubClient library

//************************Debut de setup*************************
void setup() {
  delay(1000);
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.begin(115200);
    delay(1000);
    Serial.println("debut de setup");
  }

  //Set up PIN in INPUT


//Set up PIN in OUTPUT
  pinMode(thermo_Power, OUTPUT);// on definit comme output le port qui servira a monitorer la temperature
  digitalWrite(thermo_Power, HIGH);// on commence par couper l'alim des capteurs
 
  
//Debut de connexion intenet  
  while ((WiFi.status() != WL_CONNECTED) && AvecWifi) {
    WiFi.disconnect();
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

  totalDevices = discoverOneWireDevices();         // get addresses of our one wire devices into allAddress array 
  
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("totalDevices = ");
    Serial.println(totalDevices);
  }
  for (byte i=0; i < totalDevices; i++){ 
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("sensors.setResolution(");
    Serial.print(i);  
    Serial.print(",");
    Serial.println("12)");
  }
  sensors.setResolution(allAddress[i], 12);      // and set the ADC conversion resolution of each.
    }

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("System initialized");  
  }
  sensors.requestTemperatures();                // Initiate  temperature request to all devices

//Find the adress of the sensor on the board

onBoardDeviceNumber = findOnboardDevice(allAddress, SensorBoard);
chaufDeviceNumber = findChaufDevice(allAddress, SensorBoard, onBoardDeviceNumber);
sensors.requestTemperatures();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("Temperature of device n°0= ");
    Serial.println(sensors.getTempCByIndex(0));
    Serial.print("Temperature of device n°1= ");
    Serial.println(sensors.getTempCByIndex(1));
    Serial.print("Temperature of device n°2= ");
    Serial.println(sensors.getTempCByIndex(2));
  }
if(onBoardDeviceNumber==1){
  if(chaufDeviceNumber==2){
    extDeviceNumber=3;
    }
    else{
    extDeviceNumber=2;
    }  
  }
if(onBoardDeviceNumber==2){
  if(chaufDeviceNumber==1){
    extDeviceNumber=3;
    }
    else{
    extDeviceNumber=1;
    }  
  }
  if(onBoardDeviceNumber==3){
  if(chaufDeviceNumber==2){
    extDeviceNumber=1;
    }
    else{
    extDeviceNumber=2;
    }  
  }

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("on board device n = ");
    Serial.println(onBoardDeviceNumber);
    Serial.print("chaufDeviceNumber n = ");
    Serial.println(chaufDeviceNumber);
    Serial.print("extDeviceNumber n = ");
    Serial.println(extDeviceNumber);
  }

mqttpublishtry();

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
  Serial.print("Start of loop .... ");
  Serial.print("millis() - lastConnectionTime =  ");
  Serial.print(millis() - lastConnectionTime);
  Serial.print("           RegularpostingInterval = ")  ;
  Serial.println(RegularpostingInterval);
  }

delay(30000);

  if(AvecWifi){
  UpdateTime();
  rssi = WiFi.RSSI();
  if (Year < 2016) { // si l'heure n'est pas configuree C est a dire si l'annee n'est pas bonne, alors on recupere l'heure sur le serveur NTP
    GetTimeByUDP();
    UpdateTime();
  }

  if ((WiFi.status() != WL_CONNECTED) && AvecWifi) {// si la connexion WIFI est perdue, alors on relance le setup
    WiFi.disconnect();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Connexion lost ");
    }
    setup();
  }
 }

if (millis() - lastConnectionTime > RegularpostingInterval)
  {
    sensors.requestTemperatures();
    
    ThempBoard=sensors.getTempCByIndex(onBoardDeviceNumber);
    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("Temperature of onBoardDeviceNumber= ");
    Serial.print(ThempBoard);
    Serial.print("     onBoardDeviceNumber= ");
    Serial.println(onBoardDeviceNumber);
    }
    
    ThempExt=sensors.getTempCByIndex(extDeviceNumber);
    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("Temperature of extDeviceNumber= ");
    Serial.print(ThempExt);
    Serial.print("     extDeviceNumber= ");
    Serial.println(extDeviceNumber); 
    }

    ThempChauf=sensors.getTempCByIndex(chaufDeviceNumber);
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("Temperature of chaufDeviceNumber= ");
      Serial.print(ThempChauf);
      Serial.print("     chaufDeviceNumber= ");
      Serial.println(chaufDeviceNumber);
    }
    mqttpublishtry();  

    if ((ThempChauf > 15 && mailToSend ==true && AvecMail == true )) //Detecte l'ouverture de la porte
      {
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.println();
          Serial.println("ThempChauf > 15 && mailToSend = =true && AvecMail == true");
          Serial.println("Rearmement alerte mail");
        }
        mailToSend = true;
      }

    if ((ThempChauf < ThempMin && mailToSend ==true && AvecMail == true )) //Detecte l'ouverture de la porte
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println();
        Serial.print("Alerte temperature  : ");
        Serial.println("ThempChauf < 4");
      }
      MailContent = String("Alerte temperature : ");
      MailContent = String("Themperature du circuit de chauffage < 4 deges \r\n");
      UpdateTime();
  
      if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) {
        mailToSend = false;
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.println("Email sent");
        }
      }
      else
      {
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.println("Email failed");
        }
      }
    }// fin if ((ThempChauf ....
  
  }

if (millis() - lastConnectionTime > whatchDogValue) //whatch dog 
  {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Loop Whatch Dog activation -> relaunch Setup()");
    }
    setup();
  }



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
  client.println("Subject: Arduino Report Pouilly\r\n"); //sujet du mail
  //Corps du mail
  client.print(FcMailContent); // message specifique de l'application appelante

  
 // }

  client.print(StringDate);
  client.println(StringTime);

  client.println(FcThingspeakChannelAdress);

  client.print("http:///");
  client.println(WiFi.localIP());

  client.print("SW version: "); // Date de compilation
  client.print(Release_Date);
  client.println("   ;   Wifi-thermo-num thingspeaks - bis"); // chemin

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
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("Attempting MQTT connection..........................................");
    }
    // Connect to the MQTT broker
    if (mqttClient.connect(ThingspeakClientID, ThingspeakUserID, ThingspeakUserPwd))
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("connected");
      }
    } else
    {
      connect_counter = connect_counter - 1;
      if ((mqttClient.state() == -3) or (connect_counter < 0))
      {
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.println("-3 : MQTT_CONNECTION_LOST - the network connection was broken ... launch setup");
        }
        setup();
      }
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("failed, rc=");
      
      // Print to know why the connection failed
      // See http://pubsubclient.knolleary.net/api.html#state for the failure code and its reason
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      }
      // Wait 5 seconds before retrying to connect again
      delay(5000);
    }
  }
}
}
void mqttpublish() {
  String data = String("field1=");
  data = String("field1=" + String(ThempBoard, DEC) + "&field2=" + String(ThempExt, DEC) + "&field3=" + String(ThempChauf, DEC) + "&field4=" + String(rssi, DEC));

  // Get the data string length
  int length = data.length();
  char msgBuffer[length];
  char* PublishCmd = "channels/350196/publish/6EZ2KNK872STBCP9";
  // Convert data string to character buffer
  data.toCharArray(msgBuffer, length + 1);
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println(msgBuffer);
  }
  //Publish data to ThingSpeak. Replace <YOUR-CHANNEL-ID> with your channel ID and <YOUR-CHANNEL-WRITEAPIKEY> with your write API key
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
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("End of WifiConnexionManager");
  }
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
if (SERIAL_PORT_LOG_ENABLE) {
  Serial.println("End of WifiConnectOwner");  
}
}//end WifiConnectOwner

void WaitConnexion(){
      int cpt=30;
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

byte discoverOneWireDevices() {
  byte j=0;                                        // search for one wire devices and
                                                   // copy to device address arrays.
  while ((j < NumberOfDevices) && (oneWire.search(allAddress[j]))) {        
    j++;
  }
  for (byte i=0; i < j; i++) {
    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("Device ");
    Serial.print(i);  
    Serial.print(": ");                          
    printAddress(allAddress[i]);                  // print address from each device address arry.
    }
  }
  if (SERIAL_PORT_LOG_ENABLE) {
  Serial.print("\r\n");
  }
  return j                      ;                 // return total number of devices found.
}

void printAddress(DeviceAddress addr) {
  byte i;
  for( i=0; i < 8; i++) {                         // prefix the printout with 0x
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');                        // add a leading '0' if required.
      }
      Serial.print(addr[i], HEX);                 // print the actual value in HEX
      if (i < 7) {
        Serial.print(", ");
      }
    }
  Serial.print("\r\n");
}

void printTemperature(DeviceAddress addr) {
  float tempC = sensors.getTempC(addr);           // read the device at addr.
  if (tempC == -127.00) {
    Serial.println("Error getting temperature");
  } else {
    Serial.print("sensors.getTempC(addr)");
    Serial.print(sensors.getTempC(addr));                          // and print its value.
    Serial.println(" C (");
    Serial.print("tempC");
    Serial.print(tempC);                          // and print its value.
    Serial.println(" C (");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.println(" F)");
  }
}

float readTemperature(DeviceAddress addr) {
  float tempC = sensors.getTempC(addr);           // read the device at addr.
  if (tempC == -127.00) {
    Serial.print("Error getting temperature of device n°");
    
    printAddress(addr);
  } 
  return tempC;
}

byte findOnboardDevice(byte addr[NumberOfDevices][8], DeviceAddress SensorToCompare){
    byte device;
    int i;
    int compare=0;
  for( device=0; device < totalDevices; device++){
    compare=0;   
    Serial.println("searching for onBoard Device");
    for( i=0; i < 8; i++) {                         // prefix the printout with 0x
        Serial.print("0x");
        if (addr[device][i] < 16) {
          Serial.print('0');                        // add a leading '0' if required.
        }
        Serial.print(addr[device][i], HEX);                 // print the actual value in HEX
        if (addr[device][i] == SensorToCompare[i]){
          compare = compare +1;
          }
        if (i < 7) {
          Serial.print(", ");
        }
      }//end For
      Serial.print("\n\r compare = ");
      Serial.println(compare);    
      if (compare == 8){
        Serial.print("On board Device found with n° ");
        Serial.println(device); 
        return device;
        break;
      }
   }// end For
}
 
byte findChaufDevice(byte addr[NumberOfDevices][8], DeviceAddress SensorToCompare, byte excludedDevice){
    byte device;
    byte deviceToReturn;
    int i;
    int compare=0;
    float maxTemp=-125;
    float currentTemp=0;
    for( device=0; device < totalDevices; device++){
          Serial.print("totalDevices = ");
          Serial.println(totalDevices);
          Serial.print("  device = ");
          Serial.print(device);
      currentTemp = readTemperature(addr[device]);
      if(device==excludedDevice){

          Serial.print("  device n° ");
          Serial.print(device);
          Serial.print(" Temperature= ");
          Serial.print(currentTemp);
          Serial.println("   ****  Excluded device ");
          continue;
          }
      else{
          Serial.print("  device n° ");
          Serial.print(device);
          Serial.print(" Temperature= ");
          Serial.println(currentTemp);
          Serial.print(" maxTemp= ");
          Serial.println(maxTemp);
          if(maxTemp < currentTemp){
              Serial.print(" currentTemp= ");
              Serial.println(currentTemp);
              maxTemp = currentTemp;
              deviceToReturn = device;
              Serial.print(" deviceToReturn= ");
              Serial.println(deviceToReturn);
          }
      }//end else
   }// end For
   return deviceToReturn;
}






    
