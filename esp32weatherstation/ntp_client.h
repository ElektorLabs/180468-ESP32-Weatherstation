#ifndef NTP_CLIENT_H
 #define NTP_CLIENT_H

#include "timecore.h"
#include "datastore.h"
#include <NtpClientLib.h>

class NTP_Client {
    public:
        /**************************************************************************************************
         *    Function      : begin
         *    Class         : NTP_Client
         *    Description   : Starts the NTP client
         *    Input         : Timecore* tc  
         *    Output        : none
         *    Remarks       : none
         **************************************************************************************************/      
        void begin(  Timecore* tc );

        /**************************************************************************************************
         *    Function      : SetServerName
         *    Class         : NTP_Client
         *    Description   : Sets the NTP Servername
         *    Input         : char* ServerName 
         *    Output        : none
         *    Remarks       : none
         **************************************************************************************************/
        void SetServerName( String ntpServerName );
        
       /**************************************************************************************************
       *    Function      : GetServerName
       *    Class         : NTP_Client
       *    Description   : Reads the current NTP Servername
       *    Input         : none
       *    Output        : none
       *    Remarks       : none
       **************************************************************************************************/
        char* GetServerName();
        
        /**************************************************************************************************
         *    Function      : SetNTPSyncEna
         *    Class         : NTP_Client
         *    Description   : Sets the NTP_Sync Enable
         *    Input         : bool Ena
         *    Output        : none
         *    Remarks       : none
         **************************************************************************************************/
        void SetNTPSyncEna( bool );
        
        /**************************************************************************************************
         *    Function      : GetNTPSyncEna
         *    Class         : NTP_Client
         *    Description   : Reads the NTP_Sync Enable
         *    Input         : none
         *    Output        : bool
         *    Remarks       : none
         **************************************************************************************************/
        bool GetNTPSyncEna( void );

         /**************************************************************************************************
         *    Function      : GetSyncInterval
         *    Class         : NTP_Client
         *    Description   : Reads the NTP_Sync Intervall
         *    Input         : none
         *    Output        : int32 ( in seconds )
         *    Remarks       : none
         **************************************************************************************************/
        int32_t GetSyncInterval( void );
        
         /**************************************************************************************************
         *    Function      : SetSyncInterval
         *    Class         : NTP_Client
         *    Description   : Sets the NTP_Sync Intervall
         *    Input         : int32 ( in seconds )
         *    Output        : none
         *    Remarks       : none
         **************************************************************************************************/  
        void SetSyncInterval(int32_t);

        
      /**************************************************************************************************
       *    Function      : Tick
       *    Class         : NTP_Client
       *    Description   : Needs to be called every 1 second to do the sync
       *    Input         : none
       *    Output        : none
       *    Remarks       : none
       **************************************************************************************************/
        void Tick(); /* This needs to be called every second */
        
       /**************************************************************************************************
       *    Function      : Sync
       *    Class         : NTP_Client
       *    Description   : Starts a NTP Sync
       *    Input         : none
       *    Output        : none
       *    Remarks       : none
       **************************************************************************************************/
        void Sync( void );
        
         /**************************************************************************************************
         *    Function      : ReadTime
         *    Class         : NTP_Client
         *    Description   : Sets the NTP_Sync Intervall
         *    Input         : bool* 
         *    Output        : none
         *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
         **************************************************************************************************/
        uint32_t ReadTime( bool* delayed_result); 
        
         /**************************************************************************************************
         *    Function      : SaveSettings
         *    Class         : NTP_Client
         *    Description   : Saves the Settings to EEPROM / Flash
         *    Input         : none 
         *    Output        : none
         *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
         **************************************************************************************************/
        void SaveSettings( void );
        
         /**************************************************************************************************
         *    Function      : ReadSettings
         *    Class         : NTP_Client
         *    Description   : Reads the Settings to EEPROM / Flash
         *    Input         : none 
         *    Output        : none
         *    Remarks       : If the result is deliverd delayed it sets the pointed bool to true
         **************************************************************************************************/
        void ReadSettings( void );

         /**************************************************************************************************
         *    Function      : Task
         *    Class         : NTP_Client
         *    Description   : This will trigger a sync
         *    Input         : none 
         *    Output        : none
         *    Remarks       : to avoid problems within isr's
         **************************************************************************************************/
        void Task( void );

    private:
    /**************************************************************************************************
     *    Function      : SyncEvent
     *    Class         : NTP_Client
     *    Description   : Starts the NTP client
     *    Input         : NTPSyncEvent_t event  
     *    Output        : none
     *    Remarks       : Event after a NTP Sync
     **************************************************************************************************/
    void SyncEvent(NTPSyncEvent_t event);  
       
    Timecore* timeptr = NULL;
    volatile ntp_config_t Config;
    volatile int32_t next_update=60;
    volatile bool _sync=false; 

};
 #endif
