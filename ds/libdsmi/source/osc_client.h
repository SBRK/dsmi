
//  by fishuyo
//    Basic OSC client implementation with fixed maximum packet size...
//    Can add up to 31 arguments to OSC packets given none are gigantic strings


#define OSC_MAX_SIZE 256
#define OSC_MAX_ARGS 32  //actually 31 but type string starts with ',' which takes up one of the bytes

#define OSC_EMPTY 0
#define OSC_ADDRESS 1
#define OSC_PACKED 2
#define OSC_DECODED 3

typedef struct OSCbuf_struct {
    char buffer[OSC_MAX_SIZE];            // The buffer to hold the OSC packet 
    int pos;            //offset of next empty position as we fill the buffer 
    int numargs;		     
    int posTypeString;  //offset to beginning of type string
	int status;			//keep track of packet's development
} OSCbuf;

//OSC client functions (building osc messages)
void osc_init( OSCbuf* buf);
int osc_writeaddr( OSCbuf* buf, char* addr);

int osc_addintarg( OSCbuf* buf, long arg);
int osc_addfloatarg( OSCbuf* buf, float arg);
int osc_addstringarg( OSCbuf* buf, char* arg);

char* osc_getPacket( OSCbuf* buf );
int osc_getPacketSize( OSCbuf* buf );

int osc_copyPaddedString(char *dest, char *src);
int osc_stringLength( char* str);

//OSC server functions (decoding osc messages)
  

int osc_decodePacket( OSCbuf* buf);
const char* osc_getaddr( OSCbuf* buf);
int osc_getnextarg( OSCbuf* buf, void *data, size_t* size, char* type);
