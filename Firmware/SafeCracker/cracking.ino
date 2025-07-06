/*
  These are the actual high level cracking functions

  nextCombination() - Every time this is called, we attempt the current combo and then advance to next

*/
int combinationsAttempted = 0;

//Given the current state of discs, advance to the next combo
void nextCombination()
{
  combinationsAttempted++; //Increase the overall count

  // Setting combo to memory
  EEPROM.write(DISC_A_LAST, discA);
  EEPROM.write(DISC_B_LAST, discB);
  EEPROM.write(DISC_C_LAST, discC);

  turnCCW();
  //Serial.print("Setting DiscA to ");
  //Serial.println(discA);
  setDial(discA, 4);
  turnCW();
  //Serial.print("Setting DiscB to ");
  //Serial.println(discB);
  setDial(discB, 2);
  turnCCW();
  //Serial.print("Setting DiscC to ");
  //Serial.println(discC);
  setDial(discC, 1);
  turnCW();
  //Serial.println("Trying to open");
  int goRight = discC+30;
  if (goRight > 100)
  { 
    goRight = goRight - 100;
  }

  // Set initial position before turning
  int initialPosition = steps;
  unsigned long moveStartTime = millis();
  
  // Start turning at medium speed
  setMotorSpeed(150);
  
  // Check if dial is actually moving
  delay(100); // Give it a moment to start
  
  // Monitor if dial gets stuck (door unlocked)
  for (int i = 0; i < 10; i++) {
    int currentPosition = steps;
    delay(50);
    
    // If position hasn't changed for several checks, the door is likely open
    if (abs(currentPosition - steps) < 10) { // Minimal or no movement
      if (i > 3) { // Multiple consecutive "stuck" readings
        Serial.println();
        Serial.println("Door is open!!! Dial stuck at position.");
        announceSuccess();
        disableMotor();
        while(1); // Freeze
      }
    } else {
      i = 0; // Reset counter if movement detected
    }
  }
  // If no stall detected, complete the combination
  //Serial.println(goRight);
  setDial(goRight, 1);

  showCombination(discA, discB, discC); //Update display

  //Serial.print("Time, ");
  Serial.print(millis()); //Show timestamp

  Serial.print(", Combo, ");
  Serial.print(discA);
  Serial.print("/");
  Serial.print(discB);
  Serial.print("/");
  Serial.print(discC);

  //Try the handle
  if (false)
  {

    Serial.println();
    Serial.println("Door is open!!!");
    announceSuccess();
    disableMotor(); //Power down motor

    Serial.println(F("Pausing. Press key to release handle."));
    while (!Serial.available());
    Serial.read();

    //Return to resting position

    while (1); //Freeze!
  }

  //Serial.print(", Handle position, ");
  //Serial.print(handlePosition);

  Serial.print(", attempt, ");
  Serial.print(combinationsAttempted);

  float secondsPerTest = (float)(millis() - startTime) / 1000.0 / combinationsAttempted;
  Serial.print(", seconds per attempt, ");
  Serial.print(secondsPerTest);

  Serial.println();
}

//Given a disc position, and a change amount (negative is allowed)
//Do we cross over the check spot?
//The logic as it sits will have checkCrossing(10, -3, 7) return false, meaning
//the cracker will try a combination with two same numbers (ie: 7/7/0)
boolean checkCrossing(int currentSpot, int changeAmount, int checkSpot)
{
  //Look at each step as we make a theoretical move from current to the check location
  for (int x = 0 ; x < abs(changeAmount) ; x++)
  {
    if (currentSpot == checkSpot) return (true); //If we make this move it will disrupt the disc with checkSpot

    if (changeAmount < 0) currentSpot--;
    else currentSpot++;

    if (currentSpot > 99) currentSpot = 0;
    if (currentSpot < 0) currentSpot = 99;
  }

  return (false);

}

