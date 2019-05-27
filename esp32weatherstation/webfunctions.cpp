#include <ArduinoJson.h>
#include <WebServer.h>
#include "datastore.h"
#include "webfunctions.h"

extern void sendData(String data);
extern WebServer * server;
extern TaskHandle_t MQTTTaskHandle;


/***********************************************************************/
// non public functions only used internally 
//
//
//
/***********************************************************************/

/**************************************************************************************************
*    Function      : update_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_notes();

/**************************************************************************************************
*    Function      : read_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void read_notes();

/**************************************************************************************************
*    Function      : updates the mqtt settings
*    Description   : Parses POST for new MQTT settings 
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/
void mqttsettings_update( void );


/**************************************************************************************************
*    Function      : Reads back the MQTT settings
*    Description   : Sends a json string 
*    Input         : none
*    Output        : none
*    Remarks       : only contains if a password is set no data itself
**************************************************************************************************/
void read_mqttsetting( void );

// END OF PROTOTYPES //



void register_functions(WebServer* webserver){

  server->on("/notes.dat",HTTP_GET, read_notes);
  server->on("/notes.dat",HTTP_POST, update_notes);
  server->on("/mqtt/settings",HTTP_POST, mqttsettings_update);
  server->on("/mqtt/settings",HTTP_GET, read_mqttsetting);


}


/**************************************************************************************************
*    Function      : update_notes
*    Description   : Parses POST for new notes
*    Input         : none
*    Output        : none
*    Remarks       : none
**************************************************************************************************/ 
void update_notes(){
  char data[501]={0,};
  if( ! server->hasArg("notes") || server->arg("notes") == NULL ) { // If the POST request doesn't have username and password data
    /* we are missing something here */
  } else {
   
    //Serial.printf("New Notes: %s\n\r",server->arg("notes").c_str());
    /* direct commit */
    uint32_t str_size = server->arg("notes").length();
    if(str_size<501){
      strncpy((char*)data,server->arg("notes").c_str(),501);
      eepwrite_notes((uint8_t*)data,501);
      //saveNotes( data,501 );
    } else {
      Serial.println("Note > 500 char");
    }
  }

  server->send(200);    
}

/**************************************************************************************************
*    Function      : read_notes
*    Description   : none
*    Input         : none
*    Output        : none
*    Remarks       : Retunrs the notes as plain text
**************************************************************************************************/ 
void read_notes(){
  char data[501]={0,};
  //loadNotes(data,500);
  eepread_notes((uint8_t*)data,501);
  sendData(data);    
}



void mqttsettings_update( ){
 
  mqttsettings_t Data;
  //loadMQTTSettings(&Data); 
  Data = eepread_mqttsettings();
  
  if( ! server->hasArg("MQTT_USER") || server->arg("MQTT_USER") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_USER");
    bzero(Data.mqttusername,sizeof(Data.mqttusername));
    strncpy(Data.mqttusername, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_PASS") || server->arg("MQTT_PASS") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_PASS");
    bzero(Data.mqttpassword,sizeof(Data.mqttpassword));
    strncpy(Data.mqttpassword, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_SERVER") || server->arg("MQTT_SERVER") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_SERVER");
    bzero(Data.mqttservername,sizeof(Data.mqttservername));
    strncpy(Data.mqttservername, value.c_str(),128);
  }

  if( ! server->hasArg("MQTT_HOST") || server->arg("MQTT_HOST") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_HOST");
    bzero(Data.mqtthostname,sizeof(Data.mqtthostname));
    strncpy(Data.mqtthostname, value.c_str(),64);
  }

  if( ! server->hasArg("MQTT_PORT") || server->arg("MQTT_PORT") == NULL ) { 
    /* we are missong something here */
  } else { 
    int32_t value = server->arg("MQTT_PORT").toInt();
    if( (value>=0) && ( value<=UINT16_MAX ) ){
      Data.mqttserverport = value;
    }
  }
  
  if( ! server->hasArg("MQTT_TOPIC") || server->arg("MQTT_TOPIC") == NULL ) { 
    /* we are missong something here */
  } else { 
    String value = server->arg("MQTT_TOPIC");
    bzero(Data.mqtttopic,sizeof(Data.mqtttopic));
    strncpy(Data.mqtttopic, value.c_str(),500);
  }

  if( ! server->hasArg("MQTT_ENA") || server->arg("MQTT_ENA") == NULL ) { 
    /* we are missing something here */
  } else { 
    bool value = false;
    if(server->arg("MQTT_ENA")=="true"){
      value = true;
    }
    Data.enable = value;
  }
  

  if( ! server->hasArg("MQTT_TXINTERVALL") || server->arg("MQTT_TXINTERVALL") == NULL ) { 
    /* we are missong something here */
  } else { 
    uint32_t value = server->arg("MQTT_TXINTERVALL").toInt();
    Data.mqtttxintervall = value;
  }
  /* write data to the eeprom */
  //saveMQTTSettings(&Data);
  eepwrite_mqttsettings(Data);   
  if(MQTTTaskHandle != NULL ){
    xTaskNotify( MQTTTaskHandle, 0x01, eSetBits );
  }
  server->send(200); 

}

void read_mqttsetting(){
  String response =""; 
  mqttsettings_t Data;
  Data = eepread_mqttsettings();
  //loadMQTTSettings(&Data);  
  DynamicJsonDocument  root(2000); 
  
 
  root["mqttena"]= (bool)(Data.enable);
  root["mqttserver"] = String(Data.mqttservername);
  root["mqtthost"] = String(Data.mqtthostname);
  root["mqttport"] = Data.mqttserverport;
  root["mqttuser"] = String(Data.mqttusername);
  root["mqtttopic"] = String(Data.mqtttopic);
  root["mqtttxintervall"] = Data.mqtttxintervall;
  if(Data.mqttpassword[0]!=0){
    root["mqttpass"] = "********";
  } else {
    root["mqttpass"] ="";
  }

  serializeJson(root,response);
  sendData(response);

}
