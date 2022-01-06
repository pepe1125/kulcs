// KULCSnyilvántartó szekrény v2.0

#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include "RTClib.h"
#include <AT24Cxx.h>

RTC_DS3231 rtc;
AT24Cxx eep(0x57, 32);
MFRC522 mfrc522(8, 9);   // Create MFRC522 instance

#define SOLENOID 7
#define DOOR 15
#define NYIT bitSet(PORTD,7)
#define ZAR  bitClear(PORTD,7)
#define StopCard mfrc522.PICC_HaltA()
#define StopCrypto mfrc522.PCD_StopCrypto1()

byte mac[] = {
  0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

IPAddress ip(192, 168, 88, 188);
IPAddress myDns(192, 168, 88, 1);
EthernetClient client;

char server[] = "sp.myddns.me";

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 3000;     // delay between updates, in milliseconds

bool key_state[16];
bool state;
byte kulcs;
byte user[4];
byte sp[4] = {0x20, 0xFF, 0xBE, 0x4F};

int address = 0;
byte value;

//Mux control pins
const int s0 = 2;
const int s1 = 3;
const int s2 = 5;
const int s3 = 6;

//Mux in “SIG” pin
const int SIG_pin = A0;
uint8_t readMux(uint8_t channel) {
  uint8_t controlPin[] = {s0, s1, s2, s3};
  uint8_t muxChannel[16][4] = {
    {0, 0, 0, 0},
    //channel 0
    {1, 0, 0, 0},
    //channel 1
    {0, 1, 0, 0},
    //channel 2
    {1, 1, 0, 0},
    //channel 3
    {0, 0, 1, 0},
    //channel 4
    {1, 0, 1, 0},
    //channel 5
    {0, 1, 1, 0},
    //channel 6
    {1, 1, 1, 0},
    //channel 7
    {0, 0, 0, 1},
    //channel 8
    {1, 0, 0, 1},
    //channel 9
    {0, 1, 0, 1},
    //channel 10
    {1, 1, 0, 1},
    //channel 11
    {0, 0, 1, 1},
    //channel 12
    {1, 0, 1, 1},
    //channel 13
    {0, 1, 1, 1},
    //channel 14
    {1, 1, 1, 1}
    //channel 15
  };
  //loop through the 4 sig
  for (uint8_t i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  } //read the value at the SIG pin
  bool val = digitalRead(SIG_pin);
  //return the value
  return val;
}

void setup() {
  pinMode(SOLENOID, OUTPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(DOOR, INPUT_PULLUP);

  Serial.begin(57600);
  SPI.begin();
  mfrc522.PCD_Init();
  if (! rtc.begin()) {
    Serial.println("RTC problem");
    Serial.flush();
  }
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  while (!digitalRead(DOOR));

  Ethernet.init(10);
  // start the Ethernet connection:
  Serial.print("ETH ");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Fail DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("HW error!");
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("cable not connected");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
  } else {
    Serial.print("DHCP IP ");
    Serial.println(Ethernet.localIP());
  }

  for (byte i = 0; i < 3; i++) {
    NYIT;
    delay(50);
    ZAR;
    delay(50);
  }

  for (uint8_t i = 0; i < 4; i++) {
    eep.update(i, (uint8_t)sp[i]);
    // Serial.print(sp[i], HEX);
  }

}

void loop() {

  while (!digitalRead(DOOR));

  digitalWrite(8, HIGH);
  if (!state) {
    for (uint8_t i = 0; i < 16; i++) {
      key_state[i] = readMux(i);
      Serial.print(key_state[i]);
      Serial.print("\t");
    }
    Serial.println();
    state = true;
  }

  for (uint8_t i = 0; i < 16; i++) {
    if (key_state[i] != readMux(i)) {
      state = false;
    }
  }

  MFRC522::MIFARE_Key key;
  for (uint8_t i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  uint8_t block;
  uint8_t len;
  MFRC522::StatusCode status;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  StopCrypto;
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  StopCrypto;
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  DateTime now = rtc.now();

  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.print(" ");

  String tag = "";
  for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
    user[i] = mfrc522.uid.uidByte[i];
    tag += String(user[i], HEX);
  }
  //Serial.println(tag);



  uint8_t auth = 0;
  for (uint8_t i = 0; i < 4; i++) {
    value = eep.read((uint8_t)i);
    Serial.print(value, HEX);
    if (user[i] == sp[i]) {
      auth++;
    }
  }

  if (auth > 3) {
    tone(7, 100, 200);
    delay(220);
    NYIT;
    Serial.print(" UNLOCK: ");
    auth = 0;
  }
  else {
    tone(7, 50, 250);
    StopCard;
    StopCrypto;
    Serial.println(" LOCKED!");
    return;
  }

  uint8_t buffer1[18];
  block = 4;
  len = 18;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("fail: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    delay(100);
    tone(7, 50, 200);
    mfrc522.PCD_Reset();
    mfrc522.PCD_Init();
    StopCard;
    StopCrypto;
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    tone(7, 50, 200);
    delay(250);
    mfrc522.PCD_Reset();
    mfrc522.PCD_Init();
    StopCard;
    StopCrypto;
    return;
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
  }
  Serial.print(" ");

  uint8_t buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    tone(7, 50, 200);
    delay(250);
    mfrc522.PCD_Reset();
    mfrc522.PCD_Init();
    StopCard;
    StopCrypto;
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    tone(7, 50, 200);
    delay(250);
    mfrc522.PCD_Reset();
    mfrc522.PCD_Init();
    StopCard;
    StopCrypto;
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }
  Serial.println();

  delay(400); //change value if you want to read cards faster
  StopCard;
  StopCrypto;

  digitalWrite(8, HIGH); // RFID OFF
  while (digitalRead(DOOR));
  delay(600);

  ZAR;

  Ethernet.init(10);
  httpRequest(tag);
  delay(440);

}

void httpRequest(String http_tag) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP GET request:
    client.print("GET /kulcs/data.php?");
    client.print("tag=");
    client.print(http_tag);
    client.println(" HTTP/1.1");
    client.println("Host: sp.myddns.me");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();
    Serial.println("SEND");

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("failed");
  }
}
