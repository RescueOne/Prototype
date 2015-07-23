#include <avr/EEPROM.h>
#include <phys253.h>          
#include <LiquidCrystal.h>


/*
MENU Reference Sheet

Start & Stop = open Menu
  -> options will be ... PID Tuning, QRD Debug Code, IR, etc...
  -> press start to run, stop for back
    
*/
class MenuItem
{
public:
	String    Name;
	uint16_t  Value;
	uint16_t* EEPROMAddress;
	static uint16_t MenuItemCount;
	MenuItem(String name)
	{
		MenuItemCount++;
		EEPROMAddress = (uint16_t*)(2 * MenuItemCount);
		Name 		  = name;
		Value         = eeprom_read_word(EEPROMAddress);
	}
	void Save()
	{
		eeprom_write_word(EEPROMAddress, Value);
	}
};

int PID_MENU = 0;
int ARM_MENU = 1;
int QRD_TEST = 2;
int ARM_TEST = 3;

uint16_t MenuItem::MenuItemCount = 0;
/* Add the menu items here */
MenuItem PIDTuning        = MenuItem("PID Tuning");
MenuItem ArmTuning        = MenuItem("Arm Tuning");
MenuItem QRDTest          = MenuItem("QRD Test");
MenuItem ArmTest          = MenuItem("Arm Test");
// For PID Tuning
MenuItem Speed            = MenuItem("Speed");
MenuItem ProportionalGain = MenuItem("P-gain");
MenuItem DerivativeGain   = MenuItem("D-gain");
MenuItem IntegralGain     = MenuItem("I-gain");
MenuItem Threshold        = MenuItem("Threshold");
MenuItem menuItems[]      = {PIDTuning, ArmTuning, QRDTest, ArmTest, Speed, ProportionalGain, DerivativeGain, IntegralGain, Threshold};
 
void setup()
{
  #include <phys253setup.txt>
  LCD.clear();
  LCD.home();
}

// Pins
int MOTOR_LEFT = 2; //PWM output for left motor
int MOTOR_RIGHT = 3; //PWM output for right motor
int QRD_LEFT_PIN = 1; //Analog pin for left QRD
int QRD_RIGHT_PIN = 0; //Analog pin for right QRD
int QRD_PET = 2;

// Constants
int MAX_INTEGRAL = 50; //Maximum integral term value
int MAX_ANALOG = 1023;
int THRESHOLD = menuItems[8].Value; //threshold for switch from white to black
  
//Variables
  int P = menuItems[5].Value; //Proportional gain value
  int D = menuItems[6].Value; //Derivative gain value
  int I = menuItems[7].Value; //Integral gain value
  int S = menuItems[4].Value; //Speed
  int qrd_left = 0; //Value of left qrd
  int qrd_right = 0; //Value of right qrd
  int qrd_pet = 0;
  int error = 0; //Current error
  int last_error = 0; //Previous error
  int recent_error = 0; //Recent error
  int total_error = 0; //Integral of the error
  int proportional = 0; //Proportional control
  int derivative = 0; //Derivative control
  int integral = 0; //Integral control
  int duration_recent = 0; //Number of loops on recent error
  int duration_last = 0; //Number of loops on last error
  int compensation = 0; //

/*
===============
== MAIN LOOP ==
===============
*/
void loop() {
  
  motor.speed(MOTOR_LEFT,0);
  motor.speed(MOTOR_RIGHT,0);
  
  // TODO set STOP to exit/back
  // and start and stop = menu
  LCD.clear(); LCD.home();
  LCD.print("Start+Stop: Menu");
  LCD.setCursor(0, 1);
  LCD.print("Start: PID");
  delay(100);
 
  if (startbutton() && stopbutton())
  {
    delay(100);
    if (startbutton() && stopbutton())
    {
      MainMenu();
    }
  }

  if (startbutton())
  {
    delay(100);
    if (startbutton())
    {
      PID();  
    } 
  }
}


/*
=========
== PID ==
=========
*/

void PID()
{  
  LCD.clear(); LCD.home();
  LCD.print("Following");
  
  int spd = (int)((float)S*((float)255/(float)1023));
  
  //PID loop
  while (true)
  {
    //Read QRD's
    qrd_left = analogRead(QRD_LEFT_PIN);
    qrd_right = analogRead(QRD_RIGHT_PIN);
    
    qrd_pet = analogRead(QRD_PET);
    
    // If the robot is over the pet tape
    if (qrd_pet > THRESHOLD) delay(3000);
    
    /*Determine error
    * <0 its to the left
    * >0 its to the right
    * 0 its dead on
    */
    
    //left on white
    if(qrd_left < THRESHOLD){
      //right on white
      if(qrd_right < THRESHOLD){
        if(last_error < 0) {error = -5;LCD.setCursor(0,1);LCD.print("L2");}
        else {error = 5;LCD.setCursor(0,1);LCD.print("R2");}
      }
      //right on black
      else{error = -1;LCD.setCursor(0,1);LCD.print("L1");}
    }
    //left on black
    else{
      //right on white
      if(qrd_right < THRESHOLD){error = 1;LCD.setCursor(0,1);LCD.print("R1");}
      //right on black
      else{error = 0;LCD.setCursor(0,1);LCD.print("CE");}
    }
    
    //determine control factors
    
    //Proportional control
    proportional = P*error;
    
    //Integral error
    total_error += error;
    integral = I*total_error;
    if(integral > MAX_INTEGRAL) {integral = MAX_INTEGRAL;}
    if(integral < -MAX_INTEGRAL) {integral = -MAX_INTEGRAL;}
    
    //Derivative
    if(error != last_error){
      recent_error = last_error;
      duration_recent = duration_last;
      last_error = error;
      duration_last = 1;
    }
    else {
      duration_last++;
    }
    derivative = (int)(((float)D*(float)(error - recent_error))/((float)(duration_recent + duration_last)));
    
    //Compensation
    compensation = proportional + integral + derivative;
    
    //Plant control (compensation +ve means move right)
    motor.speed(MOTOR_LEFT,spd + compensation);
    motor.speed(MOTOR_RIGHT,spd - compensation);
    
    /* Press stop button to exit */
	if (stopbutton())
	{
	        delay(100);
	        if (stopbutton())
		{
			LCD.clear(); LCD.home();
			LCD.print("Leaving PID");
			delay(500);
			return;
		}
	}
  }
}

/*
===============
== DEBUGGING ==
===============
*/

void TestQRD() {
  
  while(true)
  {
      if(stopbutton()){
          delay(200);
          if(stopbutton()){
              break; 
          }
      }
      
      LCD.clear(); LCD.home();
      
      qrd_left = analogRead(QRD_LEFT_PIN);
      qrd_right = analogRead(QRD_RIGHT_PIN);
      qrd_pet = analogRead(QRD_PET);
      
      LCD.setCursor(0,0);
      if (qrd_pet > THRESHOLD) { LCD.print("STOP"); }
      else { LCD.print("TO FAR"); }
       
      //left on white
        if(qrd_left < THRESHOLD){
          //right on white
          if(qrd_right < THRESHOLD){
            if(last_error < 0) {error = -5;LCD.setCursor(0,1);LCD.print("L2");}
            else {error = 5;LCD.setCursor(0,1);LCD.print("R2");}
          }
          //right on black
          else{error = -1;LCD.setCursor(0,1);LCD.print("L1");}
        }
        //left on black
        else{
          //right on white
          if(qrd_right < THRESHOLD){error = 1;LCD.setCursor(0,1);LCD.print("R1");}
          //right on black
          else{error = 0;LCD.setCursor(0,1);LCD.print("CE");}
        }
      }
}


/*
====================
== MENU FUNCTIONS ==
====================
*/

// Main Menu for selecting another Menu
void MainMenu()
{
	LCD.clear(); LCD.home();
	LCD.print("Please wait ...");
	delay(500);
 
	while (true)
	{
		/* Show MenuItem value and knob value */
		int menuIndex = knob(6) * (MenuItem::MenuItemCount) / 1024;
                if (menuIndex > 3) menuIndex = 3; // prevent access of nested menu items
                
		LCD.clear(); LCD.home();
		LCD.print(menuItems[menuIndex].Name); LCD.print(" "); //LCD.print(menuItems[menuIndex].Value);
		LCD.setCursor(0, 1); LCD.print("Stop = Exit");
		delay(100);
 
		/* Press start button to save the new value */
		if (startbutton())
		{
			delay(100);
			if (startbutton())
			{
                                if (menuIndex == PID_MENU) TapePIDTuningMenu();
                                if (menuIndex == ARM_MENU) LCD.print("NOT CONFIG");
                                if (menuIndex == QRD_TEST) TestQRD();
                                if (menuIndex == ARM_TEST) LCD.print("NOT CONFIG");
			}
		}
 
		/* Press stop button to exit menu */
		if (stopbutton())
		{
			delay(100);
			if (stopbutton())
			{
				LCD.clear(); LCD.home();
				LCD.print("Leaving menu");
				delay(500);
				return;
			}
		}
	}
}

// This menu is used for tuning the PID control for Tape Following
void TapePIDTuningMenu() 
{
	LCD.clear(); LCD.home();
	LCD.print("Entering menu");
	delay(500);
 
	while (true)
	{
		/* Show MenuItem value and knob value */
		int menuIndex = knob(6) * (MenuItem::MenuItemCount) / 1024;
                if (menuIndex < 4) menuIndex = 4; // prevent access of nested menu items
                
		LCD.clear(); LCD.home();
		LCD.print(menuItems[menuIndex].Name); LCD.print(" "); LCD.print(menuItems[menuIndex].Value);
		LCD.setCursor(0, 1); LCD.print(knob(7)); LCD.print("?");
		delay(100);
 
		/* Press start button to save the new value */
		if (startbutton())
		{
			delay(100);
			if (startbutton())
			{
				menuItems[menuIndex].Value = knob(7);
				menuItems[menuIndex].Save();
				delay(250);
			}
		}
 
		/* Press stop button to exit menu */
		if (stopbutton())
		{
			delay(100);
			if (stopbutton())
			{
				LCD.clear(); LCD.home();
				LCD.print("Leaving menu");
				delay(500);
				return;
			}
		}
	}
}
