#include "datastore.h"
#include "ntp_client.h"


/**************************************************************************************************
 *    Function      : begin
 *    Class         : NTP_Client
 *    Description   : Starts the NTP client
 *    Input         : Timecore* tc  
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NTP_Client::begin( Timecore* tc ){

timeptr=tc;
ReadSettings();

NTP.onNTPSyncEvent ([&](NTPSyncEvent_t event) {
       SyncEvent(event);
});

NTP.begin ((char*)Config.ntpServerName, DEFAULT_NTP_TIMEZONE , false, 0);
NTP.setNTPTimeout(2500);
NTP.setInterval (50);

}

/**************************************************************************************************
 *    Function      : SyncEvent
 *    Class         : NTP_Client
 *    Description   : Starts the NTP client
 *    Input         : NTPSyncEvent_t event  
 *    Output        : none
 *    Remarks       : Event after a NTP Sync
 **************************************************************************************************/
void NTP_Client::SyncEvent(NTPSyncEvent_t event){
          switch(event){
          case noResponse:{
               Serial.println(F("NTP No Response"));
          }

          break;

          case timeSyncd:{
          uint32_t ts = NTP.getLastNTPSync ();
          /* we need to grab the offset if ther is any and remove it */
          Serial.print(F("Sync:"));
          Serial.println (ts);
          if(ts>100){
            if(timeptr!=NULL){
              timeptr->SetUTC(ts,NTP_CLOCK);
            }           
          }
        } break;

        case invalidAddress :{
          Serial.println(F("NTP invalid Address"));
        } break;

        }
 
     
}

/**************************************************************************************************
 *    Function      : SetServerName
 *    Class         : NTP_Client
 *    Description   : Sets the NTP Servername
 *    Input         : char* ServerName 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NTP_Client::SetServerName( String ServerName ){
  strncpy((char*)Config.ntpServerName,ServerName.c_str(),sizeof(Config.ntpServerName) );
  NTP.setNtpServerName ( ServerName );
}

/**************************************************************************************************
 *    Function      : Sync
 *    Class         : NTP_Client
 *    Description   : Starts a NTP Sync
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NTP_Client::Sync(){

  /* We send a request and will get the result may in the event */
  if(true == Config.NTPEnable ){
    Serial.println(F("Send NTP Request"));
    NTP.getTime();
  } else {
    Serial.println(F("NTP not active"));  
  }
  
}

/**************************************************************************************************
 *    Function      : Tick
 *    Class         : NTP_Client
 *    Description   : Needs to be called every 1 second to do the sync
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NTP_Client::Tick(){
  if(next_update>0){
    next_update--;
    //Serial.printf("Update in %i Ticks\n\r", next_update);
  } else {
   
    _sync=true;
    next_update = Config.SyncIntervall;
  }
  
}

/**************************************************************************************************
 *    Function      : GetServerName
 *    Class         : NTP_Client
 *    Description   : Reads the current NTP Servername
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
char* NTP_Client::GetServerName(){
  return (char*)Config.ntpServerName;

}

/**************************************************************************************************
 *    Function      : SetNTPSyncEna
 *    Class         : NTP_Client
 *    Description   : Sets the NTP_Sync Enable
 *    Input         : bool Ena
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void NTP_Client::SetNTPSyncEna( bool Ena){
 
  if(true==Ena){
    Config.NTPEnable=true;
    next_update=15; /* Force update after 30 seconds*/
  } else {
    Config.NTPEnable=false;
  }
 
 }

 /**************************************************************************************************
 *    Function      : GetNTPSyncEna
 *    Class         : NTP_Client
 *    Description   : Reads the NTP_Sync Enable
 *    Input         : none
 *    Output        : bool
 *    Remarks       : none
 **************************************************************************************************/
 bool NTP_Client::GetNTPSyncEna( void ){
  if(true == Config.NTPEnable){
    return true;
  } else {
     return false;
  } 
 }

 /**************************************************************************************************
 *    Function      : GetSyncInterval
 *    Class         : NTP_Client
 *    Description   : Reads the NTP_Sync Intervall
 *    Input         : none
 *    Output        : int32 ( in seconds )
 *    Remarks       : none
 **************************************************************************************************/
 int32_t NTP_Client::GetSyncInterval( void ){
  return (  Config.SyncIntervall / 60) ;
 }


 /**************************************************************************************************
 *    Function      : SetSyncInterval
 *    Class         : NTP_Client
 *    Description   : Sets the NTP_Sync Intervall
 *    Input         : int32 ( in seconds )
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
 void NTP_Client::SetSyncInterval( int32_t Sync){
  Config.SyncIntervall=( Sync * 60 );
 }

 /**************************************************************************************************
 *    Function      : ReadTime
 *    Class         : NTP_Client
 *    Description   : Sets the NTP_Sync Intervall
 *    Input         : bool* 
 *    Output        : none
 *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
 **************************************************************************************************/
 uint32_t NTP_Client::ReadTime( bool* delayed_result){
  *delayed_result=true;
  return 0;
 }

 /**************************************************************************************************
 *    Function      : SaveSettings
 *    Class         : NTP_Client
 *    Description   : Saves the Settings to EEPROM / Flash
 *    Input         : none 
 *    Output        : none
 *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
 **************************************************************************************************/
void NTP_Client::SaveSettings(){
  ntp_config_t cnf;
  memcpy((void*)(&cnf),(void*)(&Config),sizeof(ntp_config_t)); 
  write_ntp_config(cnf);

}

 /**************************************************************************************************
 *    Function      : ReadSettings
 *    Class         : NTP_Client
 *    Description   : Reads the Settings to EEPROM / Flash
 *    Input         : none 
 *    Output        : none
 *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
 **************************************************************************************************/
void NTP_Client::ReadSettings(){

 ntp_config_t cnf = read_ntp_config();
 memcpy((void*)(&Config),(void*)(&cnf),sizeof(ntp_config_t));  
  

}

 /**************************************************************************************************
 *    Function      : Task
 *    Class         : NTP_Client
 *    Description   : This will trigger a sync
 *    Input         : none 
 *    Output        : none
 *    Remarks       : to avoid problems within isr's
 **************************************************************************************************/
void NTP_Client::Task(){
    if(_sync!=false){
        //Serial.println("Do NTP Sync in Task");
         Sync();
         _sync=false;
    }
}




