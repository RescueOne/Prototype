
// Class Variables
String    Name;
uint16_t  Value;
uint16_t* EEPROMAddress;
static uint16_t MenuItemCount;

// empty constructor
Menu::Menu(string name)
{
	MenuItemCount++;
	EEPROMAddress = (uint16_t*)(2 * MenuItemCount);
	Name 		  = name;
	Value         = eeprom_read_word(EEPROMAddress);
}

// Saves the menu item
void Menu::Save()
{
	eeprom_write_word(EEPROMAddress, Value);
}

// Opens the menu
void Menu::start()
{
	LCD.clear(); LCD.home();
	LCD.print("Entering menu");
	delay(500);
 
	while (true)
	{
		/* Show MenuItem value and knob value */
		int menuIndex = knob(6) * (MenuItem::MenuItemCount) / 1024;
		LCD.clear(); LCD.home();
		LCD.print(menuItems[menuIndex].Name); LCD.print(" "); LCD.print(menuItems[menuIndex].Value);
		LCD.setCursor(0, 1);
                int val = knob(7);
                
                // only for speed
//                if(menuIndex == 0)
//                {
//                  val = (val / 511.5 - 1.0) * 255;
//                }
                // Setting every value between -255 255
                val = (val / 1023.0) * 255;
		LCD.print("Set to "); LCD.print(val); LCD.print("?");
		delay(100);
 
		/* Press start button to save the new value */
		if (startbutton())
		{
			delay(100);
			if (startbutton())
			{
                                // Can max motor speed at +-700 so capping the speed at 700
                                if (menuIndex == 0)
                                {
                                        if ( val > 700) {
                                                LCD.clear(); LCD.home();
                                                LCD.print("Speed capped at 700.");
                                                val = 700;
                                                delay(250);
                                        }
                                }
                                
                                menuItems[menuIndex].Value = val;
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