/*
   Simple library to handle ngeigie configuration from file and EEPROM
*/

#ifndef _NGEIGIE_SETUP_H
#define _NGEIGIE_SETUP_H

// Link to arduino library
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define BMRDD_EEPROM_SETUP 500
#define BMRDD_EEPROM_MARKER 0x5afeF00d

typedef enum {
  SENSOR_ENABLED_FALSE = 0,
  SENSOR_ENABLED_TRUE,
} SensorEnabled;

typedef struct {
  unsigned long marker;     // set at first run
  char user_name[16];       // nm 
  unsigned int user_id;     // uid
  char api_key[24];         // api
  char latitude[16];        // lat
  char longitude[16];       // lon
  byte sensor1_enabled;     // s1e
  float sensor1_cpm_factor; // s1f
  byte sensor2_enabled;     // s2e
  float sensor2_cpm_factor; // s2f
} ConfigType;

// Write a template value into EEPROM address [ee]
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

// Read a template value from EEPROM address [ee]
template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

class nGeigieSetup {

public:
  nGeigieSetup(SoftwareSerial &openlog, 
        ConfigType &config,
        char * buffer, size_t buffer_size);
  void initialize();
  void loadFromFile(char * setupFile);

private:
  SoftwareSerial &mOpenlog;
  ConfigType &mConfig;

  char * mBuffer;
  size_t mBufferSize;
};

#endif
