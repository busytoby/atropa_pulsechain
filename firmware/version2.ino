/*
 * HelTec Automation(TM) WIFI_LoRa_32 factory test code, witch includ
 * follow functions:
 * 
 * - Basic OLED function test;
 * 
 * - Basic serial port test(in baud rate 115200);
 * 
 * - LED blink test;
 * 
 * - WIFI connect and scan test;
 * 
 * - LoRa Ping-Pong test (DIO0 -- GPIO26 interrup check the new incoming messages);
 * 
 * - Timer test and some other Arduino basic functions.
 *
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/
#define WIFI_LORA_32_V4 1
#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1

#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

HWCDC HWCDCSerial;

/********************************* lora  *********************************************/
#define RF_FREQUENCY                                954114361 // Hz

#define TX_OUTPUT_POWER                             10        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 140 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum
{
    LOWPOWER,
    STATE_RX,
    STATE_TX
}States_t;

static uint32_t last_tx = 0;
int16_t txNumber;
int16_t rxNumber;
States_t state;
bool sleepMode = false;
int16_t rxSize;

unsigned int counter = 0;
bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends
uint64_t chipid;

bool lora_idle = true;

void OnTxDone( void )
{
	//Serial.print("TX done......");
	state=STATE_RX;
	lora_idle = true;
	Radio.Rx(0);
}

void OnTxTimeout( void )
{
  //Radio.Sleep( );
  Serial.print("TX Timeout......");
	state=STATE_TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	rxNumber++;
  memcpy(rxpacket, payload, size );
  rxpacket[size]='\0';
	//Radio.Sleep();
	int i = 0;
	for(; i < size; i++) if(payload[i] != '\0') Serial.write(payload[i]);
	if(i > 0) Serial.write('\n');
	receiveflag = true;
  state=STATE_TX;
	lora_idle = true;
}


void lora_init(void)
{
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  txNumber=0;
  rxNumber = 0;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
	RadioEvents.RxTimeout = []() { lora_idle = true; Radio.Rx(0); };

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 500 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                 LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                 LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 0, true, 0, 0, LORA_IQ_INVERSION_ON, 500 );
	state=STATE_TX;
}


/********************************* lora  *********************************************/

//SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

bool resendflag=true;
bool deepsleepflag=false;

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void setup()
{
	Serial.begin(115200);
	VextOFF();
	delay(100);
	//factory_display.init();
	//factory_display.clear();
	delay(100);

	chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	delay(100);
	Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
	delay(100);

	lora_init();
  delay(100);
	pinMode(LED ,OUTPUT);
	digitalWrite(LED, LOW);  

	Radio.Rx(0);
}

void SendToRadio(const char* txt) {
	lora_idle = false;
	last_tx = millis();
	memset(txpacket, 0, sizeof(txpacket));	
  if(txt == NULL) sprintf(txpacket,"失調症 %d 呂 例子:ACM2",++txNumber);
	else if(txt != txpacket) strcpy(txpacket, txt);
  Radio.Send((uint8_t *)txpacket, strlen(txpacket));
	++txNumber;
}

void ProcessCmd() {
	while(Serial.available()) {
		uint8_t sData = Serial.read();
		if(sData == 'a') Serial.printf("random = %d\n", Radio.Random());
		else if(sData == 'r') Serial.printf("rxCount = %d\n", rxNumber);
		else if(sData == 't') Serial.printf("txCount = %d\n", txNumber);
	}
}

void loop()
{
  Radio.IrqProcess();

  if (millis() - last_tx > 500 && lora_idle) {
		int index = 0;
    if(Serial.available()) {
			memset(txpacket, '\0', sizeof(txpacket));
			while(Serial.available()) {
				uint8_t sData = Serial.read();
				if(index == 0 && sData == ':') ProcessCmd();
				else if(sData == '\n') {
					SendToRadio(txpacket);
					//vTaskDelay(pdMS_TO_TICKS(100));
					delay(100);
					index = 0;
				} else {
					txpacket[index++] = sData;
				}
			}
  	} else if (millis() - last_tx > 55555 && lora_idle) {			
			SendToRadio(NULL);
			delay(100);
		}
	} else Radio.Send(NULL, NULL);
  delay(233);    
}
