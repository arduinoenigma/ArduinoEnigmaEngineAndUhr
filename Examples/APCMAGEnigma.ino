//this configuration decodes the encrypted text at 
//http://apcmag.com/arduino-projects-enigma-cipher-machine.htm

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#ifndef INT16U
#define INT16U unsigned short
#endif

// Rotor definitions, first two letters are the letter at which they rotate the one to the
// left, the rest are the output given an input letter
// rotors Beta & Gamma can only be used in the fourth position.
// the wheels go as follow:
// UKW,ROTOR,ROTOR,ROTOR,ROTOR,EKW
// M3
// byte WHEELTYPE[5] = {11, 1, 2, 3, 0, 12};
// M4 compatible with M3, set wheel 4 to A
// byte WHEELTYPE[5] = {11, 1, 2, 3, 4, 12};

//               ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define ROTOR1 R-EKMFLGDQVZNTOWYHXUSPAIBRCJ
#define ROTOR2 F-AJDKSIRUXBLHWTMCQGZNPYFVOE
#define ROTOR3 W-BDFHJLCPRTXVZNYEIWGAKMUSQO
#define ROTOR4 K-ESOVPZJAYQUIRHXLNFTGKDCMWB
#define ROTOR5 A-VZBRGITYUPSDNHLXAWMJQOFECK
#define ROTOR6 ANJPGVOUMFYQBENHZRDKASXLICTW
#define ROTOR7 ANNZJHGRCXMYSWBOUFAIVLPEKQDT
#define ROTOR8 ANFKQHTLXOCBJSPDZRAMEWNIUYGV
#define ROTORB --LEYJVCNIXWPBQMDRTAKZGFUHOS
#define ROTORG --FSOKANUERHMBTIYCWLQPZXVGJD
#define    ETW --ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define   UKWA --EJMZALYXVBWFCRQUONTSPIKHGD
#define   UKWB --YRUHQSLDPXNGOKMIEBFZCWVJAT
#define   UKWC --FVPJIAOYEDRZXWGCTKUQSBNMHL
#define  UKWBD --ENKQAUYWJICOPBLMDXZVFTHRGS
#define  UKWCD --RDOBJNTKVEHMLFCWZAXGYIPSUQ

const __FlashStringHelper * WHEELSF;
const __FlashStringHelper * UHRSF;
const __FlashStringHelper * UHRPLUGSF;

char EffSTECKER[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
byte WHEELPOS[4] = {'A', 'A', 'A', 'A'};

// wheel types, first entry is for rightmost wheel, last is reflector
// fifth entry can be 0 for a three rotor machine.
//byte WHEELTYPE[6] = {11, 3, 2, 1, 0, 12};
//byte WHEELTYPE[6] = {11, 3, 2, 1, 9, 15};

//default M3 1939 in enigma simulator app
//byte WHEELTYPE[6] = {11, 3, 2, 1, 0, 12};
//byte WHEELPOS[4] = {0, 'A', 'A', 'A'};

//default M4 1942 in enigma simulator app
//byte WHEELTYPE[6] = {11, 3, 2, 1, 9, 15};
//byte WHEELPOS[4] = {'A', 'A', 'A', 'A'};

// test machine
//byte WHEELTYPE[6] = {11, 1, 1, 1, 1, 15};
//byte WHEELPOS[4] = {'A', 'A', 'A', 'A'};

char KeyPressed = 0;
char EncodedKey = 0;

byte SerialRead = 0;
byte SerialSetWheels = 0;

struct enigmaData_t
{
  byte SerialFunction;
  byte Uhr;
  char PAIRS[27];
  char STECKER[27];
  byte WHEELTYPE[6];
  byte WHEELPOS[4];
  byte ROTORPOS[4];
}
EnigmaData;


void initEnigma()
{
  // set to 1 to monitor or 2 to display encoded character only
  EnigmaData.SerialFunction = 2;
  
  //ALL THE STEPS SHOWN BELOW MUST BE DONE TO INITIALIZE THE MACHINE
  
  //Rotor Types for an M4 machine, see more examples above
  EnigmaData.WHEELTYPE[0] = 11;    // ENTRY CONTACTS: ETW
  EnigmaData.WHEELTYPE[1] = 3;     // RIGHTMOST ROTOR
  EnigmaData.WHEELTYPE[2] = 2;     // MIDDLE ROTOR
  EnigmaData.WHEELTYPE[3] = 1;     // LEFT ROTOR
  EnigmaData.WHEELTYPE[4] = 0;     // ADDITIONAL WHEEL (M4 only)
  EnigmaData.WHEELTYPE[5] = 13;    // REFLECTOR: UKW

  //Wheel Positions
  EnigmaData.WHEELPOS[0] = 0;      // LEFTMOST LETTER ON M4
  EnigmaData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
  EnigmaData.WHEELPOS[2] = 'P';    // MIDDLE LETTER
  EnigmaData.WHEELPOS[3] = 'C';    // RIGHTMOST LETTER

  //Ring Settings
  EnigmaData.ROTORPOS[0] = 0;      // LEFTMOST ROTOR SETTING ON M4
  EnigmaData.ROTORPOS[1] = 13;     // LEFTMOST ROTOR SETTING ON M3
  EnigmaData.ROTORPOS[2] = 1;      // MIDDLE ROTOR SETTING
  EnigmaData.ROTORPOS[3] = 7;      // RIGHTMOST ROTOR SETTING

  for (byte i = 0; i < 4; i++)
  {
    WHEELPOS[i] = EnigmaData.WHEELPOS[i];
  }

  //Initialize stecker with no plugs
  RemoveAllPlugs();
  
  //ADD ANY PLUG PAIRS HERE USING THE AddPlug function
  AddPlug('D', 'Y');
  AddPlug('A', 'P');
  AddPlug('C', 'M');

  // set the UHR (non zero values used only if 10 plug pairs are installed)
  EnigmaData.Uhr = 0;
  CalculateUhrStecker();
}

void setup()
{
  Serial.begin(9600);

  WHEELSF = F(STR(ROTOR1) STR(ROTOR2) STR(ROTOR3) STR(ROTOR4) STR(ROTOR5) STR(ROTOR6) STR(ROTOR7) STR(ROTOR8) STR(ROTORB) STR(ROTORG) STR(ETW) STR(UKWA) STR(UKWB) STR(UKWC) STR(UKWBD) STR(UKWCD));
  UHRSF = F("\x06\x1F\x04\x1D\x12\x27\x10\x19\x1E\x17\x1C\x01\x26\x0B\x24\x25\x1A\x1B\x18\x15\x0E\x03\x0C\x11\x02\x07\x00\x21\x0A\x23\x08\x05\x16\x13\x14\x0D\x22\x0F\x20\x09");
  UHRPLUGSF = F("\x06\x00\x07\x05\x01\x08\x04\x02\x09\x03");

  initEnigma();
  ShowRotors();
}


void loop()
{
  if (Serial.available())
  {
    KeyPressed = Serial.read();

    if (KeyPressed == '!')
    {
      if (EnigmaData.WHEELTYPE[4] == 0)
      {
        SerialSetWheels = 3;
      }
      else
      {
        SerialSetWheels = 4;
      }
    }

    KeyPressed = KeyPressed & (255 - 32);

    if ((KeyPressed > 'A' - 1) && (KeyPressed < 'Z' + 1))
    {
      SerialRead = 1;
      if (SerialSetWheels)
      {
        if (WHEELPOS[4 - SerialSetWheels] != KeyPressed)
        {
          WHEELPOS[4 - SerialSetWheels] = KeyPressed;
        }
        SerialSetWheels--;
      }
      else
      {
        MoveWheels();
        EncodedKey = EncodeKey(KeyPressed);
        if (EnigmaData.SerialFunction == 2)
        {
          Serial.print(EncodedKey);
        }
      }
    }
  }

  if ((SerialRead) && (!Serial.available()))
  {
    SerialRead = 0;
  }
}

byte SteckerPairs()
{
  byte c = 0;

  for (byte i = 0; i < 26; i++)
  {
    if (EnigmaData.STECKER[i] != (65 + i))
    {
      c++;
    }
  }

  return (c / 2);
}

void RemoveAllPlugs()
{
  for (byte i = 0; i < 27; i++)
  {
    EnigmaData.PAIRS[i] = 0;
  }

  strcpy(EnigmaData.STECKER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EnigmaData.Uhr = 0;
}

void AddPlug(char PlugKey1, char PlugKey2)
{
  if ((SteckerPairs() < 13) && (PlugKey1 != PlugKey2))
  {
    EnigmaData.PAIRS[SteckerPairs() * 2] = PlugKey1;
    EnigmaData.PAIRS[SteckerPairs() * 2 + 1] = PlugKey2;
  }

  EnigmaData.STECKER[PlugKey1 - 65] = PlugKey2;
  EnigmaData.STECKER[PlugKey2 - 65] = PlugKey1;
}

void CalculateUhrStecker()
{
  const char PROGMEM *uhrptr = (const char PROGMEM *)UHRSF;
  const char PROGMEM *uhrplugptr = (const char PROGMEM *)UHRPLUGSF;

  byte ndx = 0;

  if (SteckerPairs() == 10)
  {
    for (byte i = 0; i < 26; i++)
    {
      EffSTECKER[i] = 65 + i;
    }

    for (byte i = 0; i < 10; i++)
    {
      byte pin = 0;
      byte pinright = 0;
      byte pinleft = 0;

      pin = EnigmaData.Uhr + i * 4;
      if (pin > 39)
      {
        pin -= 40;
      }

      for (byte j = 0; j < 40; j++)
      {
        if (pgm_read_byte(uhrptr + j) == pin)
        {
          pinleft = j;
        }
      }

      pinright = pgm_read_byte(uhrptr + pin);

      //these two need to be signed, see <0 below
      //char is signed -127..+128
      char plugright;
      char plugleft;

      plugright = (pinright - (EnigmaData.Uhr + 2));
      if (plugright < 0)
      {
        plugright += 40;
      }
      plugright = plugright / 4;

      plugleft = (pinleft - (EnigmaData.Uhr + 2));
      if (plugleft < 0)
      {
        plugleft += 40;
      }
      plugleft = plugleft / 4;

      EffSTECKER[EnigmaData.PAIRS[i * 2] - 65] = EnigmaData.PAIRS[pgm_read_byte(uhrplugptr + plugright) * 2 + 1];
      EffSTECKER[EnigmaData.PAIRS[pgm_read_byte(uhrplugptr + i) * 2 + 1] - 65] = EnigmaData.PAIRS[plugleft * 2];
    }
  }
  else
  {
    for (byte i = 0; i < 26; i++)
    {
      EffSTECKER[i] = EnigmaData.STECKER[i];
    }
  }

  if (EnigmaData.SerialFunction == 1)
  {
    Serial.println(F("Stecker/Uhr:"));
    Serial.println(EffSTECKER);
  }
}

void MoveWheels()
{
  byte i = 4;
  bool carry = true;

  do
  {
    i--;

    if (carry)
    {
      WHEELPOS[i]++;
      if (i > 1)
      {
        carry = IsCarry(EnigmaData.WHEELTYPE[4 - i], WHEELPOS[i]);
      }
      else
      {
        carry = false;
      }
    }
    else
    {
      // double stepping on second wheel
      if (i == 2)
      {
        byte w2 = WHEELPOS[2] + 1;

        if (w2 > 'Z')
        {
          w2 = 'A';
        }

        if (IsCarry(EnigmaData.WHEELTYPE[2], w2))
        {
          WHEELPOS[2]++;
          carry = true;
        }
      }
    }

    if (WHEELPOS[i] > 'Z')
    {
      WHEELPOS[i] = 'A';
      carry = IsCarry(EnigmaData.WHEELTYPE[4 - i], WHEELPOS[i]) || carry;
      if (i == 1)
      {
        carry = false;
      }
    }

  } while (i > 0);

}

void ShowRotors()
{
  const char PROGMEM *charptr = (const char PROGMEM *)WHEELSF;
  char k;
  INT16U wheeltype;

  if (EnigmaData.SerialFunction == 1)
  {
    Serial.println(F("Rotors:"));

    for (byte i = 0; i < 6; i++)
    {
      if (EnigmaData.WHEELTYPE[i] != 0)
      {
        switch (i)
        {
          case 0:
            {
              Serial.println(F("ETW"));
              break;
            }

          case 1:
          case 2:
          case 3:
          case 4:
            {
              Serial.print(F("R"));
              Serial.println((char)('0' + i));
              break;
            }

          case 5:
            {
              Serial.println(F("UKW"));
              break;
            }
        }

        wheeltype = ((EnigmaData.WHEELTYPE[i] - 1) * 28) + 2;

        for (byte i = 0; i < 26; i++)
        {
          k = pgm_read_byte(charptr + wheeltype + i);
          Serial.print(k);
        }
        Serial.println(F(""));
      }
    }
  }
}

byte SerialMonitorStatus;

void SerialMonitor(char k)
{
  if (k == 0)
  {
    SerialMonitorStatus = 0;
  }
  else
  {
    if (EnigmaData.SerialFunction == 1)
    {
      SerialMonitorStatus++;

      //skip R4 if a three wheel machine
      if (((EnigmaData.WHEELTYPE[5] > 11) && (EnigmaData.WHEELTYPE[5] < 15)) && ((SerialMonitorStatus == 6) || (SerialMonitorStatus == 8)))
      {
        SerialMonitorStatus++;
      }

      Serial.print(k);

      switch (SerialMonitorStatus)
      {
        case 1:
        case 13:
          {
            Serial.print(F(">Stecker>"));
            break;
          }

        case 2:
        case 12:
          {
            Serial.print(F(">ETW>"));
            break;
          }

        case 3:
        case 11:
          {
            Serial.print(F(">R1>"));
            break;
          }

        case 4:
        case 10:
          {
            Serial.print(F(">R2>"));
            break;
          }

        case 5:
        case 9:
          {
            Serial.print(F(">R3>"));
            break;
          }

        case 6:
        case 8:
          {
            Serial.print(F(">R4>"));
            break;
          }

        case 7:
          {
            Serial.print(F(">UKW>"));
            break;
          }
        default:
          {
            Serial.println(F(""));
          }
      }
    }
  }
}

bool IsCarry(byte wheelType, byte wheelPos)
{
  const char PROGMEM *charptr = (const char PROGMEM *)WHEELSF;
  INT16U wheeltype = (wheelType - 1) * 28;
  byte k1 = pgm_read_byte(charptr + wheeltype);
  byte k2 = pgm_read_byte(charptr + wheeltype + 1);
  bool v = false;

  if ((wheelPos == k1) || (wheelPos == k2))
  {
    v = true;
  }

  return v;
}

char EncodeKey(char key)
{
  const char PROGMEM *charptr = (const char PROGMEM *)WHEELSF;
  char k, k1;
  INT16U wheeltype;

  SerialMonitor(0);
  SerialMonitor(key);

  k = EffSTECKER[key - 'A'];

  SerialMonitor(k);

  for (byte i = 0; i < 6; i++)
  {
    if (EnigmaData.WHEELTYPE[i] != 0)
    {
      if ((i > 0) && (i < 5))
      {
        byte p = WHEELPOS[4 - i] - (EnigmaData.ROTORPOS[4 - i] - 1);
        if (p < 'A')
        {
          p += 26;
        }

        k = k + (p - 'A');
      }

      if (k > 'Z')
      {
        k = k - ('Z' + 1);
      }
      else
      {
        k = k - 'A';
      }

      wheeltype = ((EnigmaData.WHEELTYPE[i] - 1) * 28) + k + 2;
      k = pgm_read_byte(charptr + wheeltype);

      if ((i > 0) && (i < 5))
      {
        byte p = WHEELPOS[4 - i] - (EnigmaData.ROTORPOS[4 - i] - 1);
        if (p < 'A')
        {
          p += 26;
        }

        k = k - (p - 'A');
      }

      if (k < 'A')
      {
        k = k + 26;
      }

      SerialMonitor(k);
    }
  }

  //after reflector

  for (byte i = 0; i < 5; i++)
  {
    if (EnigmaData.WHEELTYPE[4 - i] != 0)
    {
      if (i < 4)
      {
        byte p = WHEELPOS[i] - (EnigmaData.ROTORPOS[i] - 1);
        if (p < 'A')
        {
          p += 26;
        }

        k = k + (p - 'A');
      }

      if (k > 'Z')
      {
        k = k - 26;
      }

      wheeltype = (EnigmaData.WHEELTYPE[4 - i] - 1) * 28;
      for (byte j = 0; j < 26; j++)
      {
        if ((pgm_read_byte(charptr + wheeltype + j + 2)) == k)
        {
          k1 = 'A' + j;
        }
      }

      k = k1;

      if (i < 4)
      {
        byte p = WHEELPOS[i] - (EnigmaData.ROTORPOS[i] - 1);
        if (p < 'A')
        {
          p += 26;
        }

        k = k - (p - 'A');
      }

      if (k < 'A')
      {
        k = k + 26;
      }

      SerialMonitor(k);
    }

  }

  for (byte j = 0; j < 26; j++)
  {
    if (EffSTECKER[j] == k)
    {
      k1 = 'A' + j;
    }
  }
  k = k1;

  SerialMonitor(k);

  return k;
}
