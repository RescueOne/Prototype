
/*
*  Creates a "map" with the location of an EEPROM address
*  EEPROM = TINAH storage
*/
#indef Menu_h
#define Menu_h

#include "Arduino.h"
#include "avr/EEPROM.h"
#include "phys253.h"          
#include "LiquidCrystal.h"

class Menu {
	public: 
		Menu(string name);
		void Save();
		void start();
};

#endif