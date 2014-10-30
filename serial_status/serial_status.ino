const int numFlashlights = 3;
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
long flashChargeMilli = 0;
long battChargeMilli = 0;
// How long between the flashlight charge detection and the battery charge detection, in millis, that can pass before charging is invalid
long chargeMilliLimit = 5000;

const byte syncCode = 128;
const byte blackoutCode = 64;

// Use this function to set a flashlight's battery level
// Note that this is different from setting flashBatts to a received value
// this is syncing both master and slave to the same value
void setFlashBattLevel(int i, int val)
{
  if (i >= 0 && i < numFlashlights && val >= 0 && val < 256)
  {
    if (val == syncCode)
    {
      ++val;
    }
    else if (val == blackoutCode)
    {
      ++val;
    }
    
    flashBatts[i] = val;
    // TODO: Transmit to flashlight
  }
}


// Use this function to charge a flashlight with a battery, i.e. set in TX mode
void chargeFlash(int i, int j)
{
  if (i >= 0 && i < numFlashlights && j >= 0 && j < numBatteries)
  {
    Serial.println("charging f" + String(i) + " b" + String(j));
    setFlashBattLevel(i, 255);
    battState[i][j] = 'X';
  }
}

// Try to charge a flashlight if battery and flashlight are set properly
void tryCharge()
{
  // Check whether or not a flashlight/battery pair should be used
  if (flashCharging >= 0 && battCharging >= 0 && flashCharging < numFlashlights && battCharging < numBatteries)
  {
    if (max(flashChargeMilli, battChargeMilli) - min(flashChargeMilli, battChargeMilli) < chargeMilliLimit)
    {
      if (battState[flashCharging][battCharging] == '-')
      {
        chargeFlash(flashCharging, battCharging);
        flashCharging = -1;
        battCharging = -1;
      }
    }
  }
}

// Set it so that the flashlight was detected as charging
void setFlashCharging(int i)
{
  if (i >= 0 && i < numFlashlights)
  {
    Serial.println("charging f" + String(i));
    flashCharging = i;
    flashChargeMilli = millis();
    tryCharge();
  }
}

// Set it so that the battery was detected as charging
void setBattCharging(int j)
{
  if (j >= 0 && j < numBatteries)
  {
    Serial.println("charging b" + String(j));
    battCharging = j;
    battChargeMilli = millis();
    tryCharge();
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
    Serial.print(inputString);
    
    // Show status
    if (inputString.startsWith("ls"))
    {
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
        Serial.print(" [");
        
        for (int level = 0; level < 250; level += 50)
        {
          if (level < flashBatts[i])
          {
            Serial.print("|");
          }
          else
          {
            Serial.print(" ");
          }
        }
        
        Serial.println("]");
      }
    }
    // Flashlight Set battery Level
    // fsl [i] [level]
    else if (inputString.startsWith("fsl"))
    {
      int i = inputString.substring(4,5).toInt();
      int val = inputString.substring(6).toInt();
      setFlashBattLevel(i, val);
    }
    // Flashlight Detect Charging - simulates a transmission that a flashlight has detected a magnet
    // fdc [i]
    else if (inputString.startsWith("fdc"))
    {
      int i = inputString.substring(4,5).toInt();
      setFlashCharging(i);
    }
    // Battery Detect Charging - simulates a battery switch being hit
    // bdc [j]
    else if (inputString.startsWith("bdc"))
    {
      int j = inputString.substring(4,5).toInt();
      setBattCharging(j);
    }
    // BlackOut
    // bo
    else if (inputString.startsWith("bo"))
    {
      // TODO: Transmit a blackout bit to all flashlights
    }
    inputString = "";
    inputReady = false;
  }

  // Every discharge cycle, discharge. This is temp
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

