#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>
const char* ssid = "Hoang Vu";
const char* password = "22092004";

//access point
const char* username = "DigitalClock";
const char* pass = "iot102project";

ESP8266WebServer server(80);

const char* host = "api.openweathermap.org";
const int port = 80;
const char* url = "/data/2.5/weather?lat=10.75&lon=106.6667&appid=bf9ed7659ed7aab5fd93f0c0f8db3bd7&units=metric";


boolean alarm_status = false;
String desc;
float heat, humid;
String weather;

void readWeather() {
  WiFiClient client;
        if (!client.connect(host, port)) {
            Serial.println("Connection failed.");
            return;
        }

        // Make a HTTP request
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");

        // Wait for response headers
        while (!client.available()) {
            delay(100);
        }

        // Read response headers
        while (client.available()) {
            String line = client.readStringUntil('\r');
            if (line == "\n") {
                break; // End of headers
            }
        }

        // Read response body in chunks
        const size_t bufferSize = 2048;
        DynamicJsonDocument doc(bufferSize);
        DeserializationError error;
        size_t bytesRead = 0;

        while (client.available()) {
            // Read in chunks
            char buffer[bufferSize];
            size_t size = client.readBytes(buffer, bufferSize);
            bytesRead += size;

            // Parse JSON
            error = deserializeJson(doc, buffer, size);
            if (error) {
                Serial.println("Failed to parse JSON.");
                return;
            }
        }

        // Extract data from JSON
        desc = (String) doc["weather"][0]["description"];
        heat = (float) doc["main"]["temp"];
        humid =  (float) doc["main"]["humidity"];
               
  client.stop();
}
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  readWeather();
  WiFi.softAP(username,pass);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(" Access Point IP:");
  Serial.println(myIP);
  server.on("/", establish);
  server.on("/datetime", show_datetime);
  server.on("/weather", show_weather);
  server.on("/temperature", show_temp);
  server.on("/alarm", set_alarm);
  server.on("/alarm-off", stop_alarm);
  server.on("/alarm-on", start_alarm);
  server.onNotFound(show_notfound);
  server.begin();
  Serial.flush();
}

String webpage(int op) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>IOT Project</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += " button {display: block;width: auto;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #3499db;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>IOT DIGITAL CLOCK</h1>\n";
 if(op == 1)
    ptr += "<button class=\"button-on\"><a href=\"/datetime\">Datetime</a></button>\n";
else
    ptr += "<button><a href=\"/datetime\">Datetime</a></button>\n";
ptr += "<br>";
if(op == 2)
    ptr += "<button class=\"button-on\"><a href=\"/weather\">Weather</a></button>\n";
else
    ptr += "<button><a href=\"/weather\">Weather</a></button>\n";
ptr += "<br>";
if(op == 3)
    ptr += "<button class=\"button-on\"><a href=\"/temperature\">Temperature</a></button>\n";
else
    ptr += "<button><a href=\"/temperature\">Temperature</a></button>\n";
ptr += "<br>";
if(alarm_status == false)
    ptr += "<button><a href=\"/alarm\">Alarm</a></button>\n";
else
  ptr += "<button class=\"button-off\"><a href=\"/alarm-off\">Stop Alarm</a></button>\n";
    
  
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void establish() {
  Serial.write('1');
  server.send(200, "text/html", webpage(1));
}

void show_datetime() {
  Serial.write('1');
  server.send(200, "text/html", webpage(1));
}

void show_weather() {
  Serial.write('2');
  Serial.println(desc);
  Serial.println(heat);
  Serial.println(humid);
  server.send(200, "text/html", webpage(2));
}

void show_temp() {
  Serial.write('3');
  server.send(200, "text/html", webpage(3));
}

void set_alarm() {
  String form = "<html><head><title>Date and Time Form</title></head><body>";
    form += "<h2>Date and Time Form</h2>";
    form += "<form action=\"/alarm-on\" method=\"post\">";
    form += "Date: <input type=\"date\" name=\"date\"><br><br>";
    form += "Time: <input type=\"time\" name=\"time\"><br><br>";
    form += "<input type=\"submit\" value=\"Submit\">";
    form += "</form></body></html>";
    server.send(200, "text/html", form);
}

void start_alarm() {
  String ddate = server.arg("date");
  String ttime = server.arg("time");
  Serial.write('4');
  
  Serial.println(ddate+";"+ttime);
  String content = "<html><head><title>Set Date and Time</title></head><body>";
  content += "<h2>Date and Time Set Successfully</h2>";
  content += "<p>Date: " + ddate + "</p>";
  content += "<p>Time: " + ttime + "</p>";
  content += "<form action=\"/\" method=\"get\">";
  content += "<input type=\"submit\" value=\"Return to Home Page\">";
  content += "</form>";
  content += "</body></html>";
  alarm_status = true;
  server.send(200, "text/html", content);
}

void stop_alarm() {
  Serial.write('5');
  alarm_status = false;
  server.send(200, "text/html", webpage(1));
}
void show_notfound() {
  server.send(404, "text/plain", "Not Found");
}

void loop() {
  server.handleClient();
}
