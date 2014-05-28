// NOTE:
// Before uploading to your Arduino Ethernet board,
// please replace with your own settings

#define MAX_FAILED_CONNS 1


// Your API key (a public secure key is recommended) IWAKI HAWK 002
const char *apiKey = "q1LKu7RQ8s5pmyxunnDW";
const char *lat = "35.6617" ;
const char *lon = "139.7447" ;
const char *ID = "47" ;


// This is specific to the Uno Ethernet board
int pinSpkr = 1;	// pin number of piezo speaker
int pinLED = 0;		// pin number of event LED
int resetPin = A1;   //

// Update interval in minutes
const int updateIntervalInMinutes = 1;

typedef struct
{
  unsigned short addr;
  char lat[10];
  char lon[10];
  char ID[3];
  char firmware_version[8];
} device_t;

typedef struct
{
    unsigned char state;
    unsigned char conn_fail_cnt;
} devctrl_t;

enum states
{
    NORMAL = 0,
    RESET = 1
};


// Interrupt mode:
// * For most geiger counter modules: FALLING
// * Geiger Counter Twig by Seeed Studio: RISING
const int interruptMode = RISING;


