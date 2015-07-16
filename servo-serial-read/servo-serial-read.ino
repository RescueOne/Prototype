#include <phys253.h>          
#include <LiquidCrystal.h>    

void setup()
{  
  #include <phys253setup.txt>
  Serial.begin(9600) ;  
}

void loop()
{
  
  // Set the screen to be ready to print
  LCD.clear();  LCD.home();
  LCD.setCursor(0,0); LCD.print("Servo Angle: ");
  LCD.setCursor(0,1);
  
  if (startbutton())
  {
    int knob_pin = 7;

    int angle = ( analogRead(knob_pin) / 1023.0) * 180;
    RCServo0.write(angle);
//    if (Serial.available())
//    {
//      Serial.println(angle); 
//    }
    LCD.print(analogRead(0));
    LCD.print(analogRead(1));
    delay(100);
  }
  int DELAY = 50;
  int angle = 0;
  if (stopbutton())
  {
    while( angle < 175 )
    {
      RCServo0.write(angle);
      delay(DELAY);
      angle++;
      LCD.print(angle);
    }
    
  }

}

