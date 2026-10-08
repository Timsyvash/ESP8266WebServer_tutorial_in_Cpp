#include <Arduino.h>
#include <ESP8266WiFi.h>

const char *ssid = "EpsilonNET";
const char *password = "zaq123456";

WiFiServer server(80);

String header;

String output5State = "off";
String output4State = "off";

// На Wemos D1 mini: GPIO5 = D1, GPIO4 = D2
const int output5 = D1; // GPIO5
const int output4 = D2; // GPIO4

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("New Client.");
    String currentLine = "";

    // Додаємо таймаут для захисту від застрягання
    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 2000)
    {
      if (client.available())
      {
        char c = client.read();
        header += c;

        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /5/on") >= 0)
            {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            }
            else if (header.indexOf("GET /5/off") >= 0)
            {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            }
            else if (header.indexOf("GET /4/on") >= 0)
            {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            }
            else if (header.indexOf("GET /4/off") >= 0)
            {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }

            // HTML сторінка з виправленим CSS
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button {background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            client.println("<body><h1>ESP8266 Web Server</h1>");

            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            if (output5State == "off")
            {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            if (output4State == "off")
            {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
      }
      yield(); // Віддаємо час фоновим процессам ESP8266
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }
}