#include <RFM69.h>
#include <RFM69_ATC.h>

#define NODEID      2   //Debe ser unico para cada nodo en la misma red (hasta 254 nodos)
#define NETWORKID   100 //El mismo para todos los nodos que se comunican entre si
#define GATEWAYID   1
#define FREQUENCY   RF69_915MHZ
#define ENCRYPTKEY  "sampleEncryptKey"  //exactly the same 16 characters/bytes on all nodes
#define IS_RFM69HW_HCW

#define ENABLE_ATC
#define ATC_RSSI  -80

int TRANSMITPERIOD = 200; //Transmitir un paquete al GATEWAY (en ms)
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buff[20];
byte sendSize = 0;
boolean requestACK = false;

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

void setup() {
  Serial.begin(115200);
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  #ifdef IS_RFM69HW_HCW
    radio.setHighPower();
  #endif
  radio.encrypt(ENCRYPTKEY);
  //radio.setFrequency(919000000);
  #ifdef ENABLE_ATC
    radio.enableAutoPower(ATC_RSSI);
  #endif

  Serial.println("Transmitting at 915Mhz...");
  #ifdef ENABLE_ATC
    Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
  #endif
}

void blinkLed(byte pin, int delayMs){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
}

long lastPeriod = 0;
void loop() {
  //Checa si se recibio un paquete
  if(radio.receiveDone())
  {
    Serial.print("[");Serial.print(radio.SENDERID, DEC);Serial.print("]");
    for(byte i = 0; i < radio.DATALEN; i++)
    {
      Serial.print((char)radio.DATA[i]);
      Serial.print("  [RX_RSSI: ");Serial.print(radio.RSSI);Serial.print("]");
      if(radio.ACKRequested())
      {
        radio.sendACK();
        Serial.print(" - ACK sent");  
      }
      blinkLed(LED_BUILTIN, 3);
      Serial.println();
    }
  }

  int currPeriod = millis()/TRANSMITPERIOD;
  if(currPeriod != lastPeriod)
  {
    lastPeriod = currPeriod;

    Serial.print("Sending[");
    Serial.print(sendSize);
    Serial.print("]: ");
    for(byte i = 0; i < sendSize; i++)
    {
      Serial.print((char)payload[i]);
    }
    if(radio.sendWithRetry(GATEWAYID, payload, sendSize))
    {
      Serial.print(" ok!");
    }
    else
    {
      Serial.print(" nothing...");
    }
    
    sendSize = (sendSize + 1) % 31;
    Serial.println();
    blinkLed(LED_BUILTIN,3);
  }
}
