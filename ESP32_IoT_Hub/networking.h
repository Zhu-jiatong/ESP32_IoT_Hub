// networking.h

#ifndef _networking_h
#define _networking_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ESPAsyncWebSrv.h>
#include <ESPmDNS.h>
#include "file_utils.h"
#include "esp32_session_manager.h"
#include "hardware.h"

namespace cst
{
	extern AsyncWebServer server;
	extern const String credentials_dir;
	extern const String system_dir;

	void begin_network();
	void begin_server();
} // namespace cst

#endif

