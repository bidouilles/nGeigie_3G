/*
  3D Tabrain shield version of nGeigie with LCD display

Connection:
 * An Arduino Ethernet Shield
 * D2: The output pin of the Geiger counter (active low)
 * D3: The output pin of the Geiger counter (active low)
 based on NetRad 1.1.2

 before work done by Allan Lind, Lionel and Kalin of Safecast
 This code is in the public domain.
 */
 
/**************************************************************************/
// Init
/**************************************************************************/


#include <SoftwareSerial.h>
#include <Wire.h>       
#include <LiquidCrystal.h>
#include "a3gs.h"
#include <avr/wdt.h>
#include <limits.h>


  #define LINE_SZ 95
  //#define PIN1 17 //analog 2 for interrupt for dual tube
  
  static char path[LINE_SZ];
  static char json_buf[LINE_SZ];
  static char buf[LINE_SZ];
  static char VERSION[] = "2.2.3_3G";
  const char *server = "107.161.164.166";
  const int port = 80;
  const int interruptMode = RISING;
  const int updateIntervalInMinutes = 1;
  const char *apiKey = "q1LKu7RQ8s5pmyxunnDW";
  const char *lat = "34.4772923" ;
  const char *lon = "136.1473367" ;
  const char *ID = "47" ;
  char res[a3gsMAX_RESULT_LENGTH+1];
  //char logfile_name[13]; 
  //bool logfile_ready = false;
  int len;
  unsigned long elapsedTime(unsigned long startTime);
  unsigned long int uSv ;
  
  int freeRAM ()
	{
		extern int __heap_start, *__brkval;
		int v;
		return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	}

    
   //Setup sdcard from openlog for serial2
	   SoftwareSerial sdSerial =  SoftwareSerial(10, 13); 

	// generate checksum for log format
		byte len1, chk;     
		char checksum(char *s, int N)
			{
			  int i = 0;
			  char chk = s[0];

			  for (i=1 ; i < N ; i++)
				chk ^= s[i];

			  return chk;
			}
	// initialize the library with the numbers of the interface pins
	   LiquidCrystal lcd(14,15,8,9,11,12);
		 
	// pins used by 3G shield D2, D4,D5,D6,D7 needs to be avoided. 8,9,10,12,13,14,15, can be used. Check D10 for SD card working or not

	// Sampling interval (e.g. 60,000ms = 1min)
		unsigned long updateIntervalInMillis = 0;

	// The next time to feed
		unsigned long nextExecuteMillis = 0;

	// Event flag signals when a geiger event has occurred
		volatile unsigned char eventFlag = 0;		// FIXME: Can we get rid of eventFlag and use counts>0?
		int counts_per_sample;

	// The last connection time to disconnect from the server
	// after uploaded feeds
		long lastConnectionTime = 0;

	// The conversion coefficient from cpm to µSv/h
		float conversionCoefficient = 0;
		
		
		
void onPulse()
	{
	counts_per_sample++;
	eventFlag = 1;
	}
	
/**************************************************************************/
// Setup
/**************************************************************************/
 


void setup()
	{
	  wdt_reset();	
	  Serial.begin(9600);
	  sdSerial.begin(9600);
	  Serial.println(VERSION);
	  //Free ram print
			  Serial.print(F("Free :\t"));
			  Serial.println(freeRAM());
  
		lcd.begin(8, 2);
		lcd.clear();
		lcd.print(F("nGeigie"));
		lcd.setCursor(0, 1);
		lcd.print(VERSION);
	    delay(3000);
		// Set the conversion coefficient from cpm to µSv/h
		// LND_7318:
				Serial.println(F("Sensor Model: LND 7317"));
				//Serial.println(F("Conversion factor: 344 cpm = 1 uSv/Hr"));
				conversionCoefficient = 0.0029;

		// LND_712:
				//conversionCoefficient = 0.0083;
			    //Serial.println(F("Sensor model:   LND 712"));
			        
  
	  //get time
	  Serial.print(F("Get time. "));
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Getting");
		lcd.setCursor(0, 1);
		lcd.print("time...");
		delay(3000);
		  if (a3gs.start() == 0 && a3gs.begin() == 0) 
			  {
				Serial.println(F("OK"));
				char date[a3gsDATE_SIZE], time[a3gsTIME_SIZE];
				if (a3gs.getTime(date, time) == 0) 
					{
					  Serial.print(date);
					  Serial.println(time);
					  lcd.setCursor(0, 0);
					  lcd.print(date);
					  lcd.setCursor(0, 1);
					  lcd.print(time);
					}
		  else
			Serial.println(F("Failed time."));
		   }
        
        //Interrupts setup .. numbers 0 (on digital pin 2) and 1 (on digital pin 3)
			//attachInterrupt(0, onPulse, interruptMode);                          
			attachInterrupt(1, onPulse, interruptMode); 
			
			
		//pinMode(PIN1, INPUT); digitalWrite(PIN1, HIGH);
        //PCintPort::attachInterrupt(PIN1, onPulse, FALLING);                                
		updateIntervalInMillis = updateIntervalInMinutes * 60000;                  // update time in ms
		unsigned long now = millis();
		nextExecuteMillis = now + updateIntervalInMillis;

		
		
}
	
	
//**************************************************************************/
// send data 
//**************************************************************************/

void SendDataToServer(float CPM) {  

// Convert from cpm to µSv/h with the pre-defined coefficient


	float uSv = CPM * conversionCoefficient;                   // convert CPM to Micro Sieverts Per Hour
    char CPM_string[16];
	dtostrf(CPM, 0, 0, CPM_string);
	
	//delay (6000);  // Wait for Start Serial Monitor
	Serial.println(CPM_string);
	Serial.print(F("Initializing.. "));
  
	//display geiger info
	lcd.clear();
	lcd.setCursor(0, 0);
    lcd.print(uSv);
    lcd.print(F("uS/H"));

  if (a3gs.start() == 0 && a3gs.begin() == 0) {
    Serial.println(F("Succeeded."));
    len = sizeof(res);

    sprintf_P(path, PSTR("/scripts/short.php?api_key=%s&lat=%s&lon=%s&cpm=%s&id=%s"),
      apiKey,
      lat, \
      lon, \
      CPM_string, \
      ID) ;
 

    //SD card write prepare
		memset(buf, 0, LINE_SZ);
		sprintf_P(buf, PSTR("$BNRDD,%s,,,,%s,,,,%s,,%s,,,,,,"),  \
				  ID, \
				  CPM_string, \
				  lat,\
				  lon);
				  
		 len = strlen(buf);
		  buf[len] = '\0';

		  // generate checksum
		  chk = checksum(buf+1, len);

		  // add checksum to end of line before sending
		  if (chk < 16)
			sprintf_P(buf + len, PSTR("*0%X"), (int)chk);
		  else
			sprintf_P(buf + len, PSTR("*%X"), (int)chk);

	//write to sd card
       sdSerial.println(buf);
	   //Serial.println(buf);
	 
	 
	 //send to server
	 if (a3gs.httpGET(server, port, path, res, len) == 0) {
      Serial.println(F("Send OK!"));
  		lcd.setCursor(0, 1);
      	lcd.print(F("Send OK"));    
	    }
    else {
      Serial.print(F("No HTTP"));
  	  lcd.setCursor(0, 1);
      lcd.print(F("no HTTP"));
      Serial.println(server);
    }
  }
  else
  		//clean out the buffers
       memset(buf, 0, sizeof(buf));
       memset(path, 0, sizeof(path));
	  a3gs.end();
	  a3gs.shutdown();
}

	
/**************************************************************************/
// Main Loop
/**************************************************************************/


void loop() {                                             
	// Main Loop

	if (elapsedTime(lastConnectionTime) < updateIntervalInMillis)
	{
		return;
	}

	float CPM = (float)counts_per_sample / (float)updateIntervalInMinutes;
	counts_per_sample = 0;

        SendDataToServer(CPM);
}

/**************************************************************************/
// calculate elapsed time, taking into account rollovers
/**************************************************************************/
unsigned long elapsedTime(unsigned long startTime)
{
	unsigned long stopTime = millis();

	if (startTime >= stopTime)
	{
		return startTime - stopTime;
	}
	else
	{
		return (ULONG_MAX - (startTime - stopTime));
	}
}
