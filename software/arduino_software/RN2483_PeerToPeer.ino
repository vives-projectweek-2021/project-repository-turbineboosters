#include <DHT.h>
#include <DHT_U.h>

#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB
#define btSerial                Serial

#define LED_PIN                 9
#define CURRENT_PIN             A1
#define VOLTAGE_PIN             A0

#define DHT11PIN                4
#define DHTTYPE                 DHT11

#define R_SHUNT                 0.5     //Ohms
#define R1                      100000  //Ohms
#define R2                      8200    //Ohms

#define ADC_RESOLUTION          12      //Bits
#define SUPPLY_VOLTAGE          3.3     //Volt
#define SAMPLE_INTERVAL         10     //ms
#define N_SAMPLES               10

// Both devices mush have the same AES-key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// set the address of the device where you want te send a packet to
const byte targetAddress[1] = {0x00};

// the adress of this device (The adresses can be the same. It won't loop back)
const byte deviceAddress[1] = {0x00};

bool ledState = false;
bool stat = true;

DHT dht(DHT11PIN, DHTTYPE);
RN2483_P2P peerToPeer(usbSerial, loraSerial);

void setup() {
    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    pulse();

    btSerial.begin(9600);
    usbSerial.begin(115200);
    loraSerial.begin(57600);
    loraSerial.setTimeout(1000);
    lora_autobaud();

    usbSerial.println("initializing...");
    
    peerToPeer.initLoRa();
    peerToPeer.setAesKey(key);
    peerToPeer.setAddress(deviceAddress);

    analogReadResolution(ADC_RESOLUTION);

    dht.begin();
}


void loop() 
{
    // uncomment for the transmitter
    transmit(); 

    // uncomment for the receiver
    //peerToPeer.receiveMessage(handleMessage);
}

void printPayload(const byte *payload){
    double humid = ((payload[8]<<8)|(payload[9]))/100.0;
    double temp = ((payload[6]<<8)|(payload[7]))/100.0;
    double volt = ((payload[4]<<8)|(payload[5]))/100.0;
    double amp = ((payload[2]<<8)|(payload[3]))/100.0;
    double power = ((payload[0]<<8)|(payload[1]))/100.0;

    usbSerial.print("Payload: ");
    for (int i=0; i<10; i++){
        usbSerial.print(payload[i] >> 4, HEX);
        usbSerial.print(payload[i] & 0x0f, HEX);
    }
    usbSerial.println();

    usbSerial.print("Temperature: ");
    usbSerial.print(temp);
    usbSerial.println("°C");
    
    usbSerial.print("Humitity: ");
    usbSerial.print(humid);
    usbSerial.println("%");
    
    usbSerial.print("Voltage: ");
    usbSerial.print(volt);
    usbSerial.println("V");
    
    usbSerial.print("Amps: ");
    usbSerial.print(amp);
    usbSerial.println("A");
    
    usbSerial.print("Power: ");
    usbSerial.print(power);
    usbSerial.println("W");

    usbSerial.println();
}

void handleMessage(const byte *payload)
{
    // pulse status led
    pulse();

    // decode the payload into usable values
    double humid = ((payload[8]<<8)|(payload[9]))/100.0;
    double temp = ((payload[6]<<8)|(payload[7]))/100.0;
    double volt = ((payload[4]<<8)|(payload[5]))/100.0;
    double amp = ((payload[2]<<8)|(payload[3]))/100.0;
    double power = ((payload[0]<<8)|(payload[1]))/100.0;

    // Printing payload to btSerial and usbSerial
    usbSerial.print("Payload: ");
    for (int i=0; i < 10; i++){
        btSerial.print(payload[i] >> 4, HEX);
        btSerial.print(payload[i] & 0x0f, HEX);
    }
    
    btSerial.println();

    // print to usb serial
    printPayload(payload);
}

void transmit() {
    byte tempPayload[10] = {0};    

    int sumVolt = 0;
    int sumAmp = 0;
    double sumTemp = 0;
    double sumHumid = 0;
    
    for (int i=0; i<N_SAMPLES; i++) {        
        sumTemp += dht.readTemperature();
        sumHumid += dht.readHumidity();
        sumAmp += analogRead(CURRENT_PIN);
        sumVolt += analogRead(VOLTAGE_PIN);
        delay(SAMPLE_INTERVAL);
    }

    double volt = (sumVolt/N_SAMPLES)*(SUPPLY_VOLTAGE / pow(2,ADC_RESOLUTION))*(1+(R1/R2)); 
    double amp = (sumAmp/N_SAMPLES)*(SUPPLY_VOLTAGE / pow(2,ADC_RESOLUTION))/R_SHUNT;
    double temp = sumTemp/N_SAMPLES;
    double humid = sumHumid/N_SAMPLES;

    // add values to payload for transmission through lora
    tempPayload[0] = (int)(volt*amp*100)>>8;
    tempPayload[1] = (int)(volt*amp*100)&0x00ff;
    
    tempPayload[2] = (int)(amp*100)>>8;
    tempPayload[3] = (int)(amp*100)&0x00ff;
    
    tempPayload[4] = (int)(volt*100)>>8;
    tempPayload[5] = (int)(volt*100)&0x00ff;

    tempPayload[6] = (int)(temp*100)>>8;
    tempPayload[7] = (int)(temp*100)&0x00ff;
    
    tempPayload[8] = (int)(humid*100)>>8;
    tempPayload[9] = (int)(humid*100)&0x00ff;

    usbSerial.println("Decoded: ");
    printPayload(tempPayload);
    usbSerial.println();

    // transmit payload through LoRa
    peerToPeer.transmitMessage(tempPayload, targetAddress);

    // pulse led to display that the data has been sent
    pulse();
}

void lora_autobaud()
{
    String response = "";
    while (response=="")
    {
        delay(1000);
        loraSerial.write((byte)0x00);
        loraSerial.write(0x55);
        loraSerial.println();
        loraSerial.println("sys get ver");
        response = loraSerial.readStringUntil('\n');
    }
}

void pulse()
{
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
}
