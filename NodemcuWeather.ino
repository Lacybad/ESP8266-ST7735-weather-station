
//Used Library
#include <NTPClient.h>                        // Date/Time manager 
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>                          // Library to send and receive UDP messages
#include <WiFiClient.h>                       // driver for WiFi client
#include <ArduinoJson.h>                      // Arduino Json to parse reauest into JSON object. Installed version 5.13, last version is not compatible.
#include <Adafruit_GFX.h>    // LCD graphical driver
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

#define FS_NO_GLOBALS
#include <FS.h>

#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFC9F


// ----------------------------------------------------------------------------------------

// wifi sid and password (hardcoded for the moment)
// @TODO: MAKE ssid and password NOT hardcoded?
#ifndef APSSID
#define APSSID "************" //Network name
#define APPSK  "************" //password
#endif
/* Set these to your desired credentials. */
char *ssid = APSSID;
char *password = APPSK;

// ----------------------------------------------------------------------------------------

// APIKEY is a passtoken used to identify the request from data to "pool.ntp.org
String APIKEY = "*******************************"; // API Key 
// @TODO: Make CityID configurable
String CityID = "*********"; // 
// @TODO: Make Timezone configurable
int TimeZone = 1;// GMT +1
// @TODO: Make utcOffsetInSeconds configurable
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Define NTP Client to get time and date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

WiFiClient client;    // WIFI Client
char servername[]="api.openweathermap.org";    // remote server we will connect to
String result;

// ----------------------------------------------------------------------------------------
// set of variables used into this sketch for different pourpose
boolean   night = false;
String    timeS = "";
String    day = "";
int       weatherID = 0;
String    location = "";
String    temperature = "";
String    weather = "";
String    description = "";
String    idString = "";
String    umidityPer = "";
float     Fltemperature = 0;
int       counter = 30;
String    windS = "";
String    pressure = "";
String    Visibility ="";
String    Wind_angle = "";
String    Clouds = "";


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define ST7735_DRIVER

//#define ST7735_INITB
//#define ST7735_GREENTAB
//#define ST7735_GREENTAB2
//#define ST7735_GREENTAB3
//#define ST7735_GREENTAB128 // For 128 x 128 display
#define ST7735_REDTAB
//#define ST7735_BLACKTAB



#define TFT_WIDTH  128
#define TFT_HEIGHT 160


// Display SDO/MISO  to NodeMCU pin D6 (or leave disconnected if not reading TFT)
// Display LED       to NodeMCU pin VIN (or 5V, see below)
// Display SCK       to NodeMCU pin D5
// Display SDI/MOSI  to NodeMCU pin D7
// Display DC (RS/AO)to NodeMCU pin D3
// Display RESET     to NodeMCU pin D4 (or RST, see below)
// Display CS        to NodeMCU pin D8 (or GND, see below)
// Display GND       to NodeMCU pin GND (0V)
// Display VCC       to NodeMCU 5V or 3.3V



// =======================================================================================
// S E T U P
// =======================================================================================
void setup() {
  Serial.begin(115200);

  
  tft.init();
  tft.setRotation(0);  // portrait 
 
  
  tft.fillScreen(TFT_WHITE);   
  if (!SPIFFS.begin()) {  //
    Serial.println("SPIFFS initialize error!");
   while (1) yield(); // Wait till initialize
  } 
  // wifi connection
  drawWifi();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ){
    delay(500);
    Serial.print(".");
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(5,110);
    tft.print ( "Try connect to: " );
    tft.setCursor(5,120);
    tft.print ( ssid );
  }
  IPAddress myIP = WiFi.localIP();
  tft.setCursor(5,130);
  tft.print ( "Connected!" );
  tft.setCursor(5,140);
  tft.print ( myIP );
  // client to get date and time
  // @TODO: implement better this step with https://github.com/scanlime/esp8266-Arduino/blob/master/tests/Time/Time.ino
  timeClient.begin();
  delay (2000);
}

// =======================================================================================
// L O O P
// =======================================================================================
void loop() {
  
  if(counter == 30) {//Get new data every 30 cycles (1 cycles = 60 sec)
    counter = 10;
    getWeatherData();
  }else{
    displayData();
    counter++;
    Serial.println(counter); 
  } 
  
  //get current time
  timeS = getTime();
  //get current day
  day = getDay();
  //to define if we are in night or day (to display moon or sun)
  nightOrDay (timeS);
 
}
// =======================================================================================

// get current date
String getDate(){
  timeClient.update(); 
}

// =======================================================================================

// get current time
String getTime(){
  timeClient.update();
  String timeS = timeClient.getFormattedTime();
  int length = timeS.length();
  return timeS.substring(length-8,length-3);
}

// =======================================================================================

// get current day
String getDay(){
  timeClient.update();
  return daysOfTheWeek[timeClient.getDay()];
}

// =======================================================================================

// get Weather data from openweathermap.org
// sent request for data
void getWeatherData(){ //client function to send/receive GET request data. 
  if (client.connect(servername, 80)) {  //starts client connection, checks for connection
    client.println("GET /data/2.5/weather?id="+CityID+"&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  // reading sent data
  while(client.connected() && !client.available()) delay(1); //waits for data
  Serial.println("Waiting for data");
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    result = result+c;
  }

  // replacing character '['
  client.stop(); //stop client
  result.replace('[', ' ');
  result.replace(']', ' ');
  
  Serial.println(result);

  // format received data into a jsonArray.
  // to make this code working it has been becessary to install version 
  char jsonArray [result.length()+1];
  result.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonBuffer<1024> json_buf;
  JsonObject &root = json_buf.parseObject(jsonArray);
  if (!root.success()){
    Serial.println("parseObject() failed");
  }
  
  //TODO : try to understand why this double assignement is necessary
  String temperatureLOC = root["main"]["temp"];
  String weatherLOC = root["weather"]["main"];
  String descriptionLOC = root["weather"]["description"];
  String idStringLOC = root["weather"]["id"];
  String umidityPerLOC = root["main"]["humidity"];
  String windLOC = root["wind"]["speed"];
  String pressureLOC = root["main"]["pressure"];
  String visibilityLOC= root["visibility"];
  String wind_angleLOC = root["wind"]["deg"];
  String cloudsLOC = root ["clouds"]["all"] ;//["main"] 

  temperature = temperatureLOC;
  weather = weatherLOC;
  description = descriptionLOC;
  idString = idStringLOC;
  umidityPer = umidityPerLOC;
  windS = windLOC;
  pressure= pressureLOC;
  Visibility = visibilityLOC;
  Wind_angle = wind_angleLOC;
  Clouds = cloudsLOC;
  
  int length = temperature.length();
  if(length==5){
    temperature.remove(length-3);
  }

  Fltemperature = temperature.toFloat();
  Fltemperature = Fltemperature - 273,15;

 
  
  weatherID = idString.toInt();
}

// =======================================================================================

//Display data on the LCD screen
//Une loop every 60 seconds

void displayData(){
  printGeneral("Mateszalka", timeS, day, weatherID, description, Fltemperature, umidityPer);
  delay (35000);
  //printWeather("Mateszalka", timeS, day, weatherID, description);
  //delay (pause);
  printTemperature("City Name", timeS, day, Fltemperature);
  delay (5000);
  printUmidity("City Name", timeS, day, umidityPer);
  delay (5000);
  printWind("City Name", timeS, day, windS);
  delay (5000);
  printVISIBILITY("City Name", timeS, day,  Visibility);
  delay (5000);
  printairpressure("City Name", timeS, day,  pressure);
  delay (5000);
  printwindangle("City Name", timeS, day,  Wind_angle);
  delay (5000);
  printclouds("City Name", timeS, day,  Clouds);
  delay (5000);

  
}

// =======================================================================================
// Print Home page with all details
void printGeneral(String city, String timeS, String day, int weatherID, String description, float temperature, String umidity){
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  printWeatherIcon(weatherID);

  tft.setCursor(2,140);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(description);
  
}

// =======================================================================================
// Print Weather with icon
void printWeather(String city, String timeS, String day, int weatherID, String description) {
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  printWeatherIcon(weatherID);

  tft.setCursor(1,122);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(description);
}

// =======================================================================================
// Print temperature display
void printTemperature(String city, String timeS, String day, float temperature){

  tft.fillScreen(TFT_WHITE);
 
  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawThermometer();

  tft.setCursor(30,135);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(temperature);
  tft.print("°C");
}

// =======================================================================================
// Print umidity display
void printUmidity(String city, String timeS, String day, String umidity){

  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawUmidity();

  tft.setCursor(47,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(umidity);
  tft.print("%");
}

// =======================================================================================
// Print wind display
void printWind(String city, String timeS, String day, String wind){
 
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawWind();

  tft.setCursor(40,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(wind);
  tft.print("m/s");
}



void printVISIBILITY(String city, String timeS, String day, String visibilityLOC){
 
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawvisibility();

  tft.setCursor(30,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(visibilityLOC);
  tft.print("M");
}

void printairpressure(String city, String timeS, String day, String pressure){

  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawairpressure();

  tft.setCursor(25,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(pressure);
  tft.print(" Hpa");
}

void printwindangle(String city, String timeS, String day, String Wind_angle){
 
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawwindangle();

  tft.setCursor(50,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(Wind_angle);
  tft.print("°");

}

void printclouds(String city, String timeS, String day, String Clouds){
 
  tft.fillScreen(TFT_WHITE);

  tft.setCursor(35,2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print(city);

  tft.setCursor(10,12);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(timeS + ' ' + day);

  drawclouds();

  tft.setCursor(50,132);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.print(Clouds);
  tft.print("%");
}


// Bodmers BMP image rendering function

void drawBmp(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
     // Serial.print("Loaded in "); Serial.print(millis() - startTime);
     // Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}





// =======================================================================================

// Print WeatherIcon based on id
void printWeatherIcon(int id) {
 switch(id) {
  case 800:  drawBmp("/sun2.bmp",16, 30); break;
  case 801:  drawBmp("/sunny.bmp",16, 30); break;
  case 802: drawBmp("/sunny.bmp",16, 30); break;
  case 803: drawBmp("/cloud.bmp",16, 30); break;
  case 804: drawBmp("/cloud.bmp",16, 30); break;
  
  case 200: drawBmp("/storm.bmp",16, 30); break;
  case 201: drawBmp("/storm.bmp",16, 30); break;
  case 202: drawBmp("/storm.bmp",16, 30); break;
  case 210: drawBmp("/storm.bmp",16, 30); break;
  case 211: drawBmp("/storm.bmp",16, 30); break;
  case 212: drawBmp("/storm.bmp",16, 30); break;
  case 221: drawBmp("/storm.bmp",16, 30); break;
  case 230: drawBmp("/storm.bmp",16, 30); break;
  case 231: drawBmp("/storm.bmp",16, 30); break;
  case 232: drawBmp("/storm.bmp",16, 30); break;

  case 300: drawBmp("/rain2.bmp",16, 30); break;
  case 301: drawBmp("/rain2.bmp",16, 30); break;
  case 302: drawBmp("/rain2.bmp",16, 30); break;
  case 310: drawBmp("/rain2.bmp",16, 30); break;
  case 311: drawBmp("/rain2.bmp",16, 30); break;
  case 312: drawBmp("/rain2.bmp",16, 30); break;
  case 313: drawBmp("/rain2.bmp",16, 30); break;
  case 314: drawBmp("/rain2.bmp",16, 30); break;
  case 321: drawBmp("/rain2.bmp",16, 30); break;

  case 500: drawBmp("/moderate_rain.bmp",16, 30); break;
  case 501: drawBmp("/moderate_rain.bmp",16, 30); break;
  case 502: drawBmp("/moderate_rain.bmp",16, 30); break;
  case 503: drawBmp("/moderate_rain.bmp",16, 30); break;
  case 504: drawBmp("/moderate_rain.bmp",16, 30); break;
  case 511: drawBmp("/rain2.bmp",16, 30); break;
  case 520: drawBmp("/rain.bmp",16, 30); break;
  case 521: drawBmp("/rain.bmp",16, 30); break;
  case 522: drawBmp("/rain.bmp",16, 30); break;
  case 531: drawBmp("/rain.bmp",16, 30); break;

  case 600: drawBmp("/snow4.bmp",16, 30); break;
  case 601: drawBmp("/snow.bmp",16, 30); break;
  case 602: drawBmp("/snow.bmp",16, 30); break;
  case 611: drawBmp("/snow.bmp",16, 30); break;
  case 612: drawBmp("/snow4.bmp",16, 30); break;
  case 615: drawBmp("/snow4.bmp",16, 30); break;
  case 616: drawBmp("/snow4.bmp",16, 30); break;
  case 620: drawBmp("/snow4.bmp",16, 30); break;
  case 621: drawBmp("/snow4.bmp",16, 30); break;
  case 622: drawBmp("/snow4.bmp",16, 30); break;

  case 701: drawBmp("/fog.bmp",16, 30); break;
  case 711: drawBmp("/fog.bmp",16, 30); break;
  case 721: drawBmp("/fog.bmp",16, 30); break;
  case 731: drawBmp("/fog.bmp",16, 30); break;
  case 741: drawBmp("/fog.bmp",16, 30); break;
  case 751: drawBmp("/fog.bmp",16, 30); break;
  case 761: drawBmp("/fog.bmp",16, 30); break;
  case 762: drawBmp("/fog.bmp",16, 30); break;
  case 771: drawBmp("/fog.bmp",16, 30); break;
  case 781: drawBmp("/fog.bmp",16, 30); break;
  default:break; 
  }
}

// =======================================================================================
// To switch between day/night icon
void nightOrDay(String timeS) {
  timeS = timeS.substring(0,2);
  int time = timeS.toInt();
  Serial.print ( "====" );
  Serial.print ( time );
  if(time > 20 ||  time<7) {
 night = true;
 tft.invertDisplay(true);
  }else {
 night = false;
 tft.invertDisplay(false);
  }
}

// =======================================================================================
// Clear the screen

void clearScreen() {
    tft.fillScreen(TFT_WHITE);
}

void drawClearWeather(){
  if(night){
    drawBmp("/moon.bmp",16, 30);
  }else{
    drawBmp("/sun2.bmp",16, 30);
  }
}

void drawFewClouds(){
  if(night){
    drawBmp("/cludy_night.bmp",16, 30);
  }else{
    drawBmp("/sunny.bmp",16, 30);
  }
}

void drawTheSun(){
  drawBmp("/sun2.bmp",16, 30);
}

void drawTheFullMoon(){
  drawBmp("/full_moon.bmp",16, 30);
}

void drawTheMoon(){
  drawBmp("/moon.bmp",16, 30);
  
}

void drawCloud(){
  drawBmp("/cloud.bmp",16, 30);
}

void drawThermometer(){
  drawBmp("/cold.bmp",16, 30);
}

void drawUmidity(){
  drawBmp("/humidity.bmp",16, 30);
}

void drawWifi(){
  drawBmp("/internet.bmp",35, 30);
}

void drawCloudWithSun(){
  
  drawBmp("/sunny.bmp",16, 30);
}

void drawLightRainWithSunOrMoon(){
  if(night){  
    drawBmp("/moon_rain.bmp",16, 30);
  }else{
    drawBmp("/rain2.bmp",16, 30);
  }
}

void drawLightRain(){
  drawBmp("/moderate_rain.bmp",16, 30);
}

void drawModerateRain(){
  drawBmp("/rain.bmp",16, 30);
}

void drawHeavyRain(){
   drawBmp("/rain.bmp",16, 30);
}

void drawThunderstorm(){
   drawBmp("/storm.bmp",16, 30);
}

void drawLightSnowfall(){
   drawBmp("/snow.bmp",16, 30);
}

void drawModerateSnowfall(){
   drawBmp("/snow.bmp",16, 30);
}

void drawHeavySnowfall(){
   drawBmp("/snow.bmp",16, 30);
}

void drawCloudSunAndRain(){
   drawBmp("/rain.bmp",16, 30);
}

void drawCloudAndTheMoon(){
   drawBmp("/cloudy_night.bmp",16, 30);
}

void drawCloudTheMoonAndRain(){
   drawBmp("/moon_rain.bmp",16, 30);
}

void drawWind(){  
  drawBmp("/wind.bmp",16, 30); 
}

void drawFog()  {
   drawBmp("/fog.bmp",16, 30);
}

void clearIcon(){
   drawBmp("/sun2.bmp", 16, 30);
}
void drawtemp_min(){
  drawBmp("/cold2.bmp", 16, 30);
}
void drawtemp_max(){
  drawBmp("/heat.bmp", 16, 30);
}
void drawvisibility(){
  drawBmp("/mountain.bmp", 16, 30);
}
void drawairpressure(){
  drawBmp("/air_pressure.bmp", 16, 30);
}
void drawwindangle(){
  drawBmp("/angle.bmp", 16, 30);
}
void drawclouds(){
  drawBmp("/cloud3.bmp", 16, 30);
}

