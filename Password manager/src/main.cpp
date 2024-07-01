
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Keypad setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {8, 7, 6};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonPin = 9;
const int ledPin = 10;

bool mode = false; // false for Retrieve, true for Set
String inputPassword = "";
bool verifyCurrentPassword = false; // New flag to indicate current password verification step

// Encryption key
const char* encryptionKey = "simpleKey";
int keyLength = strlen(encryptionKey);

// Encrypt or decrypt a character
char encryptDecrypt(char input, int index) {
  return input ^ encryptionKey[index % keyLength];
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Password manager");
  delay(2000);
  lcd.clear();
  lcd.print("Enter PW- Unlock");
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);
  static int lastButtonState = HIGH;

  if (currentButtonState != lastButtonState) {
    if (currentButtonState == LOW) {
      mode = !mode;
      verifyCurrentPassword = mode; // Require current password verification only in set mode
      lcd.clear();
      lcd.print(mode ? "+New PW (En-OLD)" : "Enter PW- Unlock");
      inputPassword = ""; // Reset password input
    }
    delay(100); // Debounce delay
  }
  lastButtonState = currentButtonState;

  char key = keypad.getKey();
  if (key) {
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(50);  
    if (key == '#') {
      if (mode) {
        if (verifyCurrentPassword) {
          // Check current password
          String storedPassword = "";
          for (int i = 0; EEPROM.read(i) != '\0'; i++) {
            char c = EEPROM.read(i);
            storedPassword += encryptDecrypt(c, i);
          }
          if (storedPassword == inputPassword) {
            lcd.clear();
            lcd.print("Enter New PW");
            verifyCurrentPassword = false; // Current password verified, next input will be new password
          } else {
            lcd.clear();
            lcd.print("Incorrect PW");
            delay(2000);
            lcd.clear();
            lcd.print("Enter Curr PW");
          }
        } else {
          // Set new password
          for (int i = 0; i < inputPassword.length(); i++) {
            char encryptedChar = encryptDecrypt(inputPassword[i], i);
            EEPROM.write(i, encryptedChar);
          }
          EEPROM.write(inputPassword.length(), '\0'); // Null terminator
          lcd.clear();
          lcd.print("Password Set");
          lcd.setCursor(0, 1);
          lcd.print("Successfully");
        }
      } else {
        // Retrieve and decrypt stored password
        String storedPassword = "";
        for (int i = 0; EEPROM.read(i) != '\0'; i++) {
          char c = EEPROM.read(i);
          storedPassword += encryptDecrypt(c, i);
        }
        // Compare stored password with input
        lcd.clear();
        if (storedPassword == inputPassword) {
          lcd.print("Access Granted");
          digitalWrite(ledPin, HIGH);
        } else {
          lcd.print("Access Denied");
        }
      }
      delay(2000); // Display message for 2 seconds
      lcd.clear();
      lcd.print(mode ? "Enter New PW" : "Enter PW- Unlock");
      inputPassword = ""; // Reset password input
    } else {
      inputPassword += key;
      // Display asterisks for input
      String displayPassword = "";
      for (unsigned int i = 0; i < inputPassword.length(); i++) {
        displayPassword += "*";
      }
      lcd.setCursor(0, 1);
      lcd.print(displayPassword);
    }
  }
}