/*
  These functions are used to calibrate the dialer
*/


//Test to see if we can repeatably go to a dial position
//Turns dial to random CW and CCW position and asks user to verify.
//How to use: Attach cracker to safe. Home the dial using the menu function. Then run this
//and verify the dial goes where it says it is going. If it's wrong, check homeOffset variable.
//If one direction is off, check switchDirectionAdjustment variable.
void positionTesting()
{
  int randomDial;

  for (int x = 0 ; x < 2 ; x++)
  {
    randomDial = random(0, 100);
    randomDial = 25;
    turnCCW();
    setDial(randomDial, false);

    Serial.print(F("Dial should be at: "));
    Serial.println(convertEncoderToDial(steps));
    messagePause("Verify then press key to continue");

    randomDial = random(0, 100);
    randomDial = 75;
    turnCW();
    setDial(randomDial, false);

    Serial.print(F("Dial should be at: "));
    Serial.println(convertEncoderToDial(steps));
    messagePause("Verify then press key to exit");
  }
}

