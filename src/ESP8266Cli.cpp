#include "ESP8266Cli.h"


ESP8266Cli::ESP8266Cli(HardwareSerial *io, ESP8266WiFiClass *wifi){
    this->io = io;
    this->wifi = wifi;
}

void ESP8266Cli::autoLaunch(uint8_t timeout){
    io->println("\npress any key to launch CLI");
    unsigned long int _timeout = millis() + (1000 * timeout);
    while (millis() < _timeout){
        if (io->available()){
            io->println(io->read());
            this->begin();
            break;
        }
        io->print('.');
        delay(100);
    }
    io->println("\nstarting main application");
}

void ESP8266Cli::begin(void){
    enableWiFiAtBootTime();
    io->println("\nLaunching CLI\n");
    this->help();
    while (true){
        this->getInput("~$ ");

        if (!strcmp(args[0], "exit")){
            break;
        }
        else if(!strcmp(args[0], "boot")) {
            wdt_enable(WDTO_15MS);
            while (true);
        }
        else if(!strcmp(args[0], "info")) {
            this->info();
        }
        else if(!strcmp(args[0], "help")) {
            this->help();
        }
        else if(!strcmp(args[0], "status")) {
            this->getStatus();
        }
        else if(!strcmp(args[0], "list-networks")) {
            this->listNetworks();
        }
        else if(!strcmp(args[0], "connect")) {
            this->connect();
        }
        else if(!strcmp(args[0], "reconnect")) {
            this->reconnect();
        }
        else if(!strcmp(args[0], "disconnect")) {
            this->disconnect();
        }
        else if(!strcmp(args[0], "set-autoconnect")) {
            this->setAutoConnect(args[1]);
        }
        else if(!strcmp(args[0], "set-autoreconnect")) {
            this->setAutoReconnect(args[1]);
        }
        else if(!strcmp(args[0], "set-persistence")) {
            this->setPersistence(args[1]);
        }
        else if(!strcmp(args[0], "set-mode")) {
            this->setMode(args[1]);
        }
        else{
            char tx_buffer[256];
            sprintf(tx_buffer, "Unknown Command: %s", args[0]);
            io->println(tx_buffer);
        }
        io->println();
    }
}

void ESP8266Cli::getInput(char *prompt, bool local_echo){
    /* Fills `rx_buffer` with user input */
    io->print(prompt);
    uint8_t idx = 0;
    char rx_buffer[64]; 

    while (true){
        if (!io->available()){
            continue;
        }
        char _byte = io->read();
        if (local_echo || _byte == '\n'){
            io->print(_byte);
        }
        if (_byte == '\n' || idx == 64){
            rx_buffer[idx] = '\0';
            break;
        }
        rx_buffer[idx] = _byte;
        idx++;
    }
    this->parseInput(rx_buffer);
}

void ESP8266Cli::parseInput(char *input){
    uint8_t arg_idx = 0;
    uint8_t arg_count = 0;
    for (uint8_t idx; idx <= strlen(input); idx++){
        char _byte = input[idx];
        if (_byte == ' '){
            args[arg_count][arg_idx] = '\0';
            arg_idx = 0;
            arg_count++;
            continue;
        }
        args[arg_count][arg_idx] = _byte;
        arg_idx++;
    }
}

void ESP8266Cli::help(void){
    io->println("--- COMMANDS ---");
    io->println("help: list all commands");
    io->println("boot: reboot the device using WDT");
    io->println("exit: exit CLI");
    io->println("info: display device and network info");
    io->println("status: display current device status");
    io->println("connect: connect to an access point");
    io->println("reconnect: connect to previous access point");
    io->println("disconnect: disconnect from current access point");
    io->println("list-networks: list available networks");
    io->println("set-mode [NULL|STA|AP|STA+AP]: set device mode");
    io->println("set-autoconnect [true|false]: device will reconnect on power on");
    io->println("set-autoreconnect [true|false]: device will reconnect after connection loss");
    io->println("set-persistence [true|false]: settings will persist after power cycle");
    io->println();
}

void ESP8266Cli::info(void){
    io->println("--- device info ---");
    this->getMode();
    this->getStatus();
    this->getAutoConnect();
    this->getAutoReconnect();
    this->getPersistence();

    io->println("\n--- network info ---");
    io->print("name: ");
    io->println(wifi->SSID());
    io->print("signal strength: ");
    io->println(wifi->RSSI());
    io->print("ip address: ");
    io->println(wifi->localIP());
    io->print("mac address: ");
    io->println(wifi->macAddress());

    io->println("\n--- default network config ---");
    this->getConfig();

    if (!wifi->getPersistent()){
        io->println("\n--- current network config ---");
        this->getConfig(false);
    }
}

void ESP8266Cli::getConfig(bool _default){
    struct station_config conf;

    if (_default){
        wifi_station_get_config_default(&conf);
    }
    else{
        wifi_station_get_config(&conf);
    }
    char ssid[33];
    memcpy(ssid, conf.ssid, sizeof(conf.ssid));
    ssid[32] = '\0';

    char passphrase[65];
    memcpy(passphrase, conf.password, sizeof(conf.password));
    passphrase[64] = '\0';

    io->print("ssid: ");
    io->println(ssid);
    io->print("passphrase: ");
    io->println(passphrase);
}

void ESP8266Cli::connect(void){
    this->getInput("name: ");
    char ssid[64];
    strcpy(ssid, args[0]);
    this->getInput("passphrase: ", false);
    char passphrase[64];
    strcpy(passphrase, args[0]);
    if (passphrase[0] == '\0'){
        wifi->begin(ssid);
    }
    else {
        wifi->begin(ssid, passphrase);
    }

    unsigned long int timeout = millis() + 30000;
    while (millis() < timeout){
        if (wifi->status() != WL_DISCONNECTED){
            io->println();
            this->getStatus();
            return;
        }
        io->print('.');
        delay(1000);
    }
    io->print("\nTimeout: ");
    this->getStatus();
}

void ESP8266Cli::reconnect(void){
    String ap = wifi->SSID();
    if (ap == ""){
        io->println("network not configured");
        return;
    }
    io->print("connecting to ");
    io->println(ap);
    wifi->begin();
    unsigned long int timeout = millis() + 30000;
    while (millis() < timeout){
        if (wifi->status() == WL_CONNECTED){
            io->println();
            this->getStatus();
            return;
        }
        io->print('.');
        delay(1000);
    }
    io->print("\nTimeout: ");
    this->getStatus();
}

void ESP8266Cli::disconnect(void){
    io->print("disconnecting from ");
    io->println(wifi->SSID());
    wifi->disconnect();
    unsigned long int timeout = millis() + 30000;
    while (millis() < timeout){
        if (wifi->status() != WL_CONNECTED){
            io->println();
            this->getStatus();
            return;
        }
        io->print('.');
        delay(1000);
    }
    io->print("\nTimeout: ");
    this->getStatus();
}

void ESP8266Cli::getStatus(void){
    const char* const states[] = { 
        "IDLE", 
        "SSID_NOT_FOUND",
        "SCAN_COMPLETED",
        "CONNECTED",
        "CONNECT_FAILED",
        "CONNECTION_LOST",
        "WRONG_PASSWORD",
        "DISCONNECTED"
    };
    io->print("status: ");
    io->println(states[wifi->status()]);
}

void ESP8266Cli::listNetworks(void){
    uint8_t count = wifi->scanNetworks();
    if (count == WIFI_SCAN_FAILED){
        io->println("Network discovery failed");
    }else{
        char network[128];
        char ssid[32];
        for (uint8_t nw=0; nw < count; nw++){
            wifi->SSID(nw).toCharArray(ssid, 32);
            sprintf(network, "%s (%d)", ssid, wifi->RSSI(nw));
            io->println(network);
        }
    }
}

void ESP8266Cli::setAutoConnect(char *state){
    bool val = (bool)!strcmp(state, "true");
    wifi->setAutoConnect(val);
}
void ESP8266Cli::getAutoConnect(void){
    if (wifi->getAutoConnect()){
        io->println("auto-connect: true");
    }
    else{
        io->println("auto-connect: false");
    }
}
void ESP8266Cli::setAutoReconnect(char *state){
    bool val = (bool)!strcmp(state, "true");
    wifi->setAutoReconnect(val);
}
void ESP8266Cli::getAutoReconnect(void){
    if (wifi->getAutoReconnect()){
        io->println("auto-reconnect: true");
    }
    else{
        io->println("auto-reconnect: false");
    }
}
void ESP8266Cli::setPersistence(char *state){
    bool val = (bool)!strcmp(state, "true");
    wifi->persistent(val);
}
void ESP8266Cli::getPersistence(void){
    if (wifi->getPersistent()){
        io->println("persistence: true");
    }
    else{
        io->println("persistence: false");
    }
}

void ESP8266Cli::getMode(void){
    const char* const modes[] = { 
        "NULL", 
        "STA", 
        "AP", 
        "STA+AP"
    };
    io->print("mode: ");
    io->println(modes[wifi->getMode()]);

}
void ESP8266Cli::setMode(char *state){
    bool sta = false;
    bool ap = false;
    if(!strcmp(state, "STA")){
        sta = true;
    }
    else if(!strcmp(state, "AP")){
        ap = true;
    }
    else if(!strcmp(state, "STA+AP")){
        sta = true;
        ap = true;
    }
    wifi->enableSTA(sta);
    wifi->enableAP(ap);
}