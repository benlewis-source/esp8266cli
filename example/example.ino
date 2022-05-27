#include "ESP8266Cli.h"

ESP8266Cli cmd(&Serial, &WiFi);

void setup() {
  // put your setup code here, to run once:
  cmd.autoLaunch(3);  //Wait 3s on startup for user to enter CLI menu
}

void loop() {
  // put your main code here, to run repeatedly:

}
