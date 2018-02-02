#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>

//****************Configuration pour Debug****************
const boolean SERIAL_PORT_LOG_ENABLE = true; //true pour avoir la console active et false pour la desactiver ; il faut la desactiver pour l'application car meme pour que "verrou"
const boolean AvecMail = true;//true pour activer les mails
const boolean AvecLED = false;//true pour avoir les les de debug (attention - meme sortie que la comande de l'interphone

//Declaration des compteurs


//Connexion a Internet

const char* ssid0 = "christophegueble";     //type your ssid
const char* password0 = "vacances";        //type your password
//const char* ssid1 = "GUEBLE";                           //******************** CGU a MODIFER avec SSID Pouilly et voisins
//const char* password1 = "01456789";                     //******************** CGU a MODIFER avec SSID Pouilly et voisins

const char* ssid1 = "Livebox-D108";                     //******************** CGU a MODIFER avec SSID Pouilly et voisins
const char* password1 = "29E155E7DDDFC5D299A199F64D";   //******************** CGU a MODIFER avec SSID Pouilly et voisins

const char* ssid2 = "Livebox-F41F";                     //******************** CGU a MODIFER avec MDP voisins puilly
const char* password2 = "48C799C81C55E26E6F58E1F395";   //******************** CGU a MODIFER avec MDP voisins Pouilly
String MailContent = "no mail content defined\r\n";
String StringDate = "no date";
String StringTime = "no time";

//Sensor Adresses 
//DeviceAddress Sensor1 = { 0x28, 0xFF, 0xCE, 0xA1, 0x00, 0x17, 0x03, 0x8E };
//DeviceAddress Sensor2 = { 0x28, 0xFF, 0xE2, 0x0F, 0x61, 0x16, 0x03, 0x21 };
DeviceAddress Sensor1 = { 0x28, 0xFF, 0xCE, 0xA1, 0x00, 0x17, 0x03, 0x8E };
DeviceAddress Sensor2 = { 0x28, 0xFF, 0x4D, 0x71, 0xC0, 0x16, 0x04, 0x1E };

//Device 0 Address: 28FFCEA10017038E
//Device 1 Address: 28FFE20F61160321

//Definition des Outputs
int thermo_Power = 2; //l'alimentation des thermometre se fera sur GPIO2 c'est a dire D2 c'est a dire la pin 3 de la carte ESP8266-E01 ; c'est une output
/********************************************************************/
// ONEWIRE is plugged into pin 5 on GPIO_0 the Arduino 
#define ONE_WIRE_BUS 0
#define TEMPERATURE_PRECISION 9
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 

//Definition des constantes Globales
const unsigned long seventyYears = 2208988800UL;

//Declaration des variables globales
boolean Status_IlsPorte = false;
boolean Status_Verrou = false;


boolean MailSent = true;

boolean Update_needed = false;
long rssi; //pour mesure RSSI
unsigned long secsSince1900 = 0;
time_t epoch = 0; // contient

int Hour;
int Minute;
int Second;
int Day;
int DayofWeek; // Sunday is day 0 
int Month;     // Jan is month 0
int Year;      // the Year minus 1900
int WeekDay;

int ThempInt = 90;
int ThempChauf = 90;
int ThempMin = 5;// Temperature min a partir de laquelle 1 mail par jour est envoyé

String MailToSend = "";
IPAddress ip;
char server_email[] = "smtp.orange.fr";
unsigned int localPort = 2390;
IPAddress timeServerIP;
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

WiFiClient client;
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
PubSubClient mqttClient(client);
// Define the ThingSpeak MQTT broker
const char* server = "mqtt.thingspeak.com";

// track the last connection time
unsigned long lastConnectionTime = 0; 
// post data every 60 seconds
const unsigned long postingInterval = 60L * 1000L;

// track the last connection time
unsigned long lastMailTime = 24L * 60L * 61L * 1000L; 
// post data every 24h en miliseconds
const unsigned long MailInterval = 24L * 60L * 60L * 1000L;


//************************Debut de setup*************************
void setup() {
    if(SERIAL_PORT_LOG_ENABLE){
  Serial.begin(19200);
  delay(10);
  Serial.println("Begining of Setup");
  Serial.println();
  Serial.println();
  }

   if(SERIAL_PORT_LOG_ENABLE){
   Serial.print("Attente 10s avant de débuter setup ");
   }
  delay(10000);//Attente 10s avant de débuter setup
  if(SERIAL_PORT_LOG_ENABLE){
   Serial.print("fin des 10s avant de débuter setup ");
   }
  int CptConnexion = 0;
  const int CptConnexionMax = 2;

//Set up PIN in OUTPUT
  pinMode(thermo_Power, OUTPUT);
  digitalWrite(thermo_Power, LOW);

Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

// Must be called before search()
oneWire.reset_search();
  // assigns the first address found to insideThermometer
if (!oneWire.search(Sensor1)) Serial.println("Unable to find address for insideThermometer");
  // assigns the seconds address found to outsideThermometer
if (!oneWire.search(Sensor2)) Serial.println("Unable to find address for outsideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(Sensor1);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(Sensor2);
  Serial.println();

  // set the resolution to 9 bit per device
  sensors.setResolution(Sensor1, TEMPERATURE_PRECISION);
  sensors.setResolution(Sensor2, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(Sensor1), DEC); 
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(Sensor2), DEC); 
  Serial.println();



    //WiFi.softAPdisconnect(true);
    //WiFi.disconnect(true);
    //ESP.eraseConfig();
    //ESP.reset();

do{
// Connect to WiFi network 
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid1, password1);
     while ((CptConnexion <= CptConnexionMax) && (WiFi.status() != WL_CONNECTED)) {
      CptConnexion = CptConnexion +1;
      TrigLED();
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.print("Connecting to ");
      Serial.print(ssid1);
      Serial.print("    connexion retry number ");
      Serial.println(CptConnexion);
      }
      delay(10000);
    }
    CptConnexion = 0;
}

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid2, password2);
     while ((CptConnexion <= CptConnexionMax) && (WiFi.status() != WL_CONNECTED)) {
      CptConnexion = CptConnexion +1;
      TrigLED();
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.print("Connecting to ");
      Serial.print(ssid2);
      Serial.print("    connexion retry number ");
      Serial.println(CptConnexion);
      }
      delay(10000);
    }
    CptConnexion = 0;
}

    if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid0, password0);
     while ((CptConnexion <= CptConnexionMax) && (WiFi.status() != WL_CONNECTED)) {
      CptConnexion = CptConnexion +1;
      TrigLED();
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.print("Connecting to ");
      Serial.print(ssid0);
      Serial.print("    connexion retry n ");
      Serial.println(CptConnexion);
      }
      delay(10000);
    }
    CptConnexion = 0;
  }
} while ((WiFi.status() != WL_CONNECTED)/*||(CptConnexion < (3 * CptConnexionMax))*/);//Loop connexion until Wifi Connetion is OK or test connexion > 3* CptConnexionMax
// si la connexion n'est pas possible on part monitorer les temperatures

if(WiFi.status() == WL_CONNECTED){// Debut de traitement si wifi connecté

    if(SERIAL_PORT_LOG_ENABLE){
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("Use this URL to connect: ");
      Serial.print("http://");
      Serial.print(WiFi.localIP());
      Serial.println("/");
    }
    
    udp.begin(localPort);// Start UDP port for connexion to NTP server
    
    if(SERIAL_PORT_LOG_ENABLE){
      Serial.println("Starting UDP");
      Serial.print("Local port: ");
      Serial.println(udp.localPort());
    }
     
    if(SERIAL_PORT_LOG_ENABLE){
    Serial.println("End of Setup");
    }
    
    GetTimeByUDP();
    UpdateTime();
    mqttClient.setServer(server, 1883);// Set the MQTT broker details
    
}//fin de traitement si wifi est connecté

}// fin set up
//******************end of setup*************************************
//*******************************************************************
//******************start of loop************************************ 
void loop() {
if(SERIAL_PORT_LOG_ENABLE){
    Serial.println(".");
    }
UpdateTime();
ThempInt = 90;
ThempChauf = 90;
ThempInt = sensors.getTempCByIndex(0);
ThempChauf = sensors.getTempCByIndex(1);
digitalWrite(thermo_Power, HIGH); //Power supply the Thermo
Serial.print("Temperature is of ThermoInt: "); 
//Serial.println(sensors.getTempCByIndex(0));
Serial.println(ThempInt);
Serial.print("Temperature is of ThermoChauf: "); 
//Serial.println(sensors.getTempCByIndex(1));
Serial.println(ThempChauf);
    // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 
   delay(1000); 
 



//Alerte mail
Serial.println("Test Alerte mail");
Serial.print("ThempChauf= ");
Serial.println(ThempChauf);
Serial.print("ThempMin=  ");
Serial.println(ThempMin);

if(ThempChauf < ThempMin){ // mail a envoyer si la temperature  passe sous la limite basse
    Serial.println("Mail to send because         ThempChauf < ThempMin ");
    Serial.print("lastMailTime= ");
    Serial.println(lastMailTime);
    Serial.print("millis()= ");
    Serial.println(millis());    
    if (millis() - lastMailTime > MailInterval) 
  {
    Serial.println("Mail to send because         millis() - lastMailTime > MailInterval ");
    MailSent = false;
  }
    
}
else
{
    mqttpublishtry();// test de la connexion
}

// call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.println("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  // print the device information
  printData(Sensor1);
  printData(Sensor2);




WifiConnexionTest();// Relance le setup si le WIFI est déconnecté ******* a faire a chaque fois
rssi = WiFi.RSSI();
if(Year < 2016){// si l'heure n'est pas configurée CàD si l'année n'est pas bonne, alors on recupere l'heure sur le serveur NTP
  GetTimeByUDP();
  UpdateTime();
  }

SendAlerteMail();

}//*****************END OF LOOP *******************


// function to print a device address
void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress){
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();    
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress){
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}


void SendAlerteMail(){
  if((MailSent == false)) // si il y a un mail a envoyer alors on traite la demande ici
{
    mqttpublishtry();// test de la connexion
    GetTimeByUDP();// mise a jour de l'heure
    
    if(SERIAL_PORT_LOG_ENABLE){
    Serial.println();
    Serial.println("debut mail");
    }
    MailContent=String("Pouilly alerte temperature \r\n");
    if(sendEmail(MailContent)) {  // Mail sent success
        MailSent = true;
        lastMailTime = millis();
        if(SERIAL_PORT_LOG_ENABLE){
        Serial.println(F("Email sent"));
        //Serial.println("ESP.deepSleep(60*1000000, WAKE_NO_RFCAL);");
        }
        
        //forceSleepBegin
        //ESP.deepSleep(24*3600*1000000, WAKE_NO_RFCAL);  // Sleep pour 1 jour
        //ESP.deepSleep(2*60*1000000, WAKE_NO_RFCAL); // Sleep pour 2 minutes
        //Waikup on GPIO16 reset.... witch Pin ???
    }
    else // Mail sent failure
    {
        if(SERIAL_PORT_LOG_ENABLE){
        Serial.println(F("Email not sent - System frozen for 3 minutes"));
        }
        delay(3000); // on bloque l'envoie du mail pour 3 minutes avant prochain envoie
    }
} 
}


void WifiConnexionTest(){
  if (WiFi.status() != WL_CONNECTED) {// si la connexion WIFI est perdue, alors on relance le setup
    WiFi.disconnect();
    if(SERIAL_PORT_LOG_ENABLE){
      Serial.println("Connexion lost ");
      }
    delay(2000);
    setup();
    }
}

void UpdateTime(){
      StringDate="";// reset StringDate value
      StringTime="";// reset StringTime value
      Hour = hour();
      Minute = minute();
      Second = second();
      Year = year();
      Month = month();
      Day = day();
      WeekDay = weekday();
      
      if((Month <= 3) || (Month > 10)){// on est un mois d'hiver
                    if((Month == 3) && (Day-WeekDay > 24)){//on dans le dernier mois d'hiver et après le dernier dimanche    
                          Hour = Hour +2;
                          StringTime += " UTC+2: ";
                    }
                    else{// toute la periode hivernale
                         Hour = Hour +1;
                         StringTime += " UTC+1: ";
                    }
      }
          else{// on est un mois d'été
                    if((Month == 10) && (Day-WeekDay > 24)){ //on dans le dernier mois d'été et après le dernier dimanche     
                          Hour = Hour +1;
                          StringTime += " UTC+1: ";            
                    }
                    else{// toute la periode estivale
                          Hour = Hour +2;
                          StringTime += " UTC+2: ";
                    }
          } 
      

      //StringDate += "Date: ";
          if(Day <10){
            StringDate += "0";
          }
      StringDate += Day;
          if(Month <10){
            StringDate += "/0";
          }
          else{
            StringDate += "/";
          }
      StringDate += Month;
      StringDate += "/";
      StringDate +=Year;
            
          if(Hour <10){
            StringTime += "0";
          }
      StringTime += Hour;
          if(Minute <10){
            StringTime += ":0";
          }
          else{
            StringTime += ":";
          }
      StringTime += Minute;
          if(Second <10){
            StringTime += ":0";
          }
          else{
            StringTime +=":";
          }
      StringTime += Second;
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.print("valeure de StringDate ");
      Serial.println(StringDate);
      Serial.print("valeure de StringTime ");
      Serial.println(StringTime);
      }
}

void TrigLED(){ // pour debug uniquement
  if (AvecLED){
  delay(100);
  //digitalWrite(LED, HIGH); //ATTENTION - pour test uniquement
  delay(500);
  //digitalWrite(LED, LOW); //Activation de l'interphone quand InterphonePorte=HIGH 
  }
}


void GetTimeByUDP(){
//get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 
 
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);


  int cb = udp.parsePacket();

  if (!cb) {
    if(SERIAL_PORT_LOG_ENABLE){
    Serial.println("no packet yet");
    }
  }
  else {
    if(SERIAL_PORT_LOG_ENABLE){
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
     if(SERIAL_PORT_LOG_ENABLE){
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
unsigned long sendNTPpacket(IPAddress& address)
{
   if(SERIAL_PORT_LOG_ENABLE){
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


byte sendEmail(String FcMailContent)
{


  byte thisByte = 0;
  byte respCode;
  
  if(!AvecMail) return 0;// sort de la procedure sans envoyer le mail si Avec mail=0
  
  if(client.connect(server_email,25)) {
    if(SERIAL_PORT_LOG_ENABLE){
    Serial.println(F("connected"));
    }
  } else {
    if(SERIAL_PORT_LOG_ENABLE){
    Serial.println(F("connection failed"));
    }
    return 0;
  }

  if(!eRcv()) return 0;

// change to your public ip
  client.println("helo 10.62.66.144");

  if(!eRcv()) return 0;
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println(F("Sending From"));
  }
  
// change to your email address (sender)
  client.println("MAIL From: <christophe.gueble@gmail.com>");

  if(!eRcv()) return 0;

// change to recipient address
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println("Sending To");
  }
  client.println("RCPT To: <christophe.gueble@free.fr>");
  if(!eRcv()) return 0;

  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println("Sending DATA");
  }
  
  client.println("DATA");
  if(!eRcv()) return 0;
  


//*****************Creation du Mail***********************
  client.println("To: You <christophe.gueble@free.fr>"); // Destinataire
  client.println("From: Me <christophe.gueble@free.fr>"); // My address
  client.println("Subject: Pouilly - Alerte temperature\r\n"); //sujet du mail
//Corps du mail
  client.print(FcMailContent); // message specifique de l'application appelante 

  client.println("La temperature est trop basse.\r\n"); //******************** CGU a MODIFER
  
  client.print(StringDate);
  client.println(StringTime);

  client.println("https://thingspeak.com/channels/350196\r\n"); //******************** CGU a MODIFER
 //fin du mail 
  client.println("."); 
  if(!eRcv()) return 0; //test si mail correctement envoyé
 
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println("Sending email");
  }
  
  client.println("QUIT"); //Deconnection du server mail
  if(!eRcv()) return 0;
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println("Sending QUIT");
  }
  client.stop();
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.println("disconnected");
  }
  return 1;
  UpdateTime();
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while(!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.println("\r\nTimeout");
      }
      return 0;
    }
  }

  respCode = client.peek();

  while(client.available())
  {  
    thisByte = client.read(); 
    if(SERIAL_PORT_LOG_ENABLE){   
    Serial.write(thisByte);
    }
  }

  if(respCode >= '4')
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

  client.println(F("QUIT"));

  while(!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      if(SERIAL_PORT_LOG_ENABLE){
      Serial.println("\r\nTimeout");
      }
      return;
    }
  }

  while(client.available())
  {  
    thisByte = client.read(); 
    if(SERIAL_PORT_LOG_ENABLE){   
    Serial.write(thisByte);
    }
  }
  client.stop();
   if(SERIAL_PORT_LOG_ENABLE){
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
    if (mqttClient.connect("ArduinoWiFi101Client")) 
    {
      Serial.println("connected");
    } else 
    {
      connect_counter = connect_counter -1;
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
  // Create data string to send to ThingSpeak
  String data = String("field1=" + String(ThempInt, DEC) + "&field2=" + String(ThempChauf, DEC)+ "&field3=" + String(rssi, DEC));
  // Get the data string length
  int length = data.length();
  char msgBuffer[length];
  // Convert data string to character buffer
  data.toCharArray(msgBuffer,length+1);
  Serial.println(msgBuffer);
  // Publish data to ThingSpeak. Replace <YOUR-CHANNEL-ID> with your channel ID and <YOUR-CHANNEL-WRITEAPIKEY> with your write API key
  mqttClient.publish("channels/350196/publish/6EZ2KNK872STBCP9",msgBuffer);
  // note the last connection time
  lastConnectionTime = millis();
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
}
