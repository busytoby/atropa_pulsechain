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

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/platform.h>
#include <mbedtls/bignum.h>

#include <LittleFS.h>

HWCDC HWCDCSerial;

/********************************* lora  *********************************************/
#define RF_FREQUENCY                                954114361 // Hz

#define TX_OUTPUT_POWER                             26        // dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            400
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
  memcpy(rxpacket, payload, size );
  rxpacket[size]='\0';
	//Radio.Sleep();
	int i = 0;
	for(; i < size; i++) if(payload[i] != '\0') Serial.write(payload[i]);
	if(i > 0) {
		Serial.write('\n');
		rxNumber++;
	}
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

char Version[8] = "0.209";
char Handle[20] = "[:h changeme]";

void SaveConfig() {
	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "w");
	if(!cf) Serial.println("Failed To Open Config File For Writing");
	else {
		cf.printf("h %s\n", Handle);
	}
	cf.close();
	Serial.println("Wrote /hel_config");
}

void PrintConfig() {
	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "r");
	if(!cf) {
		Serial.println("No Config File");
	} else {
		Serial.println("Reading /hel_config");
		while (cf.available()) Serial.write(cf.read());
	}
	cf.close();
}

char mpibuf[256];
char* mpistring(const mbedtls_mpi V) {
	int ret;	
	size_t n_written;
	memset(mpibuf, 0, sizeof(mpibuf));
	ret = mbedtls_mpi_write_string(&V, 10, mpibuf, sizeof(mpibuf) - 1, &n_written);
	if(ret == 0) {
		mpibuf[n_written] = '\0';
		for(int i = 1; i < n_written; i+=2) mpibuf[i] = tolower(mpibuf[i]);
		return mpibuf;
	}

  Serial.printf("MPIstring Error: -0x%04X\n", -ret);
}

mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
const char* APOGEE = "953473";
const char* APEX = "954114361";
const char* MotzkinPrime = "953467954114363";
mbedtls_mpi m, x, b, y, s, l, g, i, o, q, t, d, H, L;
// sints should be configurable and discoverable in +- order via prime test
mbedtls_mpi_sint lb = 99;
mbedtls_mpi_sint gb = 49;
mbedtls_mpi_sint ib = 211;
mbedtls_mpi_sint ob = 257;
mbedtls_mpi_sint qb = 116;
mbedtls_mpi_sint tb = 683;
mbedtls_mpi_sint db = 110;
mbedtls_mpi_sint Hb = 187;
mbedtls_mpi_sint Lb = 591;
void MathInit() {
	mbedtls_mpi m, x, b;
	mbedtls_mpi_init(&m);
	mbedtls_mpi_init(&x);
	mbedtls_mpi_init(&b);
	mbedtls_mpi_init(&y);
	mbedtls_mpi_init(&s);
	mbedtls_mpi_init(&l);
	mbedtls_mpi_init(&g);
	mbedtls_mpi_init(&i);
	mbedtls_mpi_init(&o);
	mbedtls_mpi_init(&q);
	mbedtls_mpi_init(&t);
	mbedtls_mpi_init(&d);
	mbedtls_mpi_init(&H);
	mbedtls_mpi_init(&L);

  mbedtls_entropy_init( &entropy );
  mbedtls_ctr_drbg_init( &ctr_drbg );

	mbedtls_mpi_read_string(&m, 10, APOGEE);
	mbedtls_mpi_read_string(&x, 10, APEX);
	mbedtls_mpi_read_string(&b, 10, MotzkinPrime);
	char* DysnomiaPrime = (char*) calloc(strlen(APOGEE + strlen(APEX)), sizeof(char));
	strcpy(DysnomiaPrime, APOGEE);
	strcat(DysnomiaPrime, APEX);
	mbedtls_mpi_read_string(&y, 10, DysnomiaPrime);
	mbedtls_mpi_mul_mpi(&s, &m, &x);
	mbedtls_mpi_add_mpi(&s, &s, &b);
	mbedtls_mpi_mul_mpi(&l, &m, &s);
	mbedtls_mpi_sub_int(&l, &l, lb);
	mbedtls_mpi_mul_mpi(&g, &m, &y);
	mbedtls_mpi_add_mpi(&g, &g, &b);
	mbedtls_mpi_mul_mpi(&g, &g, &m);
	mbedtls_mpi_sub_int(&g, &g, gb);
	mbedtls_mpi_mul_mpi(&i, &m, &b);
	mbedtls_mpi_add_mpi(&i, &i, &y);
	mbedtls_mpi_sub_int(&i, &i, ib);
	mbedtls_mpi_mul_mpi(&o, &s, &s);
	mbedtls_mpi_sub_int(&o, &o, ob);
	mbedtls_mpi_mul_mpi(&q, &i, &i);
	mbedtls_mpi_sub_int(&q, &q, qb);
	mbedtls_mpi_mul_mpi(&t, &o, &g);
	mbedtls_mpi_add_mpi(&t, &t, &q);
	mbedtls_mpi_sub_int(&t, &t, tb);
	mbedtls_mpi_mul_mpi(&d, &g, &s);
	mbedtls_mpi_add_mpi(&d, &d, &o);
	mbedtls_mpi_mul_mpi(&d, &g, &d);
	mbedtls_mpi_sub_int(&d, &d, db);
	mbedtls_mpi_add_int(&H, &d, db);
	mbedtls_mpi_sub_mpi(&H, &H, &b);
	mbedtls_mpi_sub_int(&H, &H, Hb);
	mbedtls_mpi k;
	mbedtls_mpi_init(&k);
	mbedtls_mpi_lset (&k, 77);
	mbedtls_mpi_mul_mpi(&k, &k, &k);
	mbedtls_mpi_mul_mpi(&L, &t, &t);
	mbedtls_mpi_sub_mpi(&L, &L, &k);
	mbedtls_mpi_free(&k);
	mbedtls_mpi_sub_int(&L, &L, Lb);

  delay(500);
	Serial.printf("%12s m= 0x%s\n", "APOGEE", mpistring(m)); delay(100);
	Serial.printf("%12s x= 0x%s\n", "APEX", mpistring(x)); delay(100);
	Serial.printf("%12s b= 0x%s\n", "MotzkinPrime", mpistring(b)); delay(100);
	Serial.printf("%12s y= 0x%s\n", "DYSNOMIA", mpistring(y)); delay(100);
	Serial.printf("%12s s= 0x%s\n", "SLOPE", mpistring(s)); delay(100);
	Serial.printf("%12s l= 0x%s\n", "LOVE", mpistring(l)); delay(100);
	Serial.printf("%12s g= 0x%s\n", "GAIN", mpistring(g)); delay(100);
	Serial.printf("%12s i= 0x%s\n", "_[1]", mpistring(i)); delay(100);
	Serial.printf("%12s o= 0x%s\n", "__[2]", mpistring(o)); delay(100);
	Serial.printf("%12s q= 0x%s\n", "___[3]", mpistring(q)); delay(100);
	Serial.printf("%12s t= 0x%s\n", "____[4]", mpistring(t)); delay(100);
	Serial.printf("%12s d= 0x%s\n", "_____[5]", mpistring(d)); delay(100);
	Serial.printf("%12s H= 0x%s\n", "______[6]", mpistring(H)); delay(100);
	Serial.printf("%12s L= 0x%s\n", "_______[7]", mpistring(L)); delay(100);

	mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                        (const unsigned char *)mpibuf,
                        strlen(mpibuf));
}

void ProcessCmd() {
	while(Serial.available()) {
		uint8_t sData = Serial.read();
		if(sData == 'a') Serial.printf("random = %d\n", Radio.Random());
		else if(sData == 'c') PrintConfig();
		else if(sData == 'h') {
			char* htxt = (char*)calloc(20, sizeof(char));
			Serial.readBytes(htxt, 20);	
			for(int i = strlen(htxt); i > 0; i--) if(htxt[i] == '\n') htxt[i] = '\0';
			int j = strlen(htxt);
			while(htxt[0] == ' ') for(int i=0; i < j - 1; i++) { htxt[i] = htxt[i+1]; htxt[i+1] = 0; }
			if(strlen(htxt) > 1) strcpy(Handle, htxt);
			Serial.printf("Handle: %s\n", Handle);
			free(htxt);
		}
		else if(sData == 'r') Serial.printf("rxCount = %d\n", rxNumber);
		else if(sData == 's') {
			uint8_t vData = Serial.read();
			uint8_t mData = Serial.read();
			mbedtls_mpi_sint sd;
			mbedtls_mpi_sint sd2;
			mbedtls_mpi M, M2;
			mbedtls_mpi_init(&M);
			mbedtls_mpi_init(&M2);
			if(vData == 'l') { sd = lb; mbedtls_mpi_copy(&M, &l);
			} else if(vData == 'g') { sd = gb; mbedtls_mpi_copy(&M, &g);
			} else if(vData == 'i') { sd = ib; mbedtls_mpi_copy(&M, &i);
			} else if(vData == 'o') { sd = ob; mbedtls_mpi_copy(&M, &o);
			} else if(vData == 'q') { sd = qb; mbedtls_mpi_copy(&M, &q);
			} else if(vData == 't') { sd = tb; mbedtls_mpi_copy(&M, &t);
			} else if(vData == 'd') { sd = db; mbedtls_mpi_copy(&M, &d);
		 	} else if(vData == 'H') { sd = Hb; mbedtls_mpi_copy(&M, &H);
			} else if(vData == 'L') { sd = Lb; mbedtls_mpi_copy(&M, &L);
			}
			int ret;
			sd2 = sd;
			if(mData == '-') {
				while(true) {
					sd2--;
					mbedtls_mpi_sub_int(&M2, &M, sd2);
					ret = mbedtls_mpi_is_prime_ext(&M2, 25, mbedtls_ctr_drbg_random, &ctr_drbg);
					delay(20);
					if(ret == 0) break;
				}
			} else if (mData == '+') {
				while(true) {
					sd2++;
					mbedtls_mpi_add_int(&M2, &M, sd2);
					ret = mbedtls_mpi_is_prime_ext(&M2, 25, mbedtls_ctr_drbg_random, &ctr_drbg );
					delay(5);
					if(ret == 0) break;
				}
			}

			if(sd != sd2) {
				if(vData == 'l') lb = sd2;
				else if(vData == 'g') gb = sd2;
				else if(vData == 'i') ib = sd2;
				else if(vData == 'o') ob = sd2;
				else if(vData == 'q') qb = sd2;
				else if(vData == 't') tb = sd2;
				else if(vData == 'd') db = sd2;
				else if(vData == 'H') Hb = sd2;
				else if(vData == 'L') Lb = sd2;
				MathInit();
			}
			Serial.printf("%cb [%s] oldValue: %d newValue: %d\n", (char)(vData), mpistring(M), sd, sd2);
			mbedtls_mpi_free(&M);
			mbedtls_mpi_free(&M2);
		}
		else if(sData == 't') Serial.printf("txCount = %d\n", txNumber);
		else if(sData == 'x') SaveConfig();
		
	}
}

// needed for first flash & after any flash clear via left+right buttons
// serials come from https://resource.heltec.cn/search
// use product id from ie: ESP32ChipID=5C9482697090
//uint32_t license[4] = { 0xBF91E8F9,0xA26B051E,0xA310D34A,0x9316739B }; // ACM1

void setup()
{
	Serial.setRxBufferSize(8192);
	Serial.begin(115200);
	//Mcu.setlicense(license, HELTEC_BOARD);
	chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	while(!Serial) continue;

	MathInit();

	Serial.println("\nMounting LittleFS ...");

  // Initialize LittleFS
  if (!LittleFS.begin(true)) { // 'true' formats the filesystem if the mount fails
    Serial.println("Failed to mount LittleFS filesystem");
    return;
  }
  Serial.println("LittleFS mounted successfully");

	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "r");
	if(!cf) {
		Serial.println("No Existing Config Found");
	} else {
		Serial.println("Reading /hel_config");
		int index = 0;
		while (cf.available()) {
			uint8_t sData = cf.read();
			if(sData == 'h') {
				cf.read(); // skip
				index = 0;
				memset(Handle, 0, 20);
				while(cf.available()) {
					uint8_t sData = cf.read();
					if(sData == '\n') break;
					Handle[index++] = sData;
			  }
			}
			if(cf.available() && sData != '\n') {
				Serial.println("Malformed Save File");
				break;
			}
		}
		cf.close();
	}

	SaveConfig();

	VextOFF();
	delay(100);
	//factory_display.init();
	//factory_display.clear();
	delay(100);

	Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
	delay(100);

	lora_init();
  delay(100);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);  

	Radio.Rx(0);
}

void SendToRadio(const char* txt) {
	lora_idle = false;
	last_tx = millis();
	++txNumber;
  if(txt == NULL) {
		memset(txpacket, 0, sizeof(txpacket));
		uint32_t r = Radio.Random();
		sprintf(txpacket,"失調症 %u 呂 例子:%s %u 水%s [約:%u]",txNumber, Handle, r, Version, rxNumber);
	}
	else if(txt != txpacket) {
		memset(txpacket, 0, sizeof(txpacket));	
		strcpy(txpacket, txt);
	}
  Radio.Send((uint8_t *)txpacket, strlen(txpacket));
	Serial.printf(": %s\n", txpacket);
}

int idlemod = 1;
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
					index = 0;
				} else txpacket[index++] = sData;
			}
  	} else if (millis() - last_tx > 55555 && lora_idle) SendToRadio(NULL);
		delay(10);
	} 
	if((millis() - last_tx) > (1551*idlemod)) {
		idlemod++;
		Radio.Send(NULL, NULL);
	}
  delay(20);    
}
