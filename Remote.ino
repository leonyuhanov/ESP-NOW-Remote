/*
   MAC Addresses of Devices you want to send ESPNOW data TO
   -------------
   Device 1:		AA:BB:CC:DD:EE:00
   Device 2:		AA:BB:CC:DD:EE:01

*/
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}
#define WIFI_CHANNEL 1

const byte numOfPins = 2, numberOfInputs=4, numOfButtons=16;
const byte rowMultiplexer[numOfPins] = {D1, D2};
const byte colMultiplexer[numOfPins] = {D4, D3};
const byte inputPin = D0;
const byte addressArray[numberOfInputs][numOfPins] = {{0,0},{0,1},{1,0},{1,1}};

byte buttonArray[numOfButtons] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
byte previousButtonArray[numOfButtons] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned short int cnt=0, inCounter=0, innerCounter=0, inputCounter=0, inputResult=0, txResult=0;
unsigned long timers[2];


//network stuff
const byte macLength=6, numberOfDevices = 2;
byte remoteDevice[numberOfDevices][macLength] = {{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01}, {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02}};

void setup() 
{
  Serial.begin(115200);
  Serial.println("\r\n\r\n\r\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  esp_now_init();
  delay(10);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  for(cnt=0; cnt<numberOfDevices; cnt++)
  {
    esp_now_add_peer(remoteDevice[cnt], ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  }
  //set up Multiplexer Address Pins
  for(cnt=0; cnt<numOfPins; cnt++)
  {
    pinMode(rowMultiplexer[cnt], OUTPUT);
    pinMode(colMultiplexer[cnt], OUTPUT);
    digitalWrite(rowMultiplexer[cnt], LOW);
    digitalWrite(colMultiplexer[cnt], LOW);
  }
  //Set up Input Pin
  pinMode(inputPin, INPUT);

}

void loop()
{
  if( readButtons() == 1)
  {
    //Button pushed start timer
    timers[0] = micros();
    while( readButtons() == 1)
    {
      //Wait untill buttnos have been released      
      memcpy( &previousButtonArray, &buttonArray, numOfButtons );
      yield();
    }
    //Button Released stop timer
    timers[1] = micros();
    if( timers[1]-timers[0] < 10000 )
    {
       Serial.printf("Bounce - NOTHING SENT....\r\n");
    }
    else
    {
      for(cnt=0; cnt<numberOfDevices; cnt++)
      {
         txResult = esp_now_send(remoteDevice[cnt], previousButtonArray, numOfButtons);
         Serial.printf("\tD[%d]\t%d", cnt, txResult);
      }
      Serial.printf("\r\nSENT\tClick took %d ms\t%d micros\t\tStart\t%d\tEnd\t%d\r\n", (timers[1]-timers[2])/1000, timers[1]-timers[2], timers[2], timers[1]);
      
      for(cnt=0; cnt<numOfButtons; cnt++)
      {
        Serial.printf("%d\t", previousButtonArray[cnt]);
      }
      
      Serial.printf("\r\n");
    }
  }
  yield();  
 
}

byte readButtons()
{
  inputResult=0;
  inputCounter=0;
  
  for(inCounter=0; inCounter<numberOfInputs; inCounter++)
  {
      //Select Row
      setRowAddress(inCounter);
      for(innerCounter=0; innerCounter<numberOfInputs; innerCounter++)
      {
        //Select Column and Read
        setColAddress(innerCounter);
        buttonArray[inputCounter] = digitalRead(inputPin);
        if(buttonArray[inputCounter]==1){inputResult++;}
        inputCounter++;
      }
  }
  if(inputResult)
  {
    return 1;
  }
  return 2;
}

void setRowAddress(byte multiplexerAddress)
{
  for(cnt=0; cnt<numOfPins; cnt++)
  {
    digitalWrite(rowMultiplexer[cnt], addressArray[multiplexerAddress][cnt]);
  }

}

void setColAddress(byte multiplexerAddress)
{
  for(cnt=0; cnt<numOfPins; cnt++)
  {
    digitalWrite(colMultiplexer[cnt], addressArray[multiplexerAddress][cnt]);
  }

}
