#define WORT_SENSOR A1
#define COOL_SENSOR A2
#define BUZZER 10
#define FAN 2
#define BACKLIGHT 3
#define BUTTON_ADC_PIN A0
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          505  // left
#define SELECT_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  //

byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events

int WORT_IDEAL_TEMP = 21;
int WORT_MAX_TEMP = 27;
int REPLACE_TEMP = 10;
int ACKNOWLEDGED = 0;
boolean ALARM = false;

void getCurrentTemp( int *sign, int *whole, int *fract);
char wort_temp[10];
char cool_temp[10];

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  
  //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
  
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("WORT:");
  lcd.setCursor(0,1);
  lcd.print("ICE:");
  
  pinMode(BUZZER, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);
  digitalWrite(WORT_SENSOR, LOW);
  digitalWrite(COOL_SENSOR, LOW);
  digitalWrite(FAN, LOW);
  

}

void loop() {
  
  byte button;
  byte timestamp;
  
  button = ReadButtons();
  
  if( buttonJustPressed || buttonJustReleased ) {
    ACKNOWLEDGED = 1;  
  }
  
  getCurrentTemp(wort_temp, WORT_SENSOR);
  getCurrentTemp(cool_temp, COOL_SENSOR);
  lcd.setCursor(6,0);
  lcd.print(wort_temp);
  lcd.setCursor(6,1);
  lcd.print(cool_temp);
  if ( strtoul(wort_temp+2, NULL, 10) >= WORT_IDEAL_TEMP ) {
    digitalWrite(FAN, HIGH);
    if ( strtoul(wort_temp+2, NULL, 10) >= WORT_MAX_TEMP) {
      lcd.setCursor(14,0);
      lcd.print("AL");
      if ( ACKNOWLEDGED == 0 ) {
        analogWrite(BUZZER, 500);
       }
    }
  } else {
    digitalWrite(FAN, LOW);
    lcd.setCursor(14,0);
    lcd.print("  ");
  }
  
  if ( strtoul(cool_temp+2, NULL, 10) >= REPLACE_TEMP ) {
      lcd.setCursor(14, 2);
      lcd.print("AL");
      if ( ACKNOWLEDGED == 0 ) {
        analogWrite(BUZZER, 500);
      }
  } else {
    lcd.setCursor(14, 2);
    lcd.print("  ");
  }
  
  if ( strtoul(wort_temp+2, NULL, 10) >= WORT_MAX_TEMP || strtoul(cool_temp+2, NULL, 10) >= REPLACE_TEMP ) {
    if ( ACKNOWLEDGED == 1 ) {
      analogWrite(BUZZER, 0);
    }
  } else { 
    analogWrite(BUZZER, 0);
    ACKNOWLEDGED = 0;
  }
  delay(1000);
  buttonJustPressed = false;
  buttonJustReleased = false;
}

void OneWireReset (int Pin) // reset.  Should improve to act as a presence pulse
{
  digitalWrite (Pin, LOW);
  pinMode (Pin, OUTPUT);        // bring low for 500 us
  delayMicroseconds (500);
  pinMode (Pin, INPUT);
  delayMicroseconds (500);
}

void OneWireOutByte (int Pin, byte d) // output byte d (least sig bit first).
{
  byte n;

  for (n=8; n!=0; n--)
  {
    if ((d & 0x01) == 1)  // test least sig bit
    {
      digitalWrite (Pin, LOW);
      pinMode (Pin, OUTPUT);
      delayMicroseconds (5);
      pinMode (Pin, INPUT);
      delayMicroseconds (60);
    }
    else
    {
      digitalWrite (Pin, LOW);
      pinMode (Pin, OUTPUT);
      delayMicroseconds (60);
      pinMode (Pin, INPUT);
    }

    d = d>>1; // now the next bit is in the least sig bit position.
  }
}


byte OneWireInByte (int Pin) // read byte, least sig byte first
{
  byte d, n, b;

  for (n=0; n<8; n++)
  {
    digitalWrite (Pin, LOW);
    pinMode (Pin, OUTPUT);
    delayMicroseconds (5);
    pinMode (Pin, INPUT);
    delayMicroseconds (5);
    b = digitalRead (Pin);
    delayMicroseconds (50);
    d = (d >> 1) | (b<<7); // shift d to right and insert b in most sig bit position
  }
  return (d);
}


void getCurrentTemp (char *temp, int temp_pin)
{
  int HighByte, LowByte, TReading, Tc_100, sign, whole, fract;

  OneWireReset (temp_pin);
  OneWireOutByte (temp_pin, 0xcc);
  OneWireOutByte (temp_pin, 0x44); // perform temperature conversion, strong pullup for one sec

  OneWireReset (temp_pin);
  OneWireOutByte (temp_pin, 0xcc);
  OneWireOutByte (temp_pin, 0xbe);

  LowByte = OneWireInByte (temp_pin);
  HighByte = OneWireInByte (temp_pin);
  TReading = (HighByte << 8) + LowByte;
  sign = TReading & 0x8000;  // test most sig bit
  if (sign) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  whole = Tc_100 / 100;  // separate off the whole and fractional portions
  fract = Tc_100 % 100;

  if (sign) {
    temp[0] = '-';
  } else {
    temp[0] = '+';
  }

  if (whole/100 == 0) {
    temp[1] = ' ';
  } else {
    temp[1] = whole/100+'0';
  }

  temp[2] = (whole-(whole/100)*100)/10 +'0' ;
  temp[3] = whole-(whole/10)*10 +'0';
  temp[4] = '.';
  temp[5] = fract/10 +'0';
  temp[6] = fract-(fract/10)*10 +'0';
  temp[7] = '\0';
}

byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
   
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   //sense if the voltage falls within valid voltage windows
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
   
   //save the latest button value, for change event detection next time round
   buttonWas = button;
   
   return( button );
}
