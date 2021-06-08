/*  
 *   Pager_OS             Version 1.0.0               Datum 08.06.2021
 *   
 *   Lizensierung:
 *   Namensnennung - Nicht-kommerziell - Weitergabe unter gleichen Bedingungen (CC BY-NC-SA)
 *   https://creativecommons.org/licenses/?lang=de
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "cubecell_SSD1306Wire.h"
//#include "cubecell_SH1107Wire.h"

#define Taster GPIO7  // Eingang für Taster
#define TON GPIO5     // Ausgang für Piepser
#define LED GPIO6     // Ausgang für LED


extern SSD1306Wire  display;  /* CubeCell-GPS (HTCC-AB02S) */

//extern SH1107Wire  display;   /* CubeCell-Board-Plus (HTCC-AB02) */

uint8_t i;
int taster;
char empfangener_Text[100];

/* OTAA para*/
// "pager-mit-großem-display" TTN-Daten
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x4B, 0x00, 0x00, 0x00, 0x04 }; // Fill me in
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x6C, 0x2A, 0x7E, 0xD6, 0x60, 0x37, 0x9F, 0x8F, 0x4C, 0xED, 0x83, 0x58, 0xCB, 0x49, 0x73, 0xAC };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 }; // Fill me in
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 1200000;  // meldet sich all 1,2 Mio Sekunden (20min)

/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;

/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = LORAWAN_UPLINKMODE;

/* Application port */
uint8_t appPort = 2;

/* Number of trials to transmit the frame */
uint8_t confirmedNbTrials = 4;


//downlink data handle function example
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
  Serial.printf("+REV DATA:%s,EmpfangeneBytes %d,PORT %d\r\n",mcpsIndication->RxSlot?"Empfangsfenster2":"Empfangsfenster1",mcpsIndication->BufferSize,mcpsIndication->Port);
  Serial.println("Empfangener Text:"); // Der empfangene Text wird im seriellen Monitor ausgegeben.

  // Variable für die einzelnen Stellen vom char-Array.
  int n = 0;
  int n1 = 0;
  int n2 = 0;
  int n3 = 0;
  
  /* Für jede Zeile auf dem Display wird ein char-Array initialisiert, mit "laenge"-Stellen */
  int laenge = 17;
  char Zeile[laenge];
  char Zeile1[laenge];
  char Zeile2[laenge];
  char Zeile3[laenge];

  /* Payload-Auswertung */
  for(i=0;i<mcpsIndication->BufferSize;i++)
  {
    int x = ("%02X",mcpsIndication->Buffer[i]);   // Der Variable x wird der empfangene Wert zugewiesen.
    empfangener_Text[i] = x; // Der empfangene Text wird auf vier Zeilen aufgeteilt.
    
    if(i < laenge){
      Zeile[n] = x;
      n++;
    }
    else if(i < laenge*2){
      Zeile1[n1] = x;
      n1++;
    }
    else if(i < laenge*3){
      Zeile2[n2] = x;
      n2++;
    }
    else if(i < laenge*4){
      Zeile3[n3] = x;
      n3++;
    }
  }
  digitalWrite(LED, HIGH); // Sobald ein Text eingeht wird der LED-Pin auf "HIGH" gesetzt.

/* Die einzelnen Zeichenketten werden nullterminiert */
  empfangener_Text[i] = 0;
  Zeile[n] = 0;
  Zeile1[n1] = 0;
  Zeile2[n2] = 0;
  Zeile3[n3] = 0;
/****************************************************/
  // Hier wird der Text über die serielle Schnittstelle ausgegeben.
  Serial.println();
  Serial.println(String(Zeile));
  Serial.println(String(Zeile1));
  Serial.println(String(Zeile2));
  Serial.println(String(Zeile3));
  Serial.println();

/* Display Konfiguration: */
  display.init();       // Das Display wird initiallisiert, sonst ist das Display schwarz (aus).
  display.clear();      // Die Anzeige wird gelöscht/gesäubert.
/* 
 *Die Schriftgröße kann hier angepasst werden.
 *Verfügbare Größen 10, 16, 24
 *Die Schriftgröße 10 ist die Standartgröße, daher kann der Befehl auch weggelassen werden.
 */
  display.setFont(ArialMT_Plain_16); // Hier wird die Schriftart und Schriftgröße bestimmt.
  display.drawString(0, 0, String(Zeile));
  display.drawString(0, 15, String(Zeile1));
  display.drawString(0, 30, String(Zeile2));
  display.drawString(0, 45, String(Zeile3));
  display.display();  // Die Änderungen werden auf die Anzeige übertragen.
/***************************************************/
/* Umwandlung von einem char Array in einen String */
  String Pager = String(empfangener_Text);
/***************************************************
 *Hier wird im dem empfangenen Text das unten eingetragenen Zeichen (@) gesucht. Es können auch Zeichenfolgen (Wörter) festgelegt werden.
 *Beachtet wird folgendes:
 *  - genaue Zeichenfolge
 *  - Groß- und Kleinschreibung
 *Befindet sich das Schlagwort nicht im Payload, wird der Wert "-1" zurück gegeben.
 */
  if(Pager.indexOf("@") != -1){
    blinkzyklus(80, 50, 1, 10);
  }
/***************************************************/  
  uint32_t color=mcpsIndication->Buffer[0]<<16|mcpsIndication->Buffer[1]<<8|mcpsIndication->Buffer[2];
#if(LoraWan_RGB==1)
  turnOnRGB(color,5000);
  turnOffRGB();
#endif
}

static void prepareTxFrame( uint8_t port )
{
  uint16_t batteryVoltage = getBatteryVoltage();

/*******************************************************************************/
/* Hier werden nach dem oben eingestellten dutycycle die Werte der Batteriespannung in mV und des Tasters (h/l) übertragen.
 * Die Daten werden in das Array "appData" gepackt, danach in HEX verschickt. 
 */
  appDataSize = 3;
    appData[0] = (uint8_t)(batteryVoltage>>8);
    appData[1] = (uint8_t)batteryVoltage;
    appData[2] = (uint8_t)taster;
    Serial.print("Batteriespannung in mV: "); // Hier wird über den seriellen Monitor die Batteriespannung ausgegeben.
    Serial.print(batteryVoltage);
    Serial.println(" mV");
    taster = 0;
/*******************************************************************************/
}

void setup() {
	boardInitMcu();
	Serial.begin(9600);
// Hier wird festgelegt ob es sich bei dem GPIO-PIN, um einen Eingang oder Ausgang handelt.
  pinMode(Taster, INPUT);   // GPIO7
  pinMode(LED, OUTPUT);     // GPIO6
  pinMode(TON, OUTPUT);     // GPIO5

#if(AT_SUPPORT)
	enableAt();
#endif
	deviceState = DEVICE_STATE_INIT;
	LoRaWAN.ifskipjoin();
}

void loop()
{
/* Hier wird mit der Betätigung des Tasters die erhaltene Nachricht bestätigt und anschließend gelöscht. */
  if(digitalRead(Taster) == LOW){
    display.init();
    display.clear();
    display.display();
    taster = 1;
    digitalWrite(LED, LOW);
  }
/********************************************************************************************************/
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(AT_SUPPORT)
			getDevParam();
#endif
			printDevParam();
			LoRaWAN.init(loraWanClass,loraWanRegion);
			deviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			prepareTxFrame( appPort );
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
			LoRaWAN.sleep();
			break;
		}
		default:
		{
			deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}

void blinkzyklus(int low, int high, int ton, int timer)
{
/* In dieser Funktion wird der Blinkzyklus bearbeitet.
 * Der Blinkzyklus wird in zwei "for"-Schleifen realisiert.
 * Die erste "for-Schleife ist für den Zustand "LED LOW" zuständig und die zweite für "LED HIGH".
 * Wie oft die "for"-Schleifen durchlaufen, hängt von den übergebenen Parametern "blinkzyklus(80, 50, 1, 10)" ab.
 * Die einzelnen Parameter, für die speziellen Eigenschaften, werden oben im Programm initialisiert.
 * Parameter:
 *  - low, high => Anzahl der Durchläufe der Schleifen.
 *  - ton       => Bei ton = 1 wird ein Ton beim Blinken wiedergegeben.
 *  - timer     => Die Dauer vom "delay". 
 */
  while(digitalRead(Taster) == HIGH){
    for(int off = 0; off<low; off++){
      digitalWrite(LED, LOW);
      digitalWrite(TON, LOW);
      delay(timer);
      if(digitalRead(Taster) == LOW){;
        ton = 0;
        break;
      }
    }
    for(int on = 0; on<high; on++){
      digitalWrite(LED, HIGH);
      if(ton==1){
        digitalWrite(TON, HIGH);
      }
      delay(timer);
      if(digitalRead(Taster) == LOW){;
        digitalWrite(LED, LOW);
        digitalWrite(TON, LOW);
        ton = 0;
        break;
      }
    }
  }
}
