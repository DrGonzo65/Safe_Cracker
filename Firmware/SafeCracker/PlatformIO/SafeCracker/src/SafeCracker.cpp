/*
  Opening a safe using a motor and a servo
  By: Nathan Seidle @ SparkFun Electronics
  Date: February 24th, 2017

  We use a motor with encoder feedback to try to glean the internal workings of the
  safe to reduce the overall solution domain. Then we begin running through
  solution domain, pulling on the handle and measuring it as we go.

  Motor current turning dial, speed = 255, ~350 to 560mA
  Motor current turning dial, speed = 50, = ~60 to 120mA

  TODO:
  current combination to nvm
*/
#include "SafeCracker.h"

// Add these function prototypes after your includes, before setup() because the Cloud compiler blows
void disableMotor();
void enableMotor();
void turnCW();
void turnCCW();
void countA();
void countB();

void setMotorSpeed(int speedValue);

#include "nvm.h" //EEPROM locations for settings
//#include <EEPROM.h>                               		//-- moved to arduinoGlue.h

//#include <SPI.h>                                  		//-- moved to arduinoGlue.h

//#include <WiFi.h>                                 		//-- moved to arduinoGlue.h

char ssid[] = "trifecta"; //  your network name
char pass[] = "fall straw dance fighting";      // your network password between the " "
int status = WL_IDLE_STATUS;


//Pin definitions
const byte encoderA = 2;
const byte encoderB = 3;
const byte photo = 5;
const byte motorPWM = 6;
const byte button = 7;
const byte motorReset = 8;
const byte servo = 9;
const byte motorDIR = 10;
const byte buzzer = 11;
const byte LED = 13;

const byte currentSense = A0;
const byte servoPositionButton = A1;
const byte displayLatch = A2;
const byte displayClock = A3;
const byte displayData = A4;

	//-- moved to arduinoGlue.h // #define CCW 0
	//-- moved to arduinoGlue.h // #define CW 1

volatile int steps = 0; //Keeps track of encoder counts. 8400 per revolution so this can get big.
boolean direction = CW; //steps goes up or down based on direction
boolean previousDirection = CW; //Detects when direction changes to add some steps for encoder slack
byte homeOffset = 0; //Found by running findFlag(). Stored in nvm.

const int timeMotorStop = 125; //ms for motor to stop spinning after stop command. 200 works

//Because we're switching directions we need to add extra steps to take
//up the slack in the encoder
//The greater the adjustment the more negative it goes
int switchDirectionAdjustment = (84 * 1) + 1; //Use 'Test dial control' to determine adjustment size
//84 * 1 - 20 = Says 34 but is actually 33.5 (undershoot)
//84 * 0 = Says 85 but is actually 85.9 (overshoot)

//A goes in CCW fashion during testing, increments 3 each new try.
//We don't know if this is the center of the notch. If you exhaust the combination domain, adjust up or down one.
	//-- moved to arduinoGlue.h // #define DISCA_START 75

//DiscB goes in CW fashion during testing, increments 3 each new try.
	//-- moved to arduinoGlue.h // #define DISCB_START 6

//DiscC goes in CCW fashion during testing. 12 indentations, 12 raised bits. Indents are 4.17 numbers wide.
	//-- moved to arduinoGlue.h // #define DISCC_START 50

//These are the combination numbers we are attempting
int discA = DISCA_START;
int discB = DISCB_START;
int discC = DISCC_START;

//Keeps track of the combos we need to try for each disc
//byte maxAAttempts = 33; //Assume solution notch is 3 digits wide
//byte maxBAttempts = 33; //Assume solution notch is 3 digits wide
byte maxCAttempts = 0; //Calculated at startup

//Keeps track of how many combos we've tried on a given disc
byte discAAttempts = 0;
byte discBAttempts = 0;
byte discCAttempts = 0;

long startTime; //Used to measure amount of time taken per test

boolean buttonPressed = false; //Keeps track of the 'GO' button.

boolean indentsToTry[12]; //Keeps track of the indents we want to try
int indentLocations[12]; //Encoder click for a given indent
int indentWidths[12]; //Calculated width of a given indent
int indentDepths[12]; //Not really used

void setup()
{
  Serial.begin(115200);
  while (!Serial) {

    ; // wait for serial port to connect. Needed for native USB port only

  }
  
  
  Serial.println("Attempting to connect to open network...");
  status = WiFi.begin(ssid, pass);
  if (status != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    while (true); // don't continue
  } else {
    Serial.print("Connected to the network");
  }

  Serial.println();
  Serial.println();
  Serial.println("Safe Cracker");
  //syslog.log(FAC_USER, PRI_INFO, "SafeCracker starting");


  pinMode(motorReset, OUTPUT);
  disableMotor();

  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);

  pinMode(motorPWM, OUTPUT);
  pinMode(motorDIR, OUTPUT);

  pinMode(LED, OUTPUT);

  pinMode(currentSense, INPUT);
  pinMode(servoPositionButton, INPUT_PULLUP);

  pinMode(photo, INPUT_PULLUP);

  pinMode(button, INPUT_PULLUP);

  pinMode(displayClock, OUTPUT);
  pinMode(displayLatch, OUTPUT);
  pinMode(displayData, OUTPUT);

  digitalWrite(displayClock, LOW);
  digitalWrite(displayLatch, LOW);
  digitalWrite(displayData, LOW);

  //Setup the encoder interrupts.
  attachInterrupt(digitalPinToInterrupt(encoderA), countA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderB), countB, CHANGE);

  //Load settings from EEPROM
  homeOffset = EEPROM.read(LOCATION_HOME_OFFSET); //After doing a findFlag calibration, adjust this number up or down until dial is at zero
  Serial.print(F("Home Offset: "));
  Serial.println(homeOffset);
  
  randomSeed(analogRead(A5));

  //Calculate how many indents we need to attempt on discC
  //maxCAttempts = 0;
  //for (byte x = 0 ; x < 12 ; x++)
  //  if (indentsToTry[x] == true) maxCAttempts++;

  //At startup discB may be negative. Fix it.
  if (discB < 0) discB += 100;

  //Tell dial to go to zero
  enableMotor(); //Turn on motor controller
  Serial.print(F("Enable motor"));
  setMotorSpeed(255);
  Serial.println();
  findFlag(); //Find the flag
  //Adjust steps with the real-world offset
  steps = (84 * homeOffset); //84 * the number the dial sits on when 'home'
  setDial(0, false); //Make dial go to zero

  clearDisplay();
  showCombination(0, 0, 0); //Display zeroes
}

void loop()
{
  static bool autoStarted = false;
  unsigned long startWaitTime = millis();
  char incoming;

  
  // Check if we should auto-start (no interactive user)
  while (!Serial.available() && !buttonPressed && !autoStarted)
  {
    // If no input after 5 seconds, auto-start cracking
    if (millis() - startWaitTime > 5000) {
      Serial.println("No user input detected. Auto-starting...");
      autoStarted = true;
      startTime = millis();
      
      // Jump to cracking loop
      incoming = 's';
      break;
    }
    
    // Check button
    if (digitalRead(button) == LOW) {
      delay(50);
      if (digitalRead(button) == LOW) {
        buttonPressed = true;
        break;
      }
    }
    delay(10);
  }
  //Fetch last combo
  Serial.println("Last combo from memory: ");
  Serial.println(EEPROM.read(DISC_A_LAST));
  Serial.println(EEPROM.read(DISC_B_LAST));
  Serial.println(EEPROM.read(DISC_C_LAST));
  //syslog.printf(FAC_USER, PRI_INFO, "Last combo from memory %d/%d/%d", EEPROM.read(DISC_A_LAST), EEPROM.read(DISC_B_LAST), EEPROM.read(DISC_C_LAST));
  
  //Main serial control menu
  Serial.println();
  Serial.print(F("Combo to start at: "));
  Serial.print(discA);
  Serial.print("/");
  Serial.print(discB);
  Serial.print("/");
  Serial.print(discC);
  Serial.println();
  //syslog.printf(FAC_USER, PRI_INFO, "Combo to start at: %d/%d/%d", discA, discB, discC);


  Serial.println(F("1) Go home and reset dial"));
  Serial.println(F("2) Test dial control"));
  Serial.println(F("3) Reset Dial"));
  Serial.println(F("4) Set starting combos"));
  Serial.println(F("9) Test rotation"));
  Serial.println(F("s) Start cracking"));

  while (!Serial.available())
  {
    if (digitalRead(button) == LOW)
    {
      delay(50);
      if (digitalRead(button) == LOW)
      {
        buttonPressed = true;
        break;
      }
      else
      {
        buttonPressed = false;
      }
    }
    else
    {
      buttonPressed = false;
    }
  }

  if (buttonPressed == true)
  {
    Serial.println(F("Button pressed!"));

    while (digitalRead(button) == false); //Wait for user to stop pressing button
    buttonPressed = false; //Reset variable

    incoming = 's'; //Act as if user pressed start cracking
  }
  else
  {
    incoming = Serial.read();
  }

  if (incoming == '1')
  {
    //Go to starting conditions
    findFlag(); //Detect the flag and center the dial

    Serial.print(F("Home offset is: "));
    Serial.println(homeOffset);

    int zeroLocation = 0;
    while (1) //Loop until we have good input
    {
      Serial.print(F("Enter where dial is actually at: "));

      while (!Serial.available()); //Wait for user input

      Serial.setTimeout(1000); //Must be long enough for user to enter second character
      zeroLocation = Serial.parseInt(); //Read user input

      Serial.print(zeroLocation);
      if (zeroLocation >= 0 && zeroLocation <= 99) break;
      Serial.println(F(" out of bounds"));
    }

    homeOffset = zeroLocation;

    Serial.print(F("\n\rSetting home offset to: "));
    Serial.println(homeOffset);

    EEPROM.write(LOCATION_HOME_OFFSET, homeOffset);

    //Adjust steps with the real-world offset
    steps = (84 * homeOffset); //84 * the number the dial sits on when 'home'

    setDial(0, false); //Turn to zero

    Serial.println(F("Dial should be at: 0"));
  }
  else if (incoming == '2')
  {
    positionTesting(); //Pick random places on dial and prove we can go to them
  }
  else if (incoming == '3')
  {
    resetDial();
  }
  else if (incoming == '4')
  {
    //Set starting combos
    for (byte x = 0 ; x < 3 ; x++)
    {
      int combo = 0;
      while (1) //Loop until we have good input
      {
        Serial.print(F("Enter Combination "));
        if (x == 0) Serial.print("A");
        else if (x == 1) Serial.print("B");
        else if (x == 2) Serial.print("C");
        Serial.print(F(" to start at: "));
        while (!Serial.available()); //Wait for user input

        Serial.setTimeout(1000); //Must be long enough for user to enter second character
        combo = Serial.parseInt(); //Read user input

        if (combo >= 0 && combo <= 99) break;

        Serial.print(combo);
        Serial.println(F(" out of bounds"));
      }
      Serial.println(combo);
      if (x == 0) discA = combo;
      else if (x == 1) discB = combo;
      else if (x == 2) discC = combo;
    }

  }
  else if (incoming == '9')
  {

    setDial(0, false); //Turn to zero without extra spin

    turnCCW();
    Serial.print("Setting dial to 20, with 4 spins");
    setDial(20, 4);
    delay(5000);
    
    turnCW();
    Serial.print("Setting dial to 62, with 3 spins");
    setDial(62, 2);
    delay(5000);

    turnCCW();
    Serial.print("Setting dial to 92, with 2 spins");
    setDial(92, 1);
    delay(5000);
    
    turnCW();
    Serial.print("Setting dial to 0, with 0 spins");
    setDial(0, 1);
    delay(5000);

  }
  else if (incoming == 's') //Start cracking!
  {
    clearDisplay();
    showCombination(discA, discB, discC); //Update display

    startTime = millis();

    //Set the discs to the current combinations (user can set if needed from menu)
    resetDiscsWithCurrentCombo(false); //Do not pause with messages

    while (1)
    {
      for (int dA = 0 ; dA < 34 ; dA++) {
        discA = addToDisc(discA, 3);
        for (int dB = 0 ; dB < 34 ; dB++) {
          discB = addToDisc(discB, 3);
          for (int dC = 0 ; dC < 34 ; dC++) {
            discC = addToDisc(discC, 3);
            nextCombination(); //Try the next combo!
            if (Serial.available())
            {
              byte incoming = Serial.read();
              if (incoming == 'p')
              {
                Serial.println(F("Pausing"));
                while (!Serial.available());
                Serial.read();
                Serial.println(F("Running"));
              }
              else if (incoming == 'x' || incoming == 's')
              {
                Serial.println(F("Cracking stopped"));
                break; //User wants to stop
              }
            }
          }
        }
      }
    } //End combination loop
  } //End incoming == 's'
}