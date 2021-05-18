/*
  esp_ntp_2_serial.ino
  ESP8266: Sending NTP time over hardware Serial (Serial1 or Serial2 (ESP32))
  weigu.lu

  ConfigTime is only settings. The real work is done by the time()
  function (esp8266 core). Time() always returns an internal time counter.
  When the delay since prevous sync is reached (60 minutes by default), time()
  re-synchronises its internal counter with NTP server on the internet.

  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
  -1 = summer one hour behind UTC; -2 = winter two hour behind UTC
  M3.5.0: changes M3 = 3 month (march) 5 = 5 week (last week of march) 0 = sunday 2h00

  Andreas Spiess Video 299: https://youtu.be/r2UAmBLBBRM 
  https://werner.rothschopf.net/201802_arduino_esp8266_ntp.htm
*/

#ifdef ESP8266   // all includes are from Arduino Core, no external lib
  #include <ESP8266WiFi.h>       
  // <time.h> and <WiFiUdp.h> not needed. already included by core.         
#else
  #include <WiFi.h>         
  #include <time.h>
#endif
#include <coredecls.h>  // ! optional settimeofday_cb() callback to check on server

// The file "secrets.h" has to be placed in the sketchbook libraries folder
// in a folder named "Secrets" and must contain your secrets e.g.:
// const char *MY_WIFI_SSID = "mySSID"; const char *MY_WIFI_PASSWORD = "myPASS";
#define USE_SECRETS
#define DEBUG
#define HSERIAL Serial1 

/****** WiFi and network settings ******/
#ifdef USE_SECRETS
  #include <secrets.h>
  const char *WIFI_SSID = MY_WIFI_SSID;             
  const char *WIFI_PASSWORD = MY_WIFI_PASSWORD;     // password
#else
  const char* *WIFI_SSID = mySSID;         // if no secrets file, add your SSID here
  const char* *WIFI_PASSWORD = myPASSWORD; // if no secrets file, add your PASS here
#endif

/****** NTP settings ******/
const char *NTP_SERVER = "lu.pool.ntp.org";
// your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const char *TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

time_t now = 0;
tm timeinfo;                      // time structure

struct My_Timeinfo {
  byte second;
  byte minute;
  byte hour;
  byte day;
  byte month;
  unsigned int year;
  byte weekday;
  unsigned int yearday;
  bool daylight_saving_flag;
  String name_of_day;
  String name_of_month;
  String date;
  String time;
  String datetime;
} my;

// functions declarations (prototypes)
void init_ntp_time();             // init NTP time: call this before the WiFi connect! 
void get_time();                  // epoch to tm structure and update global struct
void print_my_time_struct();      // print my global time structure

// ! optional  callback function to check if NTP server called
void time_is_set(bool from_sntp /* <= this parameter is optional */) {  
  Serial.println("The NTP server was called!");
}

// ! optional change here if you want another NTP polling interval (default 1h)
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 () {
  return 4 * 60 * 60 * 1000UL; // every 4 hours
}

void setup() {
#ifdef   DEBUG
  Serial.begin(115200);  
  delay(1000);
  Serial.println("\nHello");
#endif    
  HSERIAL.begin(115200);
  settimeofday_cb(time_is_set); // ! optional  callback function to check if NTP server called
  init_ntp_time();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  #ifdef   DEBUG
    Serial.print ( "." );
  #endif  
  }  
  get_time();
  #ifdef   DEBUG  
    Serial.println("\nConnected to SSID " + WiFi.SSID() + " with IP " + 
                    WiFi.localIP().toString() + "\nSignal strength is " +
                    WiFi.RSSI() + " dBm\n");
    Serial.println("Epoch time: " + String(now));
    Serial.println("Setup done!");
  #endif  
}

void loop() {  
  get_time();
#ifdef   DEBUG
  //print_my_time_struct();
  Serial.println(" " + my.datetime);
#endif  
  HSERIAL.print('T' + now + '\n');  
  delay(10000);
}

// init NTP time: call this before the WiFi connect!
void init_ntp_time() { 
  #ifdef ESP8266
    configTime(TZ_INFO, NTP_SERVER);    
  #else  
    configTime(0, 0, NTP_SERVER); // 0, 0 because we will use TZ in the next line
    setenv("TZ", TZ_INFO, 1);     // set environment variable with your time zone
    tzset();
  #endif
}
 
// epoch to tm structure and update global struct
void get_time() {
  time(&now);                     // this function calls the NTP server only every hour
  localtime_r(&now, &timeinfo);   // converts epoch time to tm structure
  my.second  = timeinfo.tm_sec;
  my.minute  = timeinfo.tm_min;
  my.hour  = timeinfo.tm_hour;
  my.day  = timeinfo.tm_mday;
  my.month  = timeinfo.tm_mon + 1;    // beer (Andreas video)
  my.year  = timeinfo.tm_year + 1900; // beer
  my.weekday = timeinfo.tm_wday; 
  if (my.weekday == 0) {              // beer
    my.weekday = 7;
  }
  my.yearday = timeinfo.tm_yday + 1;  // beer
  my.daylight_saving_flag  = timeinfo.tm_isdst;
  char buffer[25];  
  strftime(buffer, 25, "%A", localtime(&now));
  my.name_of_day = String(buffer);
  strftime(buffer, 25, "%B", localtime(&now));
  my.name_of_month = String(buffer);
  strftime(buffer, 25, "20%y-%m-%d", localtime(&now));
  my.date = String(buffer);
  strftime(buffer, 25, "%H:%M:%S", localtime(&now));
  my.time = String(buffer);  
  strftime(buffer, 25, "20%y-%m-%dT%H:%M:%S", localtime(&now));
  my.datetime = String(buffer);  
}

// print my global time structure
void print_my_time_struct() {
  Serial.println("\nmy.second: " + String(my.second));
  Serial.println("my.minute: " + String(my.minute));
  Serial.println("my.hour: " + String(my.hour));
  Serial.println("my.day: " + String(my.day));
  Serial.println("my.month: " + String(my.month));
  Serial.println("my.year: " + String(my.year));
  Serial.println("my.weekday: " + String(my.weekday));
  Serial.println("my.yearday: " + String(my.yearday));
  Serial.println("my.daylight_saving_flag: " + String(my.daylight_saving_flag));
  Serial.println("my.name_of_day: " + my.name_of_day);
  Serial.println("my.name_of_month: " + my.name_of_month);
  Serial.println("my.date: " + my.date);
  Serial.println("my.time: " + my.time);  
  Serial.println("my.datetime: " + my.datetime);
}
