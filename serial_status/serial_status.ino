const int numFlashlights = 4;
const int numBatteries = 3;

volatile int flashBatts[numFlashlights];
volatile char battState[numFlashlights][numBatteries];

long frame = 0;
const int battDischargeFrames = 60;

// This is used with serialEvent() to feed the main loop
String inputString = "";
boolean inputReady = false;

// These two variables are used for charging. When they are both non-negative then the battery flash level is set
int flashCharging = -1;
int battCharging = -1;
// These two variables are used to determine the timeout for charging
long flashChargeFrame = 0;
long battChargeFrame = 0;
// How long between the flashlight charge detection and the battery charge detection, in frames, that can pass before charging is invalid
long chargeFrameLimit = 300;

// Use this function to set a flashlight's battery level
void setFlashBattLevel(int i, int val)
{
  if (i >= 0 && i < numFlashlights && val >= 0 && val < 256)
  {
    flashBatts[i] = val;
    // TODO: Transmit to flashlight
  }
}

// Use this function to respond to charge a flashlight with a battery
void chargeFlash(int i, int j)
{
  if (i >= 0 && i < numFlashlights && j >= 0 && j < numBatteries)
  {
    setFlashBattLevel(i, 255);
    battState[i][j] = 'X';
  }
}

void setFlashCharging(int i)
{
  if (i >= 0 && i < numFlashlights)
  {
    flashCharging = i;
    flashChargeFrame = frame;
  }
}

void setBattCharging(int j)
{
  if (j >= 0 && j < numBatteries)
  {
    battCharging = j;
    battChargeFrame = frame;
  }
}

void setup()
{
  Serial.begin(9600);
  
  for (int i = 0; i < numFlashlights; ++i)
  {
    for (int j = 0; j < numBatteries; ++j)
    {
      flashBatts[i] = 255;
      // '-' is unused; 'X' is used
      battState[i][j] = '-';
    }
  }
}

void loop()
{
  if (inputReady)
  {
    // Flashlight Set battery Level
    // fsl [i] [level]
    if (inputString.startsWith("fsl"))
    {
      int i = inputString.substring(4,5).toInt();
      int val = inputString.substring(6).toInt();
      setFlashBattLevel(i, val);
    }
    // Flashlight Detect Magnet - simulates a transmission that a flashlight has detected a magnet
    // fdm [i]
    else if (inputString.startsWith("fdm"))
    {
      int i = inputString.substring(4,5).toInt();
      setFlashCharging(i);
    }
    // Battery Detect Switch - simulates a battery switch being hit
    // bds [j]
    else if (inputString.startsWith("bds"))
    {
      int j = inputString.substring(4,5).toInt();
      setBattCharging(j);
    }
    Serial.print(inputString);
    inputString = "";
    inputReady = false;
  }

  // Every discharge cycle, discharge and show results
  if (frame % battDischargeFrames == 0)
  {
    // Note; it's kind of complicated to have the flashlights update the master with their battery level, so just simulate it.
    for (int i = 0; i < numFlashlights; ++i)
    {
      if (flashBatts[i] > 0)
      {
        flashBatts[i] -= 1;
      }
    }
    
    Serial.println("Flashlights:");
    for (int i = 0; i < numFlashlights; ++i)
    {
      Serial.print(i);

      // Print which batteries have been taken
      Serial.print(' ');
      for (int j = 0; j < numBatteries; ++j)
      {
        Serial.print(battState[i][j]);
      }
      
      // Print what the flashlight power level is
      Serial.print(' ');
      Serial.print(flashBatts[i]);
      
      Serial.println();
    }
    
  }
  
  // Check whether or not a flashlight/battery pair should be used
  if (flashCharging >= 0 && battCharging >= 0 && flashCharging < numFlashlights && battCharging < numBatteries)
  {
    if (abs(flashChargeFrame - battChargeFrame) < chargeFrameLimit)
    {
      if (battState[flashCharging][battCharging] == '-')
      {
        chargeFlash(flashCharging, battCharging);
        flashCharging = -1;
        battCharging = -1;
      }
    }
  }
  
  ++frame;
  
  delay(15);
}

void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n')
    {
      inputReady = true;
    }
  }
}
