//DataCATchup ver. 1.0.0   Created Run&Send code for CATchup project
//DataCATchup ver. 1.0.1   Delay controller condition and timestamp update added
//DataCATchup ver. 1.1.0   Added UDP protocol for a quickly data sending

#include <Arduino_LSM6DS3.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
//#include "arduino_secrets.h"



// Dati WIFI
char ssid[] = "";
char pass[] = "";

const char serverName[] = "192.168.1.211";  // server name
int port = 5000;
unsigned int localPort = 2390;  
int BufferSize = 128;

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

// Inizializzo la matrice di dati da passare al server
String DataMatrix ="\"";

// Inizializzo il contatore dei dati
int i = 0;

// Inizializzo le variabili per il timestamp
String response;
String response2;
int IndiceSeparatore;
int TimestampMicros;
int TimestampSeconds;
String Time;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup() 
{  
  
  IMU.begin();  
 
  while ( status != WL_CONNECTED )
  {
    status = WiFi.begin( ssid, pass );
  }

  IPAddress ip = WiFi.localIP();

  /********************************/
  
  client.get("/timestamp");
  //float StatusCode = client.responseStatusCode();
  response = client.responseBody();

  //Faccio partire lo streaming del server in ricezione
  client.get("/streaming");
  //float StatusCode = client.responseStatusCode();
  response2 = client.responseBody();

  IndiceSeparatore = response.indexOf(".");
  StrTimestampMicros = response.substring(IndiceSeparatore+1);
  TimestampMicros = response.substring(IndiceSeparatore+1).toInt();
  TimestampSeconds = response.substring(0, IndiceSeparatore).toInt();

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

   
  //Formattazione stringa timestamp
  Time = String(TimestampSeconds) + String(TimestampMicros);
  
  // Formattazione dati da inviare al server
  StringaDati  += String(A_x) += String("@") += String(A_y) += String("@") += String(A_z) += String("@") += String(g_x) += String("@") += String(g_y) += String("@") += String(g_z) += String("@") += String(Time);
  
  char CharDati [] = "";
  StringaDati.toCharArray(CharDati, BufferSize); 
  Udp.beginPacket(serverName, 65013);
  Udp.write(CharDati);
  Udp.endPacket();
  delayMicroseconds(32000);
    
  StringaDati = ""; 
  
  // Aggiorno il Timestamp espresso in microsecondi
  int AddTimestamp = micros() - T;
  TimestampMicros += AddTimestamp;
  if(TimestampMicros >= 1000000)
  {
    TimestampSeconds ++;
  }
  TimestampMicros = TimestampMicros%1000000;
  
  int DeltaT = micros() - T;  
  //Serial.println("Delta time in micros: " + String(DeltaT));

}  