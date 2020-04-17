#include <Keyboard.h>

// For OLED display
#include <U8g2lib.h>
// For I2C
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

const int i2cDisplayAddress = 0x3c;

#define BACKLIGHT 6
#define KEYS 8
#define MAXKEYSEQ 3

// All supported back light values
int backlitValues[] = { 0, 7, 63, 200 };
// Set default back light value
int backlit = 2;

int keys[] = { 4, 5, 7, 8, 9, A0, A1, A2 };

char keymap[KEYS][MAXKEYSEQ] = { 
                      { KEY_LEFT_CTRL, 'a', NULL }, 
                      { KEY_LEFT_CTRL, 'c', NULL }, 
                      { KEY_LEFT_CTRL, 'v', NULL }, 
                      { KEY_LEFT_CTRL, KEY_LEFT_GUI, 'd' },
                      { 'a', NULL, NULL },
                      { 'b', NULL, NULL },
                      { 'c', NULL, NULL },
                      { 'h', 'i', NULL }
};

void drawDisplay(const char *line1, const char *line2 = "", const char *line3 = "")
{
  // Write on OLED display
  // Clear the internal memory
  u8g2.clearBuffer();
  // Set appropriate font
  u8g2.setFont(u8g2_font_ncenR14_tr);
  u8g2.drawStr(0,14, line1);
  u8g2.drawStr(0,37, line2);
  u8g2.drawStr(0,62, line3);
  // Transfer internal memory to the display
  u8g2.sendBuffer();
}

bool isSensorAvailable(int sensorAddress)
{
  // Check if I2C sensor is present
  Wire.beginTransmission(sensorAddress);
  return 0 == Wire.endTransmission();
}

void updateBacklit(int key1, int key2)
{
  backlit++;
  if ( sizeof(backlitValues)/sizeof(int) == backlit )
  {
    backlit = 0;
    drawDisplay("Keyboard", "Backlit", "Off");
  }
  else
  {
    String backlitStatus = "On (";
    backlitStatus += backlit;
    backlitStatus += ")";
    drawDisplay("Keyboard", "Backlit", backlitStatus.c_str());
  }
  analogWrite(BACKLIGHT, backlitValues[backlit]);
  
  // Sit here while keys are still pressed, update backlight only once
  while( (LOW == digitalRead(key1)) || (LOW == digitalRead(key2)) ) { }
}

boolean processKey(int key, char characters[])
{
  delay(10);
  boolean pressed = false;
  for (int button = 0; button < KEYS; button++)
  {
    if (keys[button] == key)
    {
      continue;
    }
    if (LOW == digitalRead(keys[button]))
    {
      updateBacklit(key, keys[button]);
      return true;
    }
  }

  // Check again (debouncing). If still pressed, send key
  if (LOW == digitalRead(key))
  {
    // Backlight to full brightness during press
    analogWrite(BACKLIGHT,255);

    // Press keys
    for (int iter=0; iter < MAXKEYSEQ; iter++)
    {
      if (NULL == characters[iter])
      {
        continue;  
      }
      Keyboard.press(characters[iter]);
    }
    Keyboard.releaseAll();
    pressed = true;

    // Sit here while key is still pressed, sending charatcer only once
    while(LOW == digitalRead(key)) { }
  }
  return pressed;
}

void setup()
{
  Wire.begin();
  u8g2.begin();

  for (int button = 0; button < KEYS; button++)
  {
    pinMode(keys[button], INPUT_PULLUP);
  }

  pinMode(BACKLIGHT, OUTPUT);

  Keyboard.begin();

  drawDisplay("Mini", "Mechanical", "Keyboard");
}

void loop()
{
  boolean buttonPressed = false;
  for (int button = 0; button < KEYS; button++)
  {
    if ( (LOW == digitalRead(keys[button])) &&
         (true == processKey(keys[button], keymap[button])) )
    {
      buttonPressed = true;
      break;
    }
  }

  // If no keys were pressed, update the backlight
  if (false == buttonPressed)
  {
    analogWrite(BACKLIGHT, backlitValues[backlit]);
  }
}
