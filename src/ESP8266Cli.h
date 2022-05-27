#ifndef ESP_CLI_H
#define ESP_CLI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>


class ESP8266Cli {
    private:
        HardwareSerial *io;
        ESP8266WiFiClass *wifi;
        char args[5][64];
        void getInput(char *prompt, bool local_echo=true);
        void parseInput(char *input);
        
        // commands
        void info(void);
        void help(void);
        void listNetworks(void);
        void connect(void);
        void reconnect(void);
        void disconnect(void);

        // getters
        void getMode(void);
        void getStatus(void);
        void getConfig(bool _default=true);
        void getAutoConnect(void);
        void getAutoReconnect(void);
        void getPersistence(void);

        // setters
        void setMode(char *state);
        void setAutoConnect(char *state);
        void setAutoReconnect(char *state);
        void setPersistence(char *state);

    public:
        ESP8266Cli(HardwareSerial *io, ESP8266WiFiClass *wifi);
        void begin(void);
        void autoLaunch(uint8_t timeout);
};

#endif