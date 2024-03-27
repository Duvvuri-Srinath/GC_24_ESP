#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

char ssid[] = "ENTER_SSID";
char password[] = "ENTER_PASS";
String teamid = "38.1";
String apikey = "op8L9vcGfuCwlHIVEMUM55ugcisbkWjP";
WiFiClientSecure client;
#define HOST "esptrade.gciitbbs.com"
bool check = false;

const int OVERBOUGHT_THRESHOLD = 80;
const int OVERSOLD_THRESHOLD = 20;

// Define the moving average periods
const int ma5Period = 5;
const int ma11Period = 11;
const int stochasticK_Period = 7;
const int stochasticD_Period = 3;

// Arrays to hold the price data
float ma5[ma5Period] = {0,0,0,0,0};
float ma11[ma11Period] = {0,0,0,0,0,0,0,0,0,0,0};
float stoch[stochasticD_Period] = {0,0,0};
float priceData[stochasticK_Period] = {0,0,0,0,0,0,0};

// Indexes for the moving average arrays
int ma5Index = 0;
int ma11Index = 0;
int stochIndex = 0;
int priceIndex = 0;

float currAmt = 10000;
float currStock = 0;
float currVal = 0;

// Variables to store the moving averages
float movingAverage5 = 0;
float movingAverage11 = 0;
float stochasticK_Value = 0;
float stochasticD_Value = 0;

// Function to calculate the moving average
float calculateMovingAverage(float *data, int period) {
  float sum = 0;
  for (int i = 0; i < period; i++) {
    sum += data[i];
  }
  return sum / period;
}

// Function to update the moving average array and calculate the new average
void updateMovingAverage(float *data, int &index, int period, float &movingAverage, float newVal) {
  // Update the array with the new value
  data[index] = newVal;
  
  // Calculate the new moving average
  movingAverage = calculateMovingAverage(data, period);
  
  // Increment the index and wrap around if necessary
  index = (index + 1) % period;
}

void updatePrice(float *data, int &index, int period, float newVal) {
  // Update the array with the new value
  data[index] = newVal;
  
  // Increment the index and wrap around if necessary
  index = (index + 1) % period;
}

// Function to find the lowest low in an array
float findLowestLow(float *data, int period) {
  float lowestLow = data[0];
  for (int i = 1; i < period; i++) {
    if (data[i] < lowestLow) {
      lowestLow = data[i];
    }
  }
  return lowestLow;
}

// Function to find the highest high in an array
float findHighestHigh(float *data, int period) {
  float highestHigh = data[0];
  for (int i = 1; i < period; i++) {
    if (data[i] > highestHigh) {
      highestHigh = data[i];
    }
  }
  return highestHigh;
}


// Function to calculate stochastic oscillator
float calculateStochastic(float *data, int period, float currentClose) {
  float lowestLow = findLowestLow(data, period);
  float highestHigh = findHighestHigh(data, period);
  return ((currentClose - lowestLow) / (highestHigh - lowestLow)) * 100;
}

void fetchstocksdata(){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("GET "));  
  client.print("/api/curr-stock-data"); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }
  // //While the client is still availble read each byte and print to the serial monitor  
  // String input ="";
  // while (client.available()){
  //   char c = 0;
  //   client.readBytes(&c, 1);
  //   input+=c;
  // } 
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  // previousOpen = doc["open"].as<float>(); // "224.7"
  // previousHigh = doc["high"].as<float>(); // "224.74"
  // previousLow = doc["low"].as<float>(); // "224.4518"
  // previousClose = doc["close"].as<float>(); // "224.4518"
  // previousVolume = doc["volume"].as<float>(); // "5570"
  // const char* time = doc["time"];

  float newVal = doc["close"].as<float>(); 

  currVal = newVal;

  updateMovingAverage(ma5, ma5Index, ma5Period, movingAverage5, newVal);
  updateMovingAverage(ma11, ma11Index, ma11Period, movingAverage11, newVal);
  updatePrice(priceData, priceIndex, stochasticK_Period, newVal);
  stochasticK_Value = calculateStochastic(priceData, stochasticK_Period, newVal);
  updateMovingAverage(stoch, stochIndex, stochasticD_Period, stochasticD_Value, stochasticK_Value);

}

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  client.setInsecure();


  for(int i=0;i<ma11Period;i++){
    fetchstocksdata();
  }
  
}
void buystocks(String amount){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("POST "));  
  client.print("/api/buy?amount="+amount); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));
  

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }

  //While the client is still availble read each byte and print to the serial monitor  
  while (client.available()){
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
  }
}

void sellstocks(String amount){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("POST "));  
  client.print("/api/sell?amount="+amount); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));
  

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }

  //While the client is still availble read each byte and print to the serial monitor  
  while (client.available()){
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
  }
}

void mystatus(){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("GET "));  
  client.print("/api/mystatus"); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }

  //While the client is still availble read each byte and print to the serial monitor  
  while (client.available()){
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
  } 

  // StaticJsonDocument<256> doc;
  // DeserializationError error = deserializeJson(doc, client);
  // if (error) {
  //   Serial.print(F("deserializeJson() failed: "));
  //   Serial.println(error.f_str());
  //   return;
  // }

  // float currAmt = doc["balance"].as<float>(); 
  // float currStock = doc["stocks"].as<float>(); 
}

void mytransactions(){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("GET "));  
  client.print("/api/transactions"); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }

  //While the client is still availble read each byte and print to the serial monitor  
  while (client.available()){
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
  } 
}

void faucetstock(){
  if (!client.connect(HOST, 443)){
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(client);
  yield();  
  
  client.print(F("POST "));  
  client.print("/api/faucet"); // after GET
  client.println(F(" HTTP/1.0")); //!very important

  //Headers (POSTMAN)
  client.print(F("Host: "));
  client.println(HOST);
  client.println(F("Cache-Control: no-cache"));
  client.println(F("teamid: 38.1"));
  client.println(F("api-key: op8L9vcGfuCwlHIVEMUM55ugcisbkWjP"));
  

  if (client.println() == 0){
    Serial.println(F("Failed to send request"));
    return;
  }
  delay(100);
  Serial.println(client.println());
  
  // // Check HTTP status
  // char status[32] = {0};
  // client.readBytesUntil('\r', status, sizeof(status));  
  // // Check if it responded "OK" 
  // if (strcmp(status, "HTTP/1.0 200 OK") != 0 || strcmp(status, "HTTP/1.1 200 OK") != 0){
  //   Serial.print(F("Unexpected response: "));
  //   Serial.println(status);
  //   return;
  // }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
  }

  //While the client is still availble read each byte and print to the serial monitor  
  while (client.available()){
    char c = 0;
    client.readBytes(&c, 1);
    Serial.print(c);
  }
}
void loop()
{

    mystatus();

    // String buyAmt = String(currAmt*0.05, 4);
    // String sellAmt = String(currStock*currVal*0.05, 4);

  // TRADE STRATEGY

  printf("ma5: %f, ma11: %f, k: %f, d: %f\n", movingAverage5, movingAverage11, stochasticK_Value, stochasticD_Value);

  if (movingAverage5 > movingAverage11 && stochasticK_Value > stochasticD_Value) {
    // Golden cross - MA5 crosses above MA11
    Serial.println("Buy Signal");
    buystocks("1");
    mystatus();

  } else if (movingAverage5 < movingAverage11 && stochasticK_Value < stochasticD_Value) {
    // Death cross - MA5 crosses below MA11
    Serial.println("Sell Signal");
    sellstocks("1");
    mystatus();
  }

  fetchstocksdata();

  // buystocks("1");
  // sellstocks("1");
  // faucetstock();
  // mystatus();
  // mytransactions();

  delay(60000);
}