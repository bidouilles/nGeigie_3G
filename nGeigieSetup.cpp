/*
   Simple library to handle ngeigie configuration from file and EEPROM
*/

#include "nGeigieSetup.h"
#include "nGeigieDebug.h"

nGeigieSetup::nGeigieSetup(SoftwareSerial &openlog, ConfigType &config,
    char * buffer, size_t buffer_size):
    mOpenlog(openlog), mConfig(config), mBuffer(buffer), mBufferSize(buffer_size) {
}

void nGeigieSetup::initialize() {
  // Configuration 
  DEBUG_PRINTLN("Loading EEPROM configuration");
  memset(&mConfig, 0, sizeof(mConfig));
  EEPROM_readAnything(BMRDD_EEPROM_SETUP, mConfig);

  if (mConfig.marker != BMRDD_EEPROM_MARKER) {
    DEBUG_PRINTLN("  - First time setup");
    // First run, time to set default values
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.marker = BMRDD_EEPROM_MARKER;
    EEPROM_writeAnything(BMRDD_EEPROM_SETUP, mConfig);

#if ENABLE_EEPROM_DOSE
    memset(&mDose, 0, sizeof(mDose));
    EEPROM_writeAnything(BMRDD_EEPROM_DOSE, mDose);
#endif
  }
}

void nGeigieSetup::loadFromFile(char * setupFile) {
  bool config_changed = false;
  char *config_buffer, *key, *value;
  byte pos, line_lenght;
  byte i, buffer_lenght;

  mOpenlog.listen();

  // Send read command to OpenLog
  DEBUG_PRINT(" - read ");
  DEBUG_PRINTLN(setupFile);

  sprintf_P(mBuffer, PSTR("read %s 0 %d"), setupFile, mBufferSize);
  mOpenlog.print(mBuffer);
  mOpenlog.write(13); //This is \r

  while(1) {
    if(mOpenlog.available())
      if(mOpenlog.read() == '\r') break;
  }

  // Read config file in memory
  pos = 0;
  memset(mBuffer, 0, mBufferSize);
  for(int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    if(mOpenlog.available()) {
      mBuffer[pos++] = mOpenlog.read();
      timeOut = 0;
    }
    delay(1);

    if(pos == mBufferSize) {
      break;
    }
  }

  line_lenght = pos;
  pos = 0;
  
  // Process each config file lines
  while(pos < line_lenght){

    // Get a complete line
    i = 0;
    config_buffer = mBuffer + pos;
    while(mBuffer[pos++] != '\n') {
      i++;
      if(pos == mBufferSize) {
        break;
      }
    }
    buffer_lenght = i++;
    config_buffer[--i] = '\0';

    // Skip empty lines
    if(config_buffer[0] == '\0' || config_buffer[0] == '#' || buffer_lenght < 3) continue;

    // Search for keys
    i = 0;
    while(config_buffer[i] == ' ' || config_buffer[i] == '\t') {
      if(++i == buffer_lenght) break; // skip white spaces
    }
    if(i == buffer_lenght) continue;
    key = &config_buffer[i];

    // Search for '=' ignoring white spaces
    while(config_buffer[i] != '=') {
      if(config_buffer[i] == ' ' || config_buffer[i] == '\t') config_buffer[i] = '\0';
      if(++i == buffer_lenght) {
        break;
      }
    }
    if(i == buffer_lenght) continue;
    config_buffer[i++] = '\0';

    // Search for value ignoring white spaces
    while(config_buffer[i] == ' ' || config_buffer[i] == '\t') {
      if(++i == buffer_lenght) {
        break;
      }
    }
    if(i == buffer_lenght) continue;
    value = &config_buffer[i];
    
    //
    // Process matching keys
    //
    if(strcmp(key, "s1f") == 0) {
      float factor = atoi(value);
      if (mConfig.sensor1_cpm_factor != factor) {
        mConfig.sensor1_cpm_factor = factor;
        config_changed = true;
        DEBUG_PRINTLN("   - Update sensor1_cpm_factor");
      }
    }
    else if(strcmp(key, "s2f") == 0) {
      float factor = atoi(value);
      if (mConfig.sensor2_cpm_factor != factor) {
        mConfig.sensor2_cpm_factor = factor;
        config_changed = true;
        DEBUG_PRINTLN("   - Update sensor2_cpm_factor");
      }
    }
    else if(strcmp(key, "nm") == 0) {
      if (strcmp(mConfig.user_name, value) != 0 ) {
        strcpy(mConfig.user_name, value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update user_name");
      }
    }
    else if(strcmp(key, "uid") == 0) {
      if (mConfig.user_id != (unsigned int)atoi(value)) {
        mConfig.user_id = atoi(value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update user_id");
      }
    }
    else if(strcmp(key, "api") == 0) {
      if (strcmp(mConfig.api_key, value) != 0 ) {
        strcpy(mConfig.api_key, value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update api_key");
      }
    }
    else if(strcmp(key, "s1e") == 0) {
      if (mConfig.sensor1_enabled != atoi(value)) {
        mConfig.sensor1_enabled = atoi(value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update sensor1_enabled flag");
      }
    }
    else if(strcmp(key, "s2e") == 0) {
      if (mConfig.sensor2_enabled != atoi(value)) {
        mConfig.sensor2_enabled = atoi(value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update sensor2_enabled flag");
      }
    }
    else if(strcmp(key, "lat") == 0) {
      if (strcmp(mConfig.latitude, value) != 0 ) {
        strcpy(mConfig.latitude, value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update latitude");
      }
    }
    else if(strcmp(key, "lon") == 0) {
      if (strcmp(mConfig.longitude, value) != 0 ) {
        strcpy(mConfig.longitude, value);
        config_changed = true;
        DEBUG_PRINTLN("   - Update longitude");
      }
    }
  }
  DEBUG_PRINTLN("   - Done.");

  if (config_changed) {
    // Configuration is changed
    DEBUG_PRINTLN("Update configuration in EEPROM");
    EEPROM_writeAnything(BMRDD_EEPROM_SETUP, mConfig);
  }
}
