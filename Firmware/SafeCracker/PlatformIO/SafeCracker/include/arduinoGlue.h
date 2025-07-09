#ifndef ARDUINOGLUE_H
#define ARDUINOGLUE_H


//============ Includes ====================
#include <EEPROM.h>
#include <SPI.h>
#include <WiFi.h>

//============ Defines & Macros====================
#define CCW 0
#define CW 1
#define DISCA_START 75
#define DISCB_START 6
#define DISCC_START 50
#define a  1<<0
#define b  1<<6
#define c  1<<5
#define d  1<<4
#define e  1<<3
#define f  1<<1
#define g  1<<2
#define dp 1<<7
#define LOCATION_HOME_OFFSET 0
#define LOCATION_TEST_INDENT_0 1
#define LOCATION_TEST_INDENT_1 2
#define LOCATION_TEST_INDENT_2 3
#define LOCATION_TEST_INDENT_3 4
#define LOCATION_TEST_INDENT_4 5
#define LOCATION_TEST_INDENT_5 6
#define LOCATION_TEST_INDENT_6 7
#define LOCATION_TEST_INDENT_7 8 
#define LOCATION_TEST_INDENT_8 9 
#define LOCATION_TEST_INDENT_9 10
#define LOCATION_TEST_INDENT_10 11
#define LOCATION_TEST_INDENT_11 12
#define LOCATION_INDENT_DIAL_0 13 //Int storage
#define LOCATION_INDENT_DIAL_1 15
#define LOCATION_INDENT_DIAL_2 17
#define LOCATION_INDENT_DIAL_3 19
#define LOCATION_INDENT_DIAL_4 21
#define LOCATION_INDENT_DIAL_5 23
#define LOCATION_INDENT_DIAL_6 25
#define LOCATION_INDENT_DIAL_7 27
#define LOCATION_INDENT_DIAL_8 29
#define LOCATION_INDENT_DIAL_9 31
#define LOCATION_INDENT_DIAL_10 33
#define LOCATION_INDENT_DIAL_11 35
#define LOCATION_SERVO_REST 37 //Ints
#define LOCATION_SERVO_TEST_PRESSURE 39
#define LOCATION_SERVO_HIGH_PRESSURE 41
#define DISC_A_LAST 50
#define DISC_B_LAST 51
#define DISC_C_LAST 52

//============ Extern Variables ============
extern const byte      LED;                               		//-- from SafeCracker
extern const byte      button;                            		//-- from SafeCracker
extern boolean         buttonPressed;                     		//-- from SafeCracker
extern const byte      buzzer;                            		//-- from SafeCracker
extern const byte      currentSense;                      		//-- from SafeCracker
extern boolean         direction;                         		//-- from SafeCracker
extern int             discA;                             		//-- from SafeCracker
extern byte            discAAttempts;                     		//-- from SafeCracker
extern int             discB;                             		//-- from SafeCracker
extern byte            discBAttempts;                     		//-- from SafeCracker
extern int             discC;                             		//-- from SafeCracker
extern byte            discCAttempts;                     		//-- from SafeCracker
extern const byte      displayClock;                      		//-- from SafeCracker
extern const byte      displayData;                       		//-- from SafeCracker
extern const byte      displayLatch;                      		//-- from SafeCracker
extern const byte      encoderA;                          		//-- from SafeCracker
extern const byte      encoderB;                          		//-- from SafeCracker
extern byte            homeOffset;                        		//-- from SafeCracker
extern int             indentDepths[12];                  		//-- from SafeCracker
extern int             indentLocations[12];               		//-- from SafeCracker
extern int             indentWidths[12];                  		//-- from SafeCracker
extern boolean         indentsToTry[12];                  		//-- from SafeCracker
extern byte            maxCAttempts;                      		//-- from SafeCracker
extern const byte      motorDIR;                          		//-- from SafeCracker
extern const byte      motorPWM;                          		//-- from SafeCracker
extern const byte      motorReset;                        		//-- from SafeCracker
extern char            pass[];                            		//-- from SafeCracker
extern const byte      photo;                             		//-- from SafeCracker
extern boolean         previousDirection;                 		//-- from SafeCracker
extern const byte      servo;                             		//-- from SafeCracker
extern const byte      servoPositionButton;               		//-- from SafeCracker
extern char            ssid[];                            		//-- from SafeCracker
extern long            startTime;                         		//-- from SafeCracker
extern int             status;                            		//-- from SafeCracker
extern volatile int    steps;                             		//-- from SafeCracker
extern int             switchDirectionAdjustment;         		//-- from SafeCracker
extern const int       timeMotorStop;                     		//-- from SafeCracker
extern int             combinationsAttempted;             		//-- from cracking

//============ Function Prototypes =========
//-- from cracking.ino -----------
void nextCombination();                                     
boolean checkCrossing(int currentSpot, int changeAmount, int checkSpot);
//-- from display.ino -----------
void clearDisplay();                                        
void showNumber(long number);                               
void showTime(long milliseconds);                           
void showCombination(byte discA, byte discB, byte discC);   
void postNumber(byte number, boolean decimal, boolean inverted);
//-- from ControlFunctions.ino -----------
int gotoStep(int stepGoal, int fullRotations);              
int stepsRequired(int currentSteps, int goal);              
int setDial(int dialValue, int extraSpins);                 
int addToDisc(int discValue, int addValue);                 
void findFlag();                                            
void resetDiscsWithCurrentCombo(boolean pause);             
int convertDialToEncoder(int dialValue);                    
int convertEncoderToDial(int encoderValue);                 
void resetDial();                                           
void setMotorSpeed(int speedValue);                         
int readMotorCurrent();                                     
void turnCW();                                              
void turnCCW();                                             
void enableMotor();                                         
void disableMotor();                                        
void countA();                                              
void countB();                                              
boolean flagDetected();                                     
void announceSuccess();                                     
void messagePause(char* message);                           
void checkForUserPause();                                   
int averageAnalogRead(byte pinToRead);                      
//-- from testing.ino -----------
void positionTesting();                                     

#endif // ARDUINOGLUE_H
