#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define LEDka LED_BUILTIN
#define analogPin A0

const char* wifiSSID = "Cono'lin_RD";
const char* wifiPassword = "KldPo.2023";

unsigned long interval = 1000;

const bool on = LOW;
const bool off = HIGH;

ESP8266WebServer server(80);

// podprogram s hlavní zprávou, která je vytištěna
// při zadání IP adresy do prohlížeče
void zpravaHlavni() {
  String analog = String(analogRead(analogPin));
  String cas = String(millis() / 1000);
  String zprava = "<!DOCTYPE html>"
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
                      "<p>Analogový pin A0: " + analog + "</p>"
                      "<p>Čas od spuštění Arduina je " + cas + " vteřin.</p>"
                      "<p>Interval blikání LED: "+ String(interval) + "ms</p>"
                      "<p><a href=\"/blink250ms\">Blikej po 250ms</a></p>"
                      "<p><a href=\"/blink2000ms\">Blikej po 2000ms</a></p>"
                    "</body>"
                  "</html>";

  server.send(200, "text/html", zprava);
}



// podprogram s chybovou zprávou, která je vytištěna
// při zadání IP adresy s neexistující podstránkou
void zpravaNeznamy() {
  // vytvoření zprávy s informací o neexistujícím odkazu
  // včetně metody a zadaného argumentu
  String zprava = "Neexistujici odkaz\n\n";
  zprava += "URI: ";
  zprava += server.uri();
  zprava += "\nMetoda: ";
  zprava += (server.method() == HTTP_GET) ? "GET" : "POST";
  zprava += "\nArgumenty: ";
  zprava += server.args();
  zprava += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    zprava += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  // vytištění zprávy se statusem 404 - Nenalezeno
  server.send(404, "text/plain", zprava);
}

void setup() {
  // nastavení LED diody jako výstupní a její vypnutí
  pinMode(LEDka, OUTPUT);
  digitalWrite(LEDka, off);
  // zahájení komunikace po sériové lince
  Serial.begin(115200);
  // zahájení komunikace po WiFi s připojením
  // na router skrze zadané přihl. údaje
  WiFi.begin(wifiSSID, wifiPassword);
  // čekání na úspěšné připojení k routeru,
  // v průběhu čekání se vytiskne každých
  // 500 milisekund tečka po sériové lince
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // odřádkování a výpis informací o úspěšném připojení
  // včetně přidelené IP adresy od routeru
  Serial.println("");
  Serial.print("Pripojeno k WiFi siti ");
  Serial.println(wifiSSID);
  Serial.print("IP adresa: ");
  Serial.println(WiFi.localIP());
  // kontrola funkčnosti MDNS
  String mdnsName = "LED-lightning-corridor";
  if (MDNS.begin(mdnsName)) {
    Serial.println("MDNS responder je zapnuty.");
  }
  // nastavení vytištění hlavní zprávy po přístupu
  // na samotnou IP adresu
  server.on("/", zpravaHlavni);
  // pokud chceme vytisknout pouze menší zprávy, není
  // nutné je vytvářet v podprogramech jako zpravaHlavni,
  // viz. ukázka níže

  // nastavení vytištění jiné zprávy po přístupu na
  // podstránku ukazka, tedy např. 10.0.0.31/ukazka
  server.on("/ukazka", []() {
    String zprava = "Ukazka odkazu pro vice stranek.";
    server.send(200, "text/plain", zprava);
  });
  // nastavení stavu LED diody na zapnuto
  server.on("/blink250ms", []() {
    // zapnutí LED diody
    interval = 250;
    Serial.println("Spuštěno blikání po 250ms");
    // vytištění hlavní stránky
    zpravaHlavni();
  });
  // nastavení stavu LED diody na vypnuto
  server.on("/blink2000ms", []() {
    // vypnutí LED diody
    interval = 2000;
    Serial.println("Spuštěno blikání po 2000ms");
    // vytištění hlavní stránky
    zpravaHlavni();
  });
  // nastavení vytištění informací o neznámém
  // odkazu pomocí podprogramu zpravaNeznamy
  server.onNotFound(zpravaNeznamy);
  // zahájení aktivity HTTP serveru
  server.begin();
  Serial.println("HTTP server je zapnuty.");
  Serial.print("Otevři stránku: http://");
  Serial.print(mdnsName);
  Serial.println("/");
}

void toggleLED() {
  if (digitalRead(LEDka) == off) {
    digitalWrite(LEDka, on);
  } else {
    digitalWrite(LEDka, off);
  }
}

void printTick() {
  Serial.println("tick..");
}

void callActionWithDelay(void (*action)(), unsigned long delayTime, unsigned long* previousMillis) {
  unsigned long currentMillis = millis();

  if (currentMillis - *previousMillis >= delayTime) {
    *previousMillis = currentMillis;
    action();  // Zavolej akci
  }
}

void loop() {
  server.handleClient();
  MDNS.update();

  static unsigned long previousMillisForLED = 0;
  static unsigned long previousMillisForTick = 0;
  callActionWithDelay(toggleLED, interval, &previousMillisForLED);
  callActionWithDelay(printTick, 5000, &previousMillisForTick);
}