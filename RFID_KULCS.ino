#include <SPI.h>
#include <MFRC522.h>
#define SOLENOID 7
#define NYIT bitSet(PORTD,7)
#define ZAR  bitClear(PORTD,7)

MFRC522 mfrc522(8, 9);   // Create MFRC522 instance

bool key_state[16];
bool state;
byte kulcs;
byte user[4];
byte sp[4] = {0x20, 0xFF, 0xBE, 0x4F};

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

  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card

  for (byte i = 0; i < 3; i++) {
    NYIT;
    delay(50);
    ZAR;
    delay(50);
  }

}

void loop() {

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
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
    user[i] = mfrc522.uid.uidByte[i];
    Serial.print(user[i], HEX);
    Serial.print(" ");
  }

  uint8_t auth = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (user[i] == sp[i]) {
      auth++;
    }
  }

  if (auth > 3) {
    NYIT;
    auth = 0;
  }
  else {
    tone(7, 50, 200);
  }

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  uint8_t buffer1[18];
  block = 4;
  len = 18;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
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
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }
  Serial.println();

  delay(2000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  ZAR;

}
