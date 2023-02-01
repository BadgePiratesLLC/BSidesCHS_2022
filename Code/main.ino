#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <base64.hpp>

bool DEBUG = true;

int randomNum; // 3 digit pin random number

/* led timer variables */
os_timer_t ledTimer;
byte i = 0;
bool flags[] = {false, false, false};
byte leds[] = {16, 13, 12, 14};
byte phase = 1; // state machine control variable for blinkLights()
bool blinkyLights = false;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);

  for(int j=0; j<4; j++)
     pinMode(leds[j], OUTPUT);

  // setup blinky lights with os_timer to callback every 500ms
  os_timer_setfn(&ledTimer, blinkLights, NULL);
  os_timer_arm(&ledTimer, 1000, true);

  // set random number for challenge 1
  randomSeed(analogRead(0));
  randomNum = random(100, 1000);
  if (DEBUG)
    Serial.printf("Random: %d\n", randomNum);
}

void blinkLights(void *pArg){
  if (blinkyLights){
    if (phase == 1){
      // leds[0] always blinks
      digitalWrite(leds[0], HIGH);
      i = 1;
      phase = 2;
    }else if (phase == 2){
      // leds[1-3] blink when flags are obtained
      digitalWrite(leds[i], flags[i-1] ? HIGH : LOW);
      i++;
      if (i == 4){
        i = 0;
        phase = 3;
      }
    }else if (phase == 3){
      // turn off all leds
      for(byte j=0; j<4; j++)
        digitalWrite(leds[j], LOW);
      phase = 1;
    }
  }else{
      digitalWrite(leds[0], HIGH);
      for(byte j=1; j<4; j++)
        digitalWrite(leds[j], flags[j-1] ? HIGH : LOW);
  }
}

bool flag1(){
  Serial.printf("Enter the 3 digit PIN: ");
  while(Serial.available() == 0){}
  String input = Serial.readStringUntil('\n');
  if (input.toInt() == randomNum){
    return true;
  }else{
    return false;
  }
}

/* root webpage located at http://192.168.4.1 */
void handleRoot() {
  bool solved = false;

  for (byte j = 0; j < server.args(); j++) { 
    if (server.argName(i) == "plaintext"){
      if (server.arg(i) == "Time is what determines security. With enough time nothing is unhackable."){
        server.send(200, "text/html", "Flag 3 achieved! <!-- Easter Egg: http://192.168.4.1/lights -->");
        solved = true;
        flags[2] = true;
        Serial.println("\n[*INFO] Flag 3 achieved!");
      }
    }
  }

  if (!solved){
    server.send(200, "text/html", "<html>\n<title=\"Bsides Charleston 2022\" />\n<body>\n<h1>Flag 3</h1>\n<br /><p>k40DjDm4ZJxFyDDmfIlYsn/FRw3djw4RLkXsaYjn+cMcJmNmVVuNYralLBRYe92dM7nlGQIAx2DbBVKWTtAp1baFExHArfqwBg==</p>\n<!-- passphrase = bsideschs -->\n<form method=\"POST\">\n<br />Plaintext <input type=\"text\" name=\"plaintext\" /> <input type=\"SUBMIT\" value=\"check\"/>\n</form>\n</body>\n</html>");
  }
}

/* root webpage located at http://192.168.4.1 */
void handleLights() {
    String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; 
  Serial.println(message);
  
  for (byte j = 0; j < server.args(); j++) { 
    if (server.argName(i) == "lightmode"){
      if (server.arg(i) == "blinkyLights"){
        blinkyLights = true;
        Serial.println("Setting blinkyLights!");
      }else{
        blinkyLights = false;
        Serial.println("Setting solid lights!");
      }
    }
  }


  server.send(200, "text/html", "<html><form method=\"POST\"><label for=\"lightmode\">Choose a light mode:</label><select id=\"lightmode\" name=\"lightmode\"><option value=\"blinkyLights\">Blinky Lights</option><option value=\"solid\">Solid</option></select><input type=\"SUBMIT\" value=\"Submit\" /></form></html>");


  
}

void flag2(){
  
    String macAddress = WiFi.macAddress().substring(9); // only grab NIC specific portion of address (lower 3 octets) 
    macAddress.replace(":","");

    String ssid = "chsbsides_" + macAddress; // chsbsides_ABCDEF
    
    String seed = WiFi.macAddress().substring(15);

    String hex[] = {"A", "B", "C", "D", "E", "F"};
    String values[] = {"1", "2", "3", "4", "5", "6"};
    int i;
    for(i=0; i<5; i++)
      seed.replace(hex[i], values[i]);

    randomSeed(seed.toInt());
    
    unsigned char password[21] = "chsbsides{flag2_";
    password[16] = (char)random(97,123);
    password[17] = (char)random(97,123);
    password[18] = (char)random(97,123);
    password[19] = '}';
    password[20] = '\0'; // chsbsides{flag2_abc}
    unsigned char base64[40];
    encode_base64(password, strlen((char *)password), base64);
    
    if (DEBUG)
       Serial.printf("Password = %s\n", password);
       
    Serial.printf("Your encoded password = %s\n", base64);
    WiFi.softAP(ssid, String((char*)password));
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.printf("Your SSID = %s%s\n", "chsbsides_", macAddress);
    
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.on("/lights", handleLights);
    server.begin();
    Serial.println("HTTP server started");


    // wait for client to connect
    while (WiFi.softAPgetStationNum() == 0){
      delay(1000);
    }
}

void loop() {
  if (!flags[0]){
    /* challenge 1 - brute force 3 digit pin */
    while (!flag1()) {
      Serial.printf("Access denied\n");
    }
    Serial.println("\n[*INFO] Flag 1 achieved!");
    flags[0] = true;
  }

  if (!flags[1]){
    /* challenge 2 - connect to WiFi */
    flag2();
    flags[1] = true;
    Serial.println("\n[*INFO] Flag 2 achieved!");
    Serial.println("[*INFO] Visit http://192.168.1.4 in your browser.");
  }


  /* challenge 3 - web page challenge */
  server.handleClient();

}
