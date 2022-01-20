#include <Arduino_LSM6DS3.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
//#include "arduino_secrets.h"



// Dati WIFI
char ssid[] = "";
char pass[] = "";

const char serverName[] = "192.168.1.211";  // server name
int port = 5000;

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

// Inizializzo la stringa di dati composta dalle 3 componenti dell'accelerazione e dalle 3 componenti della velocità angolare
String StringaDati = "";

// Inizializzo la matrice di dati da passare al server
String DataMatrix ="\"";

// Inizializzo il contatore dei dati
int i = 0;

// Inizializzo le variabili per il timestamp
float Time;
String response;

void setup() 
{  
  IMU.begin();  

  Serial.begin(9600);
  while ( !Serial );
  while ( status != WL_CONNECTED )
  {
    Serial.print( "Attempting to connect to Network named: " );
    Serial.println( ssid );
    status = WiFi.begin( ssid, pass );
  }

  Serial.print( "SSID: " );
  Serial.println( WiFi.SSID() );

  IPAddress ip = WiFi.localIP();
  Serial.print( "IP Address: " );
  Serial.println( ip );

  /********************************/
  Serial.print("Ecco il timestamp di accensione Arduino");
  client.get("/timestamp");
  response = client.responseStatusCode();
  Time = response.toFloat(); 
  Serial.print(Time);
  
}

void loop() 
{
  if (!IMU.begin()) 
  {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  int t = micros();
  if(IMU.accelerationAvailable())
  {
    IMU.readAcceleration(A_x, A_y, A_z); 
  }
  if(IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(g_x, g_y, g_z); 
  }
  int deltaT = micros()-t;

  Time += 0.04;  
  
  StringaDati  += String(A_x) += String("@") += String(A_y) += String("@") += String(A_z) += String("@") += String(g_x) += String("@") += String(g_y) += String("@") += String(g_z) += String("@") += String(Time) += String("/");
  
  DataMatrix += StringaDati;
  
  
  
  
  if(i==49)
  {
    //DataMatrix = DataMatrix.substring(DataMatrix.length() - 1);
    DataMatrix.remove(DataMatrix.length() -1);
    DataMatrix += String("\"");
    
    String contentType = "application/json";
    String postData = "{\"data\" : "+DataMatrix+"}";
    client.post("/predict", contentType,  postData ); 
    DataMatrix = "\""; 
    
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    Serial.print( "Status code: " );
    Serial.println( statusCode );
    String response = client.responseBody();
    Serial.print( "Response: " );
    Serial.println( response );
    i=0;
  }
  else
  {
    i++;
  } 

  StringaDati = ""; 
  //postData = "";
  
  // Aspetta un tempo di 40 millisecondi, circa, per prendere un nuovo dato                            micros andrà sostituito (causa overflow) col timestamp (che servirà dunque ad impostare anche la frequenza corretta)
  delayMicroseconds(39800-deltaT) ;
  
  
}