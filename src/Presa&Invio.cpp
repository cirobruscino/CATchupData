//DataCATchup ver. 1.0.0   Created Run&Send code for CATchup project
//DataCATchup ver. 1.0.1   Delay controller condition and timestamp update added
//DataCATchup ver. 1.1.0   Added UDP protocol for a quickly data sending
//DataCATchup ver. 1.1.1   Resolved UDP sending bug using a Serial.println at the bottom of the code

#include <Arduino_LSM6DS3.h>
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
  Serial.begin(9600);
  IMU.begin();  
 
  while ( status != WL_CONNECTED )
  {
    /*Serial.print( "Attempting to connect to Network named: " );
    Serial.println( ssid );*/
    status = WiFi.begin( ssid, pass );
  }

  /*Serial.print( "SSID: " );
  Serial.println( WiFi.SSID() );*/

  IPAddress ip = WiFi.localIP();
  /*Serial.print( "IP Address: " );
  Serial.println( ip );*/

  /********************************/
  
  /*Serial.print("Ecco il timestamp di accensione Arduino");
  Serial.print("\n");*/
  client.get("/timestamp");
  //float StatusCode = client.responseStatusCode();
  response = client.responseBody();

  //Faccio partire lo streaming del server in ricezione
  client.get("/streaming");
  //float StatusCode = client.responseStatusCode();
  response2 = client.responseBody();

  //Serial.println(response2);

  IndiceSeparatore = response.indexOf(".");
  TimestampMicros = response.substring(IndiceSeparatore+1).toInt();
  TimestampSeconds = response.substring(0, IndiceSeparatore).toInt();

  
  /*Serial.print("Timestamp del server: " + response);
  Serial.print("\n");
  Serial.print("Microseconds: " + String(TimestampMicros));
  Serial.print("\n");
  Serial.print("Seconds: " + String(TimestampSeconds));
  Serial.print("\n");
  
  Serial.println("\nStarting connection to server...");*/
  Udp.begin(localPort);
  
}

void loop() 
{
  /*digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);*/  
 
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

  //Corretta formattazione della parte in microsecondi del timestamp:
  /*StrTimestampMicros = String(TimestampMicros);
  if(StrTimestampMicros.length() == 1)
  {
    StrTimestampMicros = "00000" + String(TimestampMicros);
  }
  else if(StrTimestampMicros.length() == 2)
  {
    StrTimestampMicros = "0000" + String(TimestampMicros); 
  }
  else if(StrTimestampMicros.length() == 3)
  {
    StrTimestampMicros = "000" + String(TimestampMicros); 
  }
  else if(StrTimestampMicros.length() == 4)
  {
    StrTimestampMicros = "00" + String(TimestampMicros); 
  }
  else if(StrTimestampMicros.length() == 5)
  {
    StrTimestampMicros = "0" + String(TimestampMicros); 
  }
  else
  {
    StrTimestampMicros = String(TimestampMicros);
  }*/
  
  //Formattazione stringa timestamp
  Time = String(TimestampSeconds) + String(TimestampMicros);
  
  // Formattazione dati da inviare al server
  StringaDati = String(A_x) += String("@") += String(A_y) += String("@") += String(A_z) += String("@") += String(g_x) += String("@") += String(g_y) += String("@") += String(g_z) += String("@") += String(Time);
  //StringaDati = "Ciao@Ciao@Ciao@Ciao@Ciao@Ciao";
  
  char CharDati[] = "";
  StringaDati.toCharArray(CharDati, BufferSize); 
  Udp.beginPacket(serverName, 65013);
  Udp.write(CharDati);
  Udp.endPacket();
  delayMicroseconds(32000);  
  
  //CharDati = "";
  //free(StringaDati);
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
  Serial.println("Delta time in micros: " + String(DeltaT));

}  