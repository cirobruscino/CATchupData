#include <Arduino_LSM6DSOX.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
//#include "arduino_secrets.h"



// Dati WIFI
char ssid[] = "TIM-27310011";
char pass[] = "Zaccheroni4";
char Text[] = "Ciao a tutti";

const char serverName[] = "192.168.1.211";  // server name
int port = 5000;
unsigned int localPort = 2390;  
int BufferSize = 256;

WiFiClient wifi;
HttpClient client = HttpClient( wifi, serverName, port );
int status = WL_IDLE_STATUS;


// Inizializzo le variabili per il vettore accelerazione ed il vettore velocità angolare
float A_x;
float A_y; 
float A_z;
float g_x;
float g_y; 
float g_z;

// Fisso la frequenza di presa dati tramite il periodo, espresso in microsecondi
int Periodo = 40000;

// Inizializzo la stringa di dati composta dalle 3 componenti dell'accelerazione e dalle 3 componenti della velocità angolare
String StringaDati = "";

// Inizializzo il contatore dei dati
int i = 0;

// Inizializzo le variabili per il timestamp
//String response;
String response2;
int DeltaT = 0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup() 
{  
  Serial.begin(9600);
  IMU.begin();  
 
  while ( status != WL_CONNECTED )
  {
    status = WiFi.begin( ssid, pass );
  }
  IPAddress ip = WiFi.localIP();

  //Faccio partire lo streaming del server in ricezione
  client.get("/streaming");
  response2 = client.responseBody();

  Udp.begin(localPort);
  
}

void loop() 
{
  int T = micros();
   
  if (!IMU.begin()) 
  {
    //Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  if(IMU.accelerationAvailable())
  {
    IMU.readAcceleration(A_x, A_y, A_z); 
  }
  
  if(IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(g_x, g_y, g_z); 
  }

  
  // Formattazione dati da inviare al server
  StringaDati = String(A_x) += String("@") += String(A_y) += String("@") += String(A_z) += String("@") += String(g_x) += String("@") += String(g_y) += String("@") += String(g_z) += String("!");
  
  char CharDati[] = "";
  StringaDati.toCharArray(CharDati, BufferSize); 
  Udp.beginPacket(serverName, 65013);
  Udp.write(CharDati);
  Udp.endPacket();
  StringaDati = ""; 

  int InternalDelay = micros() - T;
  int DelayTime = Periodo-InternalDelay;
  delayMicroseconds(DelayTime-276-42-45); 
  
  
  DeltaT = micros() - T;  
  Serial.println("Delta time in micros: " + String(DeltaT));

}  