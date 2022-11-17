#include <Arduino.h>
#include <AsyncTCP.h>
#include <E4S.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

E4S board = E4S();

bool motorToggle;

const char *ssid = "Ballenwerper";
const char *password = "controller";
const char *input_parameter = "value";
String slider_value = "255";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);

String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Ballenwerper</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: "
         "0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} "
         "h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=
      ".button {display: block;width: 80px;background-color: #3498db;border: "
      "none;color: white;padding: 13px 30px;text-decoration: none;font-size: "
      "25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += ".piston {background-color: #03c04a;}\n";
  ptr += ".piston:active {background-color: #028a0f;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Ballenwerper\n";

  if (motorToggle) {
    ptr += "<p>Motor: ON</p><a class=\"button button-off\" "
           "href=\"/motor-off\">OFF</a>\n";
  } else {
    ptr += "<p>Motor: OFF</p><a class=\"button button-on\" "
           "href=\"/motor-on\">ON</a>\n";
  }

  ptr += "</body>\n";
  ptr += "</html>\n";
  Serial.print(ptr);
  return ptr;
}

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html> 
<html>
   <head>
      <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
      <title>Ballenwerper</title>
      <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
         body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
         .button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}
         .button-on {background-color: #3498db;}
         .button-on:active {background-color: #2980b9;}
         .button-off {background-color: #34495e;}
         .button-off:active {background-color: #2c3e50;}
         .piston {background-color: #03c04a;}
         .piston:active {background-color: #028a0f;}
         p {font-size: 14px;color: #888;margin-bottom: 10px;}
      </style>
   </head>
   <body>
      <h1>Ballenwerper</h1>
      <p>Motor: %ENABLE2%</p>
      <a class="button button-%ENABLE1%" href="/motor-%ENABLE3%">%ENABLE2%</a>
      <p id="pwmInfo">Speed: %SLIDERVALUE%
      <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="255" value="%SLIDERVALUE%" step="1" class="slider"></p>
   </body>
   <script>
      function updateSliderPWM(element) {
        var slider_value = document.getElementById("pwmSlider").value;
        document.getElementById("pwmInfo").innerHTML = `Speed: ${slider_value}`;

        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?value="+slider_value, true);
        xhr.send();
      }
   </script>
</html>
)rawliteral";

String processor(const String &var) {
  if (var == "SLIDERVALUE") {
    return slider_value;
  } else if (var == "ENABLE1") {
    if (motorToggle) {
      return "on";
    }
    return "off";
  } else if (var == "ENABLE2") {
    if (motorToggle) {

      return "ON";
    }
    return "OFF";
  }

  else if (var == "ENABLE3") {
    if (motorToggle) {

      return "off";
    }
    return "on";
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      slider_value = message;
    } else {
      message = "No message sent";
    }
    board.digitalDirectOutput1.writeAnalog(slider_value.toInt());
    request->send(200, "text/plain", "OK");
  });
  server.on("/motor-on", HTTP_GET, [](AsyncWebServerRequest *request) {
    board.relay1.write(1);
    motorToggle = true;
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/motor-off", HTTP_GET, [](AsyncWebServerRequest *request) {
    board.relay1.write(0);
    motorToggle = false;
    request->send_P(200, "text/html", index_html, processor);
  });
  server.begin();
};

void loop() {}