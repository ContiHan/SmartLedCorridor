#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* wifiSSID = "Cono'lin_RD";
const char* wifiPassword = "KldPo.2023";

unsigned long blinkInterval = 1000;

const bool ledOn = LOW;
const bool ledOff = HIGH;

ESP8266WebServer server(80);


void mainHtmlMessage() {
  String analogA0Value = String(analogRead(A0));
  String timeFromStartInSecond = String(millis() / 1000);
  String htmlMessage = "<!DOCTYPE html>"
                  "<html lang=\"cs\">"
                    "<head>"
                      "<meta charset=\"UTF-8\">"
                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                      "<title>Smart LED Corridor</title>"
                      "<link rel=\"icon\" href=\"https://t2.gstatic.com/faviconV2?client=SOCIAL&type=FAVICON&fallback_opts=TYPE,SIZE,URL&url=http://dev.to&size=16\" type=\"image/x-icon\">"
                      "<style>"
                        "body { font-family: Arial, sans-serif; margin: 20px; padding: 20px; }"
                        "h1 { color: #333366; }"
                        "p { color: #666666; }"
                        "a { background-color: #4CAF50; color: white; padding: 10px 20px; text-decoration: none; display: inline-block; }"
                        "a:hover { background-color: #45a049; }"
                      "</style>"
                    "</head>"
                    "<body>"
                      "<h1>Ahoj Arduino světe!</h1>"
                      "<p>Analogový pin A0: " + analogA0Value + "</p>"
                      "<p>Čas od spuštění Arduina je " + timeFromStartInSecond + " vteřin.</p>"
                      "<p>Interval blikání LED: "+ String(blinkInterval) + "ms</p>"
                      "<p><a href=\"/blink250ms\">Blikej po 250ms</a></p>"
                      "<p><a href=\"/blink2000ms\">Blikej po 2000ms</a></p>"
                    "</body>"
                  "</html>";

  server.send(200, "text/html", htmlMessage);
}

void unknownHtmlMessage() {
  String message = "Neexistujici odkaz\n\n"
                  "URI: " + server.uri() + "\n"
                  "Metoda: " + ((server.method() == HTTP_GET) ? "GET" : "POST") + "\n"
                  "Argumenty: " + String(server.args()) + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ledOff);

  Serial.begin(115200);

  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Pripojeno k WiFi siti: ");
  Serial.println(wifiSSID);
  Serial.print("IP adresa: ");
  Serial.println(WiFi.localIP());

  String mdnsName = "LED-lightning-corridor";
  if (MDNS.begin(mdnsName)) {
    Serial.println("MDNS responder je zapnuty.");
  }

  server.on("/", mainHtmlMessage);

  server.on("/blink250ms", []() {
    blinkInterval = 250;
    Serial.println("Spuštěno blikání po 250ms");
    mainHtmlMessage();
  });

  server.on("/blink2000ms", []() {
    blinkInterval = 2000;
    Serial.println("Spuštěno blikání po 2000ms");
    mainHtmlMessage();
  });

  server.onNotFound(unknownHtmlMessage);

  server.begin();
  Serial.println("HTTP server je zapnuty.");
  Serial.print("Otevři stránku: http://");
  Serial.print(mdnsName);
  Serial.println("/");
}

void toggleLED() {
  if (digitalRead(LED_BUILTIN) == ledOff) {
    digitalWrite(LED_BUILTIN, ledOn);
  } else {
    digitalWrite(LED_BUILTIN, ledOff);
  }
}

void printTick() {
  Serial.println("tick..");
}

void callActionWithDelay(void (*action)(), unsigned long delayTime, unsigned long* previousMillis) {
  unsigned long currentMillis = millis();

  if (currentMillis - *previousMillis >= delayTime) {
    *previousMillis = currentMillis;
    action();
  }
}

void loop() {
  server.handleClient();
  MDNS.update();

  static unsigned long previousMillisForLED = 0;
  static unsigned long previousMillisForTick = 0;
  callActionWithDelay(toggleLED, blinkInterval, &previousMillisForLED);
  callActionWithDelay(printTick, 5000, &previousMillisForTick);
}