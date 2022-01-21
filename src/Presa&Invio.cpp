//DataCATchup ver. 1.0.0   Created Run&Send code for CATchup project
//DataCATchup ver. 1.0.1   Delay controller condition and timestamp update added

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
int IndiceSeparatore;
int TimestampMicros;
int TimestampSeconds;
String Time;

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
  Serial.print("\n");
  client.get("/timestamp");
  //float StatusCode = client.responseStatusCode();
  response = client.responseBody();

  IndiceSeparatore = response.indexOf(".");
  TimestampMicros = response.substring(IndiceSeparatore+1).toInt();
  TimestampSeconds = response.substring(0, IndiceSeparatore).toInt();

  
  Serial.print("Timestamp del server: " + response);
  Serial.print("\n");
  Serial.print("Microseconds: " + String(TimestampMicros));
  Serial.print("\n");
  Serial.print("Seconds: " + String(TimestampSeconds));
  Serial.print("\n");
 
}

void loop() 
{
  int T = micros();
  if (!IMU.begin()) 
  {
    Serial.println("Failed to initialize IMU!");
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
  StringaDati  += String(A_x) += String("@") += String(A_y) += String("@") += String(A_z) += String("@") += String(g_x) += String("@") += String(g_y) += String("@") += String(g_z) += String("@") += String(Time) += String("/");
  DataMatrix += StringaDati;
  StringaDati = ""; 

  // Codice di controllo per verificare che si è arrivati a 2 secondi di presa dati  
  if(i==49)   // Invio dati al server
  {
    int t = millis();
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
    int TempoInvioServer = millis() -t;
    Serial.print("Tempo di invio al server in millisecondi" + String(TempoInvioServer));
  }
  else
  {
    i++;
  } 

  // Costruisco un delta tempo per tener conto del tempo di presa e conversione dati del sensore, dell'invio al server e dei vari passaggi intermedi
  int DeltaT = micros()-T;
  int DelayTime = Periodo - DeltaT;  

  if(DelayTime <= 0)
  {
    delayMicroseconds(-DelayTime);
  }
  else
  {
    delayMicroseconds(DelayTime);
  }
    

  // Aggiorno il Timestamp espresso in microsecondi
  int AddTimestamp = micros() - T;
  TimestampMicros += AddTimestamp;
  if(TimestampMicros >= 1000000)
  {
    TimestampSeconds ++;
  }
  TimestampMicros = TimestampMicros%1000000;
}