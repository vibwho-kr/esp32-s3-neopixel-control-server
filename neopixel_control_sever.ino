#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "....."; //your network ssid and password
const char *password = ".....";

const int rled = 48;

WebServer server(80);

//-----------------------------------------------------------

// the html, css and javascript for the webpage
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 s3 neopixel controller</title>
    <style>
        body {
            background: #21232b;
            color: #f3f3f3;
            font-family: 'Segoe UI', Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 40px;
            min-height: 100vh;
        }

        .container {
            background: #2d323c;
            padding: 30px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            border-radius: 10px;
            box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
        }

        h1 {
            color: #f3f3f3;
            margin-top: 0;
        }

        input[type="color"] {
            -webkit-appearance: none;
            appearance: none;
            border: none;
            width: 150px;
            height: 150px;
            border-radius: 50%;
            cursor: pointer;
            background: transparent;
            padding: 0;
            overflow: hidden;
            display: block;
        }

        input[type="color"]::-webkit-color-swatch-wrapper {
            padding: 0;
        }

        input[type="color"]::-webkit-color-swatch {
            border: none;
            border-radius: 50%;
        }

        input[type="color"]::-moz-color-swatch {
            border: none;
            border-radius: 50%;
        }
    </style>
</head>

<body>
    <div class="container">
        <h1>RGB LED Controller</h1>
        <p>Pick a color to change the LED dynamically:</p>
        <input type="color" id="picker" value="#000000" onchange="sendColor(this.value)">
    </div>
    <script>
        function sendColor(hex) {
            // Encode '#' as '%23' for the URL query parameter
            fetch('/setcolor?hex=' + encodeURIComponent(hex));
        }
    </script>
</body>
</body>
)rawliteral";

//-----------------------------------------------------------

//root is what displays by default on loading the server, in this case we load the colour picker ui
void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
  rgbLedWrite(rled, 0, 0, 0);
}

//here we handle what to do when the user goes to an invalid page on the server
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (int i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  rgbLedWrite(rled, 200, 0, 0); //flash the led red
  delay(50);
  rgbLedWrite(rled, 0, 0, 0);
}

//here we handle what to do on recieving the user sent rgb data
void handleSetColor() {
    if (server.hasArg("hex")) {
        String hexStr = server.arg("hex"); // Expected format: #RRGGBB
        
        // Strip out the '#' character if present
        if (hexStr.startsWith("#")) {
            hexStr = hexStr.substring(1);
        }
        
        // Convert hex chunks to integer color levels (0-255)
        long number = strtol(hexStr.c_str(), NULL, 16);
        int r = (number >> 16) & 0xFF;
        int g = (number >> 8) & 0xFF;
        int b = number & 0xFF;

        rgbLedWrite(rled, r, g, b);

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

//--------------------------------------------------------

void setup() {
  pinMode(rled, OUTPUT);
  rgbLedWrite(rled, 0, 0, 0);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); //setting up t he esp in wifi station mode
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/setcolor", handleSetColor);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(1); //small delay to allow time for the cpu to handle tasks
}
