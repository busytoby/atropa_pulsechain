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

//#include <MUIU8g2.h>
#include <U8g2lib.h>
//#include <U8x8lib.h>

#include <mbedtls/rsa.h>
#include <mbedtls/md.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/platform.h>
#include <mbedtls/bignum.h>

#include <LittleFS.h>

HWCDC HWCDCSerial;
SemaphoreHandle_t mutex;

/********************************* lora  *********************************************/
// SCOUT THE PUBLIC LICENSE AMYGDALA AT
//#define RF_FREQUENCY                                954114361 // Hz
//#define RF_FREQUENCY                                715585771 // Hz
//#define RF_FREQUENCY                                477057180 // Hz
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12        // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        5         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            400
#define BUFFER_SIZE                                 232 // Define the payload size here

//SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, RST_OLED, SCL_OLED, SDA_OLED); 

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
static uint32_t last_refresh = 0;
static bool ScreenOn=true;
static const size_t DISPLAY_CHARS=37;
static char screenlines[6][DISPLAY_CHARS];
static int last_line = 0;

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
  Serial.print("# TX Timeout......");
	state=STATE_TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{		
  memcpy(rxpacket, payload, size );
  rxpacket[size]='\0';
	//Radio.Sleep();
	int i = 0;
	for(; i < size; i++) if(payload[i] != '\0') Serial.write(payload[i]);
	if(i > 1) {
		Serial.write('\n');
		rxNumber++;
		
		xSemaphoreTake(mutex, 400);
		if(last_line < 5) {
			memset(screenlines[last_line], 0, DISPLAY_CHARS);
			strncpy(screenlines[last_line], rxpacket, DISPLAY_CHARS);
			screenlines[last_line][DISPLAY_CHARS] = '\0';
			last_line++;
		} else {
			if(last_line > 5) {
				for(int j = 0; j < 5; j++) {
					memset(screenlines[j], 0, DISPLAY_CHARS);
					strncpy(screenlines[j], screenlines[j+1], DISPLAY_CHARS);
					screenlines[j][DISPLAY_CHARS] = '\0';
				}
			}
			memset(screenlines[5], 0, DISPLAY_CHARS);
			strncpy(screenlines[5], rxpacket, DISPLAY_CHARS);
			screenlines[5][DISPLAY_CHARS] = '\0';
			if(last_line == 5) last_line++;
		}
		last_refresh = 0;
		xSemaphoreGive(mutex);
	}
	receiveflag = true;
  state=STATE_TX;
	lora_idle = true;
}


void lora_init(void)
{
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  txNumber=0;
  rxNumber=0;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
	RadioEvents.RxTimeout = []() { lora_idle = true; Radio.Rx(0); };

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 400 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                 LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                 LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 0, true, 0, 0, LORA_IQ_INVERSION_ON, 400 );
	state=STATE_TX;
}


/********************************* lora  *********************************************/

bool resendflag=true;

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
	Serial.println("# Screen ON");
	ScreenOn=true;
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
	Serial.println("# Screen OFF");
	ScreenOn=false;
}

void interrupt_GPIO0(void)
{
	if(ScreenOn) VextOFF();
	else VextON();
}

char Version[8] = "0.222";
char Handle[20] = "[:h changeme]";

void SaveConfig() {
	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "w");
	if(!cf) Serial.println("# Failed To Open Config File For Writing");
	else {
		cf.printf("h %s\n", Handle);
	}
	cf.close();
	Serial.println("# Wrote /hel_config");
}

void PrintConfig() {
	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "r");
	if(!cf) {
		Serial.println("# No Config File");
	} else {
		Serial.println("# Reading /hel_config");
		while (cf.available()) Serial.write(cf.read());
	}
	cf.close();
}

static char mpibuf[256] = "0x069e8fC82e18F4";
char* mpistring(const mbedtls_mpi V) {
	int ret;	
	size_t n_written;
	memset(mpibuf, 0, sizeof(mpibuf));
	ret = mbedtls_mpi_write_string(&V, 16, mpibuf, sizeof(mpibuf) - 1, &n_written);
	if(ret == 0 && n_written > 0) {
		mpibuf[n_written] = '\0';
		for(int i = 1; i < n_written; i+=2) mpibuf[i] = tolower(mpibuf[i]);
		return mpibuf;
	}

  Serial.printf("# MPIstring Error: -0x%04X\n", -ret);
}

#define PRIMEVERIFYROUNDS 1024
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
const char* APOGEE = "953473";
const char* APEX = "954114361";
const char* MotzkinPrime = "953467954114363";
mbedtls_mpi m, x, b, y, s, l, g, i, o, q, t, d, H, L;
bool math_init_complete = false;
// sints should be configurable and discoverable in +- order via prime test
mbedtls_mpi_sint lb = 99;
mbedtls_mpi_sint gb = 49;
mbedtls_mpi_sint ib = 269;
mbedtls_mpi_sint ob = 257;
mbedtls_mpi_sint qb = 132;
mbedtls_mpi_sint tb = 693;
mbedtls_mpi_sint db = 110;
mbedtls_mpi_sint Hb = 187;
mbedtls_mpi_sint Lb = 100;
void MathInit() {
	if(!math_init_complete) {
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

		mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                         (const unsigned char *)mpibuf,
                         strlen(mpibuf));
		math_init_complete = true;
	} else mbedtls_ctr_drbg_reseed(&ctr_drbg, (const unsigned char *)mpibuf, strlen(mpibuf));
	
	mbedtls_mpi_read_string(&m, 10, APOGEE);
	mbedtls_mpi_read_string(&x, 10, APEX);
	mbedtls_mpi_read_string(&b, 10, MotzkinPrime);
	mbedtls_mpi_mul_mpi(&y, &m, &x);
	mbedtls_mpi_add_mpi(&y, &y, &b);
	char DysnomiaPrime[strlen(APOGEE) + strlen(APEX)];		
	strcpy(DysnomiaPrime, APOGEE);
	strcat(DysnomiaPrime, APEX);
	mbedtls_mpi_read_string(&s, 10, DysnomiaPrime);

	Serial.printf("# %12s m= 0x%s\n", "APOGEE", mpistring(m)); delay(100);
	Serial.printf("# %12s x= 0x%s\n", "APEX", mpistring(x)); delay(100);
	Serial.printf("# %12s b= 0x%s\n", "MotzkinPrime", mpistring(b)); delay(100);
	Serial.printf("# %12s y= 0x%s\n", "SLOPE", mpistring(s)); delay(100);
	Serial.printf("# %12s s= 0x%s\n", "DYSNOMIA", mpistring(y)); delay(100);

	mbedtls_mpi_mul_mpi(&l, &m, &y);
	mbedtls_mpi_sub_int(&l, &l, lb);
	while(mbedtls_mpi_is_prime_ext(&l, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&l, &l, 1);
		lb++;
		delay(20);
	}
	Serial.printf("# %12s l= 0x%s [%lld]\n", "LOVE", mpistring(l), (long long)lb); delay(100);

	mbedtls_mpi_mul_mpi(&g, &m, &s);
	mbedtls_mpi_add_mpi(&g, &g, &b);
	mbedtls_mpi_mul_mpi(&g, &g, &m);
	mbedtls_mpi_sub_int(&g, &g, gb);
	while(mbedtls_mpi_is_prime_ext(&g, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&g, &g, 1);
		gb++;
		delay(20);
	}
	Serial.printf("# %12s g= 0x%s [%lld]\n", "GAIN", mpistring(g), (long long)gb); delay(100);

	mbedtls_mpi_mul_mpi(&i, &m, &b);
	mbedtls_mpi_add_mpi(&i, &i, &s);
	mbedtls_mpi_sub_int(&i, &i, ib);
	while(mbedtls_mpi_is_prime_ext(&i, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&i, &i, 1);
		ib++;
		delay(20);
	}
	Serial.printf("# %12s i= 0x%s [%lld]\n", "_[1]", mpistring(i), (long long)ib); delay(100);

	mbedtls_mpi_mul_mpi(&o, &y, &y);
	mbedtls_mpi_sub_int(&o, &o, ob);
	while(mbedtls_mpi_is_prime_ext(&o, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&o, &o, 1);
		ob++;
		delay(20);
	}
	Serial.printf("# %12s o= 0x%s [%lld]\n", "__[2]", mpistring(o), (long long)ob); delay(100);
	
	mbedtls_mpi_mul_mpi(&q, &i, &i);
	mbedtls_mpi_sub_int(&q, &q, qb);
	while(mbedtls_mpi_is_prime_ext(&q, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&q, &q, 1);
		qb++;
		delay(10);
	}
	Serial.printf("# %12s q= 0x%s [%lld]\n", "___[3]", mpistring(q), (long long)qb); delay(100);
	
	mbedtls_mpi_mul_mpi(&t, &o, &g);
	mbedtls_mpi_add_mpi(&t, &t, &q);
	mbedtls_mpi_sub_int(&t, &t, tb);
	while(mbedtls_mpi_is_prime_ext(&t, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&t, &t, 1);
		tb++;
		delay(10);
	}
	Serial.printf("# %12s t= 0x%s [%lld]\n", "____[4]", mpistring(t), (long long)tb); delay(100);
	
	mbedtls_mpi_mul_mpi(&d, &g, &y);
	mbedtls_mpi_add_mpi(&d, &d, &o);
	mbedtls_mpi_mul_mpi(&d, &g, &d);
	mbedtls_mpi_sub_int(&d, &d, db);
	while(mbedtls_mpi_is_prime_ext(&d, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&d, &d, 1);
		db++;
		delay(5);
	}
	Serial.printf("# %12s d= 0x%s [%lld]\n", "_____[5]", mpistring(d), (long long)db); delay(100);
	
	mbedtls_mpi_add_int(&H, &d, db);
	mbedtls_mpi_sub_mpi(&H, &H, &b);
	mbedtls_mpi_sub_int(&H, &H, Hb);
	while(mbedtls_mpi_is_prime_ext(&H, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&H, &H, 1);
		Hb++;
		delay(5);
	}
	Serial.printf("# %12s H= 0x%s [%lld]\n", "______[6]", mpistring(H), (long long)Hb); delay(100);
	
	mbedtls_mpi k;
	mbedtls_mpi_init(&k);
	mbedtls_mpi_lset(&k, Hb - db);
	mbedtls_mpi_mul_mpi(&k, &k, &k);
	mbedtls_mpi_mul_mpi(&L, &t, &t);
	mbedtls_mpi_sub_mpi(&L, &L, &k);
	mbedtls_mpi_free(&k);
	mbedtls_mpi_sub_int(&L, &L, Lb);
	while(mbedtls_mpi_is_prime_ext(&L, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
		mbedtls_mpi_sub_int(&L, &L, 1);
		Lb++;
		delay(5);
	}
	Serial.printf("# %12s L= 0x%s [%lld]\n", "_______[7]", mpistring(L), (long long)Lb); delay(100);
}

char DateTime[40];
char* DTString() {
	uint64_t ctime = esp_timer_get_time();
	uint64_t TPD = 86400000000;
	uint64_t TPH = TPD / 34;
  uint64_t TPM = TPH / 100;
  uint64_t TPS = TPM / 34;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute; 
	uint16_t Second;

	memset(DateTime, 0, 40);
	Day = (uint16_t)ctime / TPD;
  Hour = (ctime - (TPD * Day)) / TPH;
  Minute = ((ctime - (TPD * Day) - (TPH * Hour)) / TPM);
  Second = ((ctime - (TPD * Day) - (TPH * Hour) - (TPM * Minute)) / TPS);
	snprintf(DateTime, 40, "Day:%X %02X:%02X.%02X", Day, Hour, Minute, Second);
	return DateTime;
}

static char KeyEntropy[96];
char* GenKey() {
	for(int i = 0; i < 89; i+=8) {
		long long r3 = Radio.Random();
		KeyEntropy[i] ^= *(unsigned char*)&r3 & 0xF;
		KeyEntropy[i+1] ^= *((unsigned char*)&r3) >> 1 & 0xF;
		KeyEntropy[i+2] ^= *((unsigned char*)&r3+1) & 0xF;
		KeyEntropy[i+3] ^= *((unsigned char*)&r3+1) >> 1 & 0xF;
		KeyEntropy[i+4] ^= *((unsigned char*)&r3+2) & 0xF;
		KeyEntropy[i+5] ^= *((unsigned char*)&r3+2) >> 1 & 0xF;
		KeyEntropy[i+6] ^= *((unsigned char*)&r3+3) & 0xF;
		KeyEntropy[i+7] ^= *((unsigned char*)&r3+3) >> 1 & 0xF;
	}

	//KeyEntropy[95] = '\0';
	mpistring(L);
	char NewKey[strlen(mpibuf)+1];
	for(int i = 0; i <= strlen(mpibuf); i++) {
		NewKey[i] = (KeyEntropy[i] % 16) + '0';
		if(NewKey[i] > '9')  NewKey[i] = NewKey[i] - 10 - '0' + ((i%2==0)?'A':'a');
		//Serial.printf("%d %d %d %d\n", KeyEntropy[i], mpibuf[i], (KeyEntropy[i] % 16) + '0', 'A');
	}
	NewKey[strlen(mpibuf)] = '\0';

	strcpy(mpibuf, NewKey);
	return mpibuf;
}

void ProcessCmd() {
	while(Serial.available()) {
		uint8_t sData = Serial.read();
		if(sData == 'a') Serial.printf("# random = %d\n", Radio.Random());
		else if(sData == 'b') Serial.printf("# time = %s\n", DTString());
		else if(sData == 'c') PrintConfig();
		else if(sData == 'h') {
			char htxt[20];
			Serial.readBytes(htxt, 20);	
			for(int i = strlen(htxt); i > 0; i--) if(htxt[i] == '\n') htxt[i] = '\0';
			int j = strlen(htxt);
			while(htxt[0] == ' ') for(int i=0; i < j - 1; i++) { htxt[i] = htxt[i+1]; htxt[i+1] = 0; }
			if(strlen(htxt) > 1) strcpy(Handle, htxt);
			Serial.printf("# Handle: %s\n", Handle);
		}
		else if(sData == 'n') Serial.printf("# New Key: 0x%s\n", GenKey());
		else if(sData == 'r') Serial.printf("# rxCount = %d\n", rxNumber);
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
			mbedtls_mpi_add_int(&M, &M, sd);
			if(mData == '-') {
				while(true) {
					sd2--;
					mbedtls_mpi_sub_int(&M2, &M, sd2);
					ret = mbedtls_mpi_is_prime_ext(&M2, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg);
					delay(20);
					if(ret == 0) break;
				}
			} else if (mData == '+') {
				while(true) {
					sd2++;
					mbedtls_mpi_sub_int(&M2, &M, sd2);
					ret = mbedtls_mpi_is_prime_ext(&M2, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg );
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
			Serial.printf("# %cb [%s] oldValue: %d newValue: %d\n", (char)(vData), mpistring(M), sd, sd2);
			mbedtls_mpi_free(&M);
			mbedtls_mpi_free(&M2);
		}
		else if(sData == 't') Serial.printf("# txCount = %d\n", txNumber);
		else if(sData == 'x') SaveConfig();
		
	}
}

// needed for first flash & after any flash clear via left+right buttons
// serials come from https://resource.heltec.cn/search
// use product id from ie: ESP32ChipID=5C9482697090
//uint32_t license[4] = { 0xBF91E8F9,0xA26B051E,0xA310D34A,0x9316739B }; // ACM1

void setup()
{
	mutex = xSemaphoreCreateMutex();
	attachInterrupt(0,interrupt_GPIO0,RISING);
	Serial.setRxBufferSize(8192);
	Serial.begin(115200);
	//Mcu.setlicense(license, HELTEC_BOARD);
	chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	while(!Serial) continue;

	MathInit();

	Serial.println("# Mounting LittleFS ...");

  // Initialize LittleFS
  if (!LittleFS.begin(true)) { // 'true' formats the filesystem if the mount fails
    Serial.println("# Failed to mount LittleFS filesystem");
    return;
  }
  Serial.println("# LittleFS mounted successfully");

	const char* cfile = "/hel_config";
	File cf = LittleFS.open(cfile, "r");
	if(!cf) {
		Serial.println("# No Existing Config Found");
	} else {
		Serial.println("# Reading /hel_config");
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
				Serial.println("# Malformed Save File");
				break;
			}
		}
		cf.close();
	}

	SaveConfig();

	VextON();
	delay(100);
	//factory_display.init();
	//factory_display.clear();
	//factory_display.setFont(ArialMT_Plain_10);
  //factory_display.setTextAlignment(TEXT_ALIGN_LEFT);
	//factory_display.display();

	u8g2.initDisplay();
	u8g2.begin();
	u8g2.setPowerSave(0);
	u8g2.enableUTF8Print();
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_boutique_bitmap_7x7_t_all); 
	u8g2.sendBuffer();

	delay(100);

	Serial.printf("# ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
	delay(100);

	lora_init();
  delay(100);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);  
	VextOFF();
	Radio.Rx(0);
}

int idlemod = 1;
void SendToRadio(const char* txt) {
	//xSemaphoreTake(mutex, 400);
	lora_idle = false;
	++txNumber;
	//xSemaphoreGive(mutex);
  if(txt == NULL) {
		memset(txpacket, 0, sizeof(txpacket));
		uint32_t r = Radio.Random();
		sprintf(txpacket,"失%u呂%s例%u子水%s約%u",txNumber, Handle, r, Version, rxNumber);
	}
	else if(txt != txpacket) {
		memset(txpacket, 0, sizeof(txpacket));	
		strcpy(txpacket, txt);
	}
  Radio.Send((uint8_t *)txpacket, strlen(txpacket));
	Serial.printf(": %s\n", txpacket);
	idlemod=1;
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
					last_tx = millis();
					//vTaskDelay(pdMS_TO_TICKS(100));
					index = 0;					
				} else txpacket[index++] = sData;
			}
  	} else if (millis() - last_tx > 55555 && lora_idle) {
			SendToRadio(NULL);
			last_tx = millis();
		}
		delay(10);
	} 
	if((millis() - last_tx) > (1551*idlemod)) {
		idlemod++;
		Radio.Send(NULL, NULL);
	}
	if(ScreenOn && (millis() - last_refresh) > 911) {
		xSemaphoreTake(mutex, 400);
		if(last_refresh == 0) vTaskDelay(pdMS_TO_TICKS(100));
		u8g2.clearBuffer();
		for(int j = 0; j < last_line; j++) u8g2.drawUTF8(1, 13+(10*j), screenlines[j]);
		u8g2.sendBuffer();
		last_refresh = millis();
		xSemaphoreGive(mutex);
	}
	if(!ScreenOn && last_refresh != 0) last_refresh = millis();
  delay(20);    
}
