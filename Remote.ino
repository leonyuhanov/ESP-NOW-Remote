
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}
#define WIFI_CHANNEL 1

const byte numOfPins = 4, numberOfInputs=16;
const byte multiplexerAddressPins[numOfPins] = {D1, D2, D3, D4}, inputPin = D0;
byte addressArray[numberOfInputs][numOfPins] = {{0,0,0,0},{0,0,0,1},{0,0,1,0},{0,0,1,1},{0,1,0,0},{0,1,0,1},{0,1,1,0},{0,1,1,1},{1,0,0,0},{1,0,0,1},{1,0,1,0},{1,0,1,1},{1,1,0,0},{1,1,0,1},{1,1,1,0},{1,1,1,1}};
byte buttonArray[numberOfInputs] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, testData[7];
unsigned short int cnt=0, inCounter=0, inputResult=0;
unsigned long timers[3], defaultIOTimeout = 250;

//network stuff
byte broadCastAddress[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
//byte broadCastAddress[] = {0x5C, 0xCF, 0x7F, 0x17, 0xA4, 0xED};

void setup() 
{
  //Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  esp_now_init();
  delay(10);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(broadCastAddress, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  
  //set up Multiplexer Address Pins
  for(cnt=0; cnt<numOfPins; cnt++)
  {
    pinMode(multiplexerAddressPins[cnt], OUTPUT);
    digitalWrite(multiplexerAddressPins[cnt], LOW);
  }
}

//Input Breaker Timers
void ioTimerStart(unsigned long durationInMillis)
{
  timers[0] = millis(); 
  timers[2] = durationInMillis;
}

byte ioTimeOut()
{
  timers[1] = millis();
  if(timers[2] < timers[1]-timers[0])
  {
    return 1;
  }
  return 0;
}

void loop()
{
  if( readButtons() == 1)
  {
    //Test for diferent Data Length
    if(buttonArray[numberOfInputs-1]==1)
    {
      esp_now_send(broadCastAddress, testData, 7);
    }
    else
    {
      esp_now_send(broadCastAddress, buttonArray, numberOfInputs);
    }
  }
  yield();  
}

byte readButtons()
{
  //If a timer has been started do nothing
  if( !ioTimeOut() ) { return 0; }
  //Else read buttons into input array
  inputResult=0;
  for(inCounter=0; inCounter<numberOfInputs; inCounter++)
  {
    setMultiplexerAddress(inCounter);
    buttonArray[inCounter] = digitalRead(inputPin);
    delay(5);
    if(buttonArray[inCounter]==1){inputResult++;}
  }
  if(inputResult)
  {
    ioTimerStart(defaultIOTimeout);
    return 1;
  }
  return 2;
}

void setMultiplexerAddress(byte multiplexerAddress)
{
  for(cnt=0; cnt<numOfPins; cnt++)
  {
    digitalWrite(multiplexerAddressPins[cnt], addressArray[multiplexerAddress][cnt]);
  }

}

