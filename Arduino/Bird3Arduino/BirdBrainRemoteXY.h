/*
   -- BirdBrain --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.10 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.13.11 or later version;
     - for iOS 1.10.3 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "BirdBrain"
#define REMOTEXY_ACCESS_PASSWORD "randompass"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 94 bytes
  { 255,1,0,0,0,87,0,17,0,0,0,25,1,106,200,1,1,5,0,129,
  9,6,90,12,191,69,83,80,51,50,32,66,105,114,100,32,66,114,97,105,
  110,0,3,8,26,23,63,3,191,26,129,37,34,24,8,191,69,110,97,98,
  108,101,0,129,37,54,27,8,191,68,105,115,97,98,108,101,0,129,37,74,
  42,8,191,76,111,99,107,32,87,104,101,101,108,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t ScooterState; // =0 if select position A, =1 if position B ...

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////


