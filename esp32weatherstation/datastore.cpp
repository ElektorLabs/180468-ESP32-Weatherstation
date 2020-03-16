#include <EEPROM.h>
#include <CRC32.h>
#include "datastore.h"


/* function prototypes */
void eepwrite_struct(void* data_in, uint32_t e_size, uint32_t address );
bool eepread_struct( void* element, uint32_t e_size, uint32_t startaddr  );

/* This will be the layout used by the data within the flash */
#define CREDENTIALS_START 0
/* credentials are 256+4Byte */
#define TIMECORECONFIG_START 280
/* config is 24 byte + 4 byte */
#define NTP_START 320
/* config is 137+4 byte */
#define NOTES_START 500
/* notes take 512 byte */
#define MQTT_START 2048
/* this takes 1024byte */
/**************************************************************************************************
 *    Function      : datastoresetup
 *    Description   : Gets the EEPROM Emulation set up
 *    Input         : none 
 *    Output        : none
 *    Remarks       : We use 4096 byte for EEPROM 
 **************************************************************************************************/
void datastoresetup() {
  /* We emulate 4096 byte here */
  EEPROM.begin(4096);

}




/**************************************************************************************************
 *    Function      : write_credentials
 *    Description   : writes the wifi credentials
 *    Input         : credentials_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_webcredentials(web_credentials_t c){
   eepwrite_struct( ( (void*)(&c) ), sizeof(web_credentials_t) , CREDENTIALS_START );
}

/**************************************************************************************************
 *    Function      : read_credentials
 *    Description   : reads the wifi credentials
 *    Input         : none
 *    Output        : credentials_t
 *    Remarks       : none
 **************************************************************************************************/
web_credentials_t read_webcredentials( void ){
  web_credentials_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(web_credentials_t) , CREDENTIALS_START ) ){ 
    Serial.println("WEB CREDENTIALS");
    bzero((void*)&retval,sizeof( web_credentials_t ));
    /* We need to read the default values */
    strncpy(retval.user,"admin",128);
    strncpy(retval.pass,"esp32",256);
    write_webcredentials(retval);

  }
  return retval;
}



/**************************************************************************************************
 *    Function      : eepread_struct
 *    Description   : reads a given block from flash / eeprom 
 *    Input         : void* element, uint32_t e_size, uint32_t startaddr  
 *    Output        : bool ( true if read was okay )
 *    Remarks       : Reads a given datablock into flash and checks the the crc32 
 **************************************************************************************************/
bool eepread_struct( void* element, uint32_t e_size, uint32_t startaddr  ){
  
  bool done = true;
  CRC32 crc;
  Serial.println("Read EEPROM");
  uint8_t* ret_ptr=(uint8_t*)(element);
  uint8_t data;
  
  for(uint32_t i=startaddr;i<(e_size+startaddr);i++){
      data = EEPROM.read(i);
      crc.update(data);
      *ret_ptr=data;
      ret_ptr++; 
  }
  /* Next is to read the crc32*/
  uint32_t data_crc = crc.finalize(); 
  uint32_t saved_crc=0;
  uint8_t* bytedata = (uint8_t*)&saved_crc;
  for(uint32_t z=e_size+startaddr;z<e_size+startaddr+sizeof(data_crc);z++){
    *bytedata=EEPROM.read(z);
    bytedata++;
  } 
  
  if(saved_crc!=data_crc){
    Serial.println("SAVED CONF");
    done = false;
  }

  return done;
}

/**************************************************************************************************
 *    Function      : eepwrite_struct
 *    Description   : writes the display settings
 *    Input         : void* data, uint32_t e_size, uint32_t address 
 *    Output        : bool
 *    Remarks       : Writes a given datablock into flash and adds a crc32 
 **************************************************************************************************/
void eepwrite_struct(void* data_in, uint32_t e_size, uint32_t address ){
  Serial.println("Write EEPROM");
  uint8_t* data=(uint8_t*)(data_in);
  CRC32 crc;
  
 
  for(uint32_t i=address;i<e_size+address;i++){
      EEPROM.write(i,*data);
      crc.update(*data);
      data++;
  }
 /* the last thing to do is to calculate the crc32 for the data and write it to the end */
  uint32_t data_crc = crc.finalize(); 
  uint8_t* bytedata = (uint8_t*)&data_crc;
  for(uint32_t z=e_size+address;z<e_size+address+sizeof(data_crc);z++){
    EEPROM.write(z,*bytedata);
    bytedata++;
  } 
  EEPROM.commit();
  
}

/**************************************************************************************************
 *    Function      : write_ntp_config
 *    Description   : writes the ntp config
 *    Input         : ntp_config_t c 
 *    Output        : none
 *    Remarks       : none 
 **************************************************************************************************/
void write_ntp_config(ntp_config_t c){
   eepwrite_struct( ( (void*)(&c) ), sizeof(ntp_config_t) , NTP_START );    
}

/**************************************************************************************************
 *    Function      : read_ntp_config
 *    Description   : writes the ntp config
 *    Input         : none
 *    Output        : ntp_config_t
 *    Remarks       : none
 **************************************************************************************************/
ntp_config_t read_ntp_config( void ){
  ntp_config_t retval;
  if(false ==  eepread_struct( (void*)(&retval), sizeof(ntp_config_t) , NTP_START ) ){
    Serial.println("NTP CONF");
    bzero((void*)&retval,sizeof( ntp_config_t ));
    write_ntp_config(retval);
  } 
  return retval;
}

/**************************************************************************************************
 *    Function      : write_timecoreconf
 *    Description   : writes the time core config
 *    Input         : timecoreconf_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void write_timecoreconf(timecoreconf_t c){
  eepwrite_struct( ( (void*)(&c) ), sizeof(timecoreconf_t) , TIMECORECONFIG_START );  
}


/**************************************************************************************************
 *    Function      : read_timecoreconf
 *    Description   : reads the time core config
 *    Input         : none
 *    Output        : timecoreconf_t
 *    Remarks       : none
 **************************************************************************************************/
timecoreconf_t read_timecoreconf( void ){
  timecoreconf_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(timecoreconf_t) , TIMECORECONFIG_START ) ){ 
    Serial.println("TIME CONF");
    retval = Timecore::GetDefaultConfig();
    write_timecoreconf(retval);
  }
  return retval;
}


/**************************************************************************************************
 *    Function      : eepwrite_notes
 *    Description   : writes the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepwrite_notes(uint8_t* data, uint32_t size){
  for(uint32_t i=NOTES_START;i<512+NOTES_START;i++){
      EEPROM.write(i,0xFF);
  }
  EEPROM.commit();
  eepwrite_struct(data,size,NOTES_START);
}


/**************************************************************************************************
 *    Function      : eepread_notes
 *    Description   : reads the user notes 
 *    Input         : uint8_t* data, uint32_t size
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepread_notes(uint8_t* data, uint32_t size){
  if(size>501){
    size=501;
  }
  
  if( false == (eepread_struct( data, size, NOTES_START  ) ) ){
   Serial.println("Notes corrutp");
   bzero(data,size);
   eepwrite_notes(data,size);
  }
   
  return;
}


/**************************************************************************************************
 *    Function      : eepwrite_mqttsettings
 *    Description   : write the mqtt settings
 *    Input         : mqttsettings_t data
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void eepwrite_mqttsettings(mqttsettings_t data){
  eepwrite_struct( ( (void*)(&data) ), sizeof(mqttsettings_t) , MQTT_START );
}

/**************************************************************************************************
 *    Function      : eepread_mqttsettings
 *    Description   : reads the mqtt settings
 *    Input         : none
 *    Output        : mqttsettings_t
 *    Remarks       : none
 **************************************************************************************************/
mqttsettings_t eepread_mqttsettings( void ){
  
  mqttsettings_t retval;
  if(false == eepread_struct( (void*)(&retval), sizeof(mqttsettings_t) , MQTT_START ) ){ 
    Serial.println("MQTT CONF");
    bzero((void*)&retval,sizeof( mqttsettings_t ));
    eepwrite_mqttsettings(retval);
  }
  return retval;

}

/**************************************************************************************************
 *    Function      : erase_eeprom
 *    Description   : writes the whole EEPROM with 0xFF  
 *    Input         : none
 *    Output        : none
 *    Remarks       : This will invalidate all user data 
 **************************************************************************************************/
void erase_eeprom( void ){
  
 
  for(uint32_t i=0;i<4096;i++){
     EEPROM.write(i,0xFF);
  }
  EEPROM.commit();
 
}






