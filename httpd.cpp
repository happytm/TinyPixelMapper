/*
		Here we Have the Web Server (httpd) and the update client over http 

		the html files are located in the data subfolder and need to be sent in with 
		ESP8266/ESP32 Sketch Data Upload.  
		update server over HTTP test


*/

	#include "config_TPM.h"
	#ifdef ESP32
		#include <WiFi.h>	
		#include <HTTPClient.h>
		#include <ESPmDNS.h>
		#include<SPIFFS.h>

		//#ifndef PLATFORMIO
			#include <WebServer.h>
			WebServer  httpd(80);					// The Web Server 
		//#else 
		//	#include <FS.h>	
		//	#include <ESP32WebServer.h>		
		//	ESP32WebServer httpd(80);

		//#endif
		#include <Update.h>

		//#include <ESP32httpUpdate.h>
		//#include <WiFiClient.h>
	#endif



// ********* Externals
	#include "tools.h"						// for bools reading/writing
	#include "config_fs.h"					
	#include "httpd.h"

// *********** External Variables 
	#include "wifi-ota.h"					// get the wifi structures
	extern wifi_Struct wifi_cfg;			// link to wifi variable wifi_cfg


// static serv from progmem. https://github.com/Gheotic/ESP-HTML-Compressor
	
	


String httpd_getContentType(String filename) {
  if (httpd.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}


/*
bool exists(String path){
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
} */


bool httpd_handleFileRead(String path) {

	 
      
	if (path.endsWith("/")) path += "index.html";
	debugMe("handleFileRead: " + path);

	String contentType = httpd_getContentType(path);	
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
		debugMe("prestreeam");
		size_t sent = httpd.streamFile(file, contentType);
		debugMe("post stream");
		//String(file.size());
		file.close();
		debugMe("  " + path + " closed and sent :" + String(sent) );
		return true;
	}
	return false;
}
File fsUploadFile;
void httpd_handleFileUpload() {
  if (httpd.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = httpd.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    debugMe("handleFileUpload Name: "); debugMe(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    debugMe("handleFileUpload Size: "); debugMe(String(upload.totalSize));
  }
}



void httpd_handleFileDelete() {
	if (httpd.args() == 0) return httpd.send(500, "text/plain", "BAD ARGS");
	String path = httpd.arg(0);

             
	 debugMe("handleFileDelete: " + path);

	if (path == "/")
		return httpd.send(500, "text/plain", "BAD PATH");
	if (!SPIFFS.exists(path))
		return httpd.send(404, "text/plain", "FileNotFound");
	SPIFFS.remove(path);
	httpd.send(200, "text/plain", "");
	path = String();
}

void httpd_handleFileCreate() {
	if (httpd.args() == 0)
		return httpd.send(500, "text/plain", "BAD ARGS");
	String path = httpd.arg(0);

	 debugMe("handleFileCreate: " + path);

	if (path == "/")
		return httpd.send(500, "text/plain", "BAD PATH");
	if (SPIFFS.exists(path))
		return httpd.send(500, "text/plain", "FILE EXISTS");
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return httpd.send(500, "text/plain", "CREATE FAILED");
	httpd.send(200, "text/plain", "");
	path = String();
}



void httpd_handlecConfFileList() {
	String path = "/";

	if (httpd.hasArg("dir")) 
		path = httpd.arg("dir");
	
 
	 debugMe("handleCONFFileList: " + path);

	File dir = SPIFFS.open(path);

	path = String();

	String output = "[";

	File fileX = dir.openNextFile();

	

	
	while (fileX) {
		if( String(fileX.name()).startsWith("/conf/"))
		{
			//File entry = dir.open("r");
			if (output != "[") output += ',';
			//bool isDir = fileX.isDirectory();
			bool isDir = false;
			output += "{\"type\":\"";
			output += (isDir) ? "dir" : "file";
			output += "\",\"name\":\"";
			output += String(fileX.name()).substring(1);
			output += "\"}";
			debugMe(String(fileX.name()));
		}
			fileX.close();
			fileX = dir.openNextFile();
		
		//dir.close();
	}

	dir.close();


	output += "]";
	httpd.send(200, "text/json", output);
	//debugMe(output);
}








void httpd_handleFileList() {
	String path = "/";

	if (httpd.hasArg("dir")) 
		path = httpd.arg("dir");
	
 
	 debugMe("handleFileList: " + path);

	File dir = SPIFFS.open(path);

	//File file = dir.openNextFile();

	path = String();

	String output = "[";

	File fileX = dir.openNextFile();

	debugMe(String(fileX.name()));

	while (fileX) {
		//File entry = dir.open("r");
		if (output != "[") output += ',';
		//bool isDir = fileX.isDirectory();
		bool isDir = false;
		output += "{\"type\":\"";
		output += (isDir) ? "dir" : "file";
		output += "\",\"name\":\"";
		output += String(fileX.name()).substring(1);
		output += "\"}";
		//debugMe(String(fileX.name()));
		fileX.close();
		fileX = dir.openNextFile();
		debugMe(String(fileX.name()));
		//dir.close();
	}

	dir.close();


	output += "]";
	httpd.send(200, "text/json", output);
	//debugMe(output);
}

void httpd_handle_default_args()
{

	if (httpd.args() > 0)
	{
    
		 debugMe("set args present");

		// set the brightness


		if (httpd.hasArg("ssid") && httpd.hasArg("password") && httpd.hasArg("wifiMode")) {
			String ssid_STR = httpd.arg("ssid");
			String PWD_STR = httpd.arg("password");
			String wifiMmode_STR = httpd.arg("wifiMode");

			ssid_STR.toCharArray(wifi_cfg.ssid, sizeof(wifi_cfg.ssid));
			PWD_STR.toCharArray(wifi_cfg.pwd, sizeof(wifi_cfg.pwd));
			write_bool(WIFI_MODE, 1);
			//wifiMode =  wifiMmode_STR[0]; 
			FS_wifi_write(0);



			debugMe("Setting ssid to ", false);
			 debugMe(wifi_cfg.ssid);

			debugMe("Setting password to ", false);
			debugMe(wifi_cfg.pwd);

			debugMe("Setting wifi_mode to ", false);
			debugMe(get_bool(WIFI_MODE));
			

		}

		if (httpd.hasArg("APname") && httpd.hasArg("wifiMode")) {
			String APname_STR = httpd.arg("APname");
			String wifiMmode_STR = httpd.arg("wifiMode");

			APname_STR.toCharArray(wifi_cfg.APname, 32);
			write_bool(WIFI_MODE, 0 );
			FS_wifi_write(0);

			debugMe("Setting APname to ", false);
			debugMe(wifi_cfg.APname);

			debugMe("Setting wifi_mode to ", false);
			debugMe(get_bool(WIFI_MODE));
			

		}

		if (httpd.hasArg("delete")) {
			String path = httpd.arg("delete");

			// path.toCharArray(ePassword,64);    
			SPIFFS.remove(path);

			debugMe("requested delte : ", false);
			debugMe(path);
			

		}
	}

}


void httpd_handleRequestSettings() 
{
	//String  output_bufferZ = "-" ;

	// Setup Handlers
	
	/*handling uploading firmware file */
	
	httpd.on("/update", HTTP_GET, []() { 
		httpd.sendHeader("Connection", "close");
      	httpd.send(200, "text/html",  "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
    });


	httpd.on("/update", HTTP_POST, []() {
		httpd.sendHeader("Connection", "close");
		httpd.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
		//esp_wifi_wps_disable(); 
		ESP.restart();
	}, []() {
		HTTPUpload& upload = httpd.upload();
		if (upload.status == UPLOAD_FILE_START) {
			Serial.printf("Update: %s\n", upload.filename.c_str());
			if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {//start with max available size
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_WRITE) {
			// flashing firmware to ESP //
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
				Update.printError(Serial);
			}
		}
		else if (upload.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
				Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			}
			else {
				Update.printError(Serial);
			}
		}
	});  //*/




	//get heap status, analog input value and all GPIO statuses in one json call

	

	httpd.on("/index.html", []() {
#ifdef ARTNET_ENABLED
		if (get_bool(ARTNET_ENABLE) == true)
			handleFileRead("/artnet.html");
		else
#endif
		httpd_handleFileRead("/index.html");
		httpd_handle_default_args();
	});
	




	httpd.on("/settings.html", []() {   httpd_handleFileRead("/settings.html");	      httpd_handle_default_args();   });
	httpd.on("/list", HTTP_GET, httpd_handlecConfFileList); 
	httpd.on("/listall", HTTP_GET, httpd_handleFileList);
	//load editor
	httpd.on("/edit", HTTP_GET, []() { if (!httpd_handleFileRead("/edit.htm")) httpd.send(404, "text/plain", "edit_FileNotFound"); });
	httpd.on("/edit", HTTP_DELETE, httpd_handleFileDelete);
	httpd.on("/edit", HTTP_POST, []() { httpd.send(200, "text/plain", ""); }, httpd_handleFileUpload);

	httpd.onNotFound([]() {if (!httpd_handleFileRead(httpd.uri()))  httpd.send(404, "text/plain", "FileNotFound im sorry check in the next 2'n dimension on the left"); });

	httpd.on("/wifiMode", []() { httpd.send(200, "text/plain", String(get_bool(WIFI_MODE)));   });
	httpd.on("/ssid", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.ssid);  });
	httpd.on("/password", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.pwd);   });
	httpd.on("/APname", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.APname);   });
	httpd.on("/reset", HTTP_GET, []() { httpd.send(200, "text/plain", "Rebooting"); ESP.restart();   });
	


	//httpd.on("/js/ace.js", HTTP_GET, []() { httpd.send(200, "text/javascript", data_ace_js);  });




}










void httpd_toggle_webserver()
{
	if (get_bool(HTTP_ENABLED) == true)
	{
		httpd.stop();
		write_bool(HTTP_ENABLED, false);
		 debugMe("httpd turned off");
	}
	else
	{
		httpd.begin();
		write_bool(HTTP_ENABLED, true);
		debugMe("httpd turned on");
#ifdef ESP8266

		httpUpdater.setup(&httpd);
#endif		
		//ESPhttpUpdate.setup(&httpd);
		// debugMe("HTTP server started");
		MDNS.begin(wifi_cfg.APname);
		MDNS.addService("http", "tcp", 80);
	}


}


void httpd_setup()
{
	debugMe("HTTPd_setup");
	

	httpd_handleRequestSettings();

	if (get_bool(HTTP_ENABLED) == true)
	{
		httpd.begin();					// Switch on the HTTP Server
#ifdef ESP8266
		httpUpdater.setup(&httpd);
		 debugMe("HTTP server started");
#endif
		 //ESPhttpUpdate.setup(&httpd);
		 MDNS.begin(wifi_cfg.APname);
		MDNS.addService("http", "tcp", 80);
		debugMe("Starting HTTP");
	}
}

void http_loop()
{
	if (get_bool(HTTP_ENABLED) == true)
		httpd.handleClient();
}




