/*
	TinyPixelMapper
	a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.
	
	Git site https://github.com/Gerfunky/TinyPixelMapper
	
	License (GPL-3) : https://github.com/Gerfunky/TinyPixelMapper/blob/master/LICENSE
	
	TODO: more info here!

*/

//#include "Header.h"				// add the main Header file
#include "msgeq7_fft.h"
#include "config_TPM.h"			// add the Config.h where the main Settings a DEFINED
#include "tools.h"
#include "config_fs.h"
#include "wifi-ota.h"
#include "leds.h"


void setup()
{		
	if (DEF_BOOT_DEBUGING == true)
	{
		DEF_SERIAL_PORT.begin(DEF_SERIAL_SPEED);
		DEF_SERIAL_PORT.setDebugOutput(true);


		debugMe(debug_ResetReason(0));
		debugMe(debug_ResetReason(1));


		debugMe("Starting Setup - Light Fractal");
	}
	write_bool(DEBUG_OUT, DEF_BOOT_DEBUGING);   // Set the Boot debuging level will be overwriten when loading config from SPIFFS

	FS_setup_SPIFFS();  // includes loadbool()
	
	LEDS_setup();

	wifi_setup();

	//setup_comms(DEF_BOOT_DEBUGING, DEF_SERIAL_SPEED);   // Start CMDmessanger and the Serial if DEF_BOOT_DEBUGING == false
	debugMe("DONE Setup");


}

void loop() 
{

	wifi_loop();
	LEDS_loop();
	//debugMe("running.");
}
