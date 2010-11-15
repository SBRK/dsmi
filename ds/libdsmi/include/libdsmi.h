/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                DSMI - Nintendo DS Music Interface Library                   //
//                                                                             //
// Version 30-01-2009                                                          //
// http://dsmi.tobw.net | http://code.google.com/p/dsmi/                       //
// by 0xtob (Tobias Weyand) & TheRain (Collin Meyer)                           //
// OSC client by fishuyo                                                       //
// Licensed under LGPL                                                         //
//                                                                             //
// Created with devKitPro (http://www.devkitpro.org) and libnds                //
// Special thanks to sgstair for WiFi lib and all his assistance               //
// and to natrium42 for creating the DSerial and MIDI adapter                  //
// Thanks also to the patrons of GBAdev.org forums, and the #dsdev channel     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


// Message types must be sent with a MIDI Channel # (see MIDI spec for more details)
// Usage like so:  write_MIDI(NOTE_ON|0x01, 60, 127);
//    where the MIDI channel (0-15) is being OR'd with the note on message,
//    60 is the note number, and 127 is the keypress velocity.

// note on is 0x9n where n is the MIDI Channel
// data1 is note number, data2 is key velocity 
#define NOTE_ON 0x90

// note off is 0x8n where n is the MIDI channel
// data1 is note number, data2 is release velocity
#define NOTE_OFF 0x80

// midi CC (control change) is 0xBn where n is the MIDI channel
// data1 is the control number, data2 is the control value 0-127
#define MIDI_CC 0xB0

// midi PC (pitch change) is 0xEn where n is the MIDI channel
// the message is a 14 bit value, where
// data1 is the least significant 7 bits and data2 is the most significant 7 bits
#define MIDI_PC	0xE0

#define DSMI_SERIAL	0
#define DSMI_WIFI	1


#ifdef __cplusplus
extern "C" {
#endif

// ------------ CONNECT ------------ //

// If a DSerial is inserted, this sets up the connection to the DSerial.
// Else, it connects to the default access point stored in Nintendo WFC
// memory (use an official game, e.g. mario kart to set this up)
// The initialized interface is set as the default interface.
//
// Returns 1 if connected, and 0 if failed.
extern int dsmi_connect(void);

// Using these you can force a wifi connection even if a DSerial is
// inserted or set up both connections for forwarding.
extern int dsmi_connect_dserial(void);
extern int dsmi_connect_wifi(void);



// ------------ WRITE ------------ //

// Send a MIDI message over the default interface, see MIDI spec for more details
extern void dsmi_write(u8 message,u8 data1, u8 data2);

// Force a MIDI message to be sent over DSerial
extern void dsmi_write_dserial(u8 message,u8 data1, u8 data2);

// Force a MIDI message to be sent over Wifi
extern void dsmi_write_wifi(u8 message,u8 data1, u8 data2);

// ------------ OSC WRITE ------------ //
// OSC messages are sent only over wifi and do not require the dsmidiwifi server application
//   To send and OSC message:
//     1. call dsmi_osc_new with an OSC address string ( i.e. anything starting with '/' )
//     2. add up to 31 arguments (an arbitrary limit I set) given that none of the arguments are gigantic strings
//     3. call dsmi_osc_send()
//     4. repeat!

// Resets the OSC buffer and sets the destination open sound control address, returns 1 if ok, 0 if address string not valid
extern int dsmi_osc_new( char* addr);

// Adds arguments to the OSC packet
extern int dsmi_osc_addintarg( long arg);
extern int dsmi_osc_addfloatarg( float arg);
extern int dsmi_osc_addstringarg( char* arg);

// Sends the OSC packet
extern int dsmi_osc_send(void);

// ------------ READ ------------ //

extern void dsmi_set_read_callback(void (*onData_)(u8 message, u8 data1, u8 data2));

// Checks if a new message arrived at the default interface and returns it by
// filling the given pointers
//
// Returns 1, if a message was received, 0 if not
extern int dsmi_read(u8* message, u8* data1, u8* data2);

// Force receiving over DSerial
// extern int dsmi_read_dserial(u8* message, u8* data1, u8* data2); // TODO

// Force receiving over Wifi
extern int dsmi_read_wifi(u8* message, u8* data1, u8* data2);


// ------------ OSC READ-------- //
//Returns 1 if message was received, 0 otherwise
extern int dsmi_osc_read();
//returns pointer to address string in osc message
extern const char* dsmi_osc_getaddr();
//get next argument in osc message
//data is buffer to fill with argument data
//size is size of buffer in, and written data size out
//type is either 'i' 'f' or 's' for int, float, or string
//returns 1 if success, 0 if no more arguments, -1 buffer not big enough, -2 other error
extern int dsmi_osc_getnextarg( void* data, size_t* size, char* type );

// ------------ MISC ------------ //

// Returns the default interface (DSMI_SERIAL or DSMI_WIFI)
extern int dsmi_get_default_interface(void);



#ifdef __cplusplus
};
#endif
