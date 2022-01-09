#include <Arduino.h>
#include <Arduino_LSM6DS3.h>

float A_x = 0;
float A_y = 0; 
float A_z = 0;

float g_x = 0;
float g_y = 0; 
float g_z = 0;

int i = 0;
String Data;
String DataMatrix[1][50];

void setup() 
{
    Serial.begin(9600);
    IMU.begin();  
}

void loop() 
{
  

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

    Data += String(A_x) += String(", ") += String(A_y) += String(", ") += String(A_z) += String(", ") += String(g_x) += String(", ") += String(g_y) += String(", ") += String(g_z); 

    
    DataMatrix[1][i] = Data;
    
    for(int j=0;j<50;j++)
    {
    Serial.println("Dato " + String(j+1) + ": " + DataMatrix[1][j]);
    }
    Serial.println("Memoria Dati: " + String(sizeof(DataMatrix)));

    Data = "";

    if(i<50)
    {
        i= i+1;
    }
    else
    {
        i=0;
    }


    delay(1000);

}