
//  by fishuyo
//    Basic OSC client implementation with fixed maximum packet size...
//    Can add up to 31 arguments to OSC packets given none are gigantic strings

#include <string.h>

#include <netinet/in.h>

#include "osc_client.h"

void osc_init( OSCbuf* buf){
	
  buf->pos = 0;
  buf->numargs = 0;
  buf->posTypeString = 0;
  buf->status = OSC_EMPTY;

}
int osc_writeaddr( OSCbuf* buf, char* addr){
  
  if (buf->status != OSC_EMPTY) return 0;
  if ( addr[0] != '/') return 0;
  if ( strlen( addr ) >= OSC_MAX_SIZE) return 0;
  
  buf->posTypeString = osc_copyPaddedString( buf->buffer, addr);
  buf->buffer[buf->posTypeString] = ',';
  buf->pos = buf->posTypeString + OSC_MAX_ARGS;
  buf->status = OSC_ADDRESS;
  
  return 1;

}

int osc_addintarg( OSCbuf* buf, long arg){
  
  long convInt;
  if( buf->status != OSC_ADDRESS) return 0;
  if( buf->numargs >= OSC_MAX_ARGS - 1) return 0;
  if( buf->pos + 4 > OSC_MAX_SIZE - 1) return 0;
  
  buf->numargs++;
  buf->buffer[ buf->posTypeString + buf->numargs] = 'i';
  convInt = ntohl( arg );
  memcpy( &buf->buffer[buf->pos], &convInt, 4);
  buf->pos += 4;
  return 1;

}
int osc_addfloatarg( OSCbuf* buf, float arg){
  
  long* convFloat = (long*)&arg;
  if( buf->status != OSC_ADDRESS) return 0;
  if( buf->numargs >= OSC_MAX_ARGS - 1) return 0;
  if( buf->pos + 4 > OSC_MAX_SIZE - 1) return 0;
  
  buf->numargs++;
  buf->buffer[ buf->posTypeString + buf->numargs] = 'f';
  *convFloat = ntohl( *convFloat );
  memcpy( &buf->buffer[buf->pos], convFloat, 4);
  buf->pos += 4;
  return 1;  
}
int osc_addstringarg( OSCbuf* buf, char* arg){

  if( buf->status != OSC_ADDRESS) return 0;
  if( buf->numargs >= OSC_MAX_ARGS - 1) return 0;
  if( buf->pos + osc_stringLength( arg) > OSC_MAX_SIZE - 1) return 0;
  
  buf->numargs++;
  buf->buffer[ buf->posTypeString + buf->numargs] = 's';
  buf->pos += osc_copyPaddedString( &buf->buffer[ buf->pos], arg); 
  return 1;
  
}

char* osc_getPacket( OSCbuf* buf ){
  
  int i, pos, pad;
  
  pad = (4 - ((buf->numargs + 1) % 4)) % 4;
  pos = buf->posTypeString + buf->numargs + 1 + pad;
  for( i = buf->posTypeString + buf->numargs + 1; pad > 0; pad--)
    buf->buffer[i++] = '\0';
  for( i = buf->posTypeString + OSC_MAX_ARGS; i < buf->pos; i++)
    buf->buffer[pos++] = buf->buffer[i];
	
  buf->pos = pos;
  buf->status = OSC_PACKED;
  return buf->buffer;
  
}

int osc_getPacketSize( OSCbuf* buf ){
  
  if( buf->status == OSC_PACKED)
    return buf->pos;
  else
    return buf->pos - (OSC_MAX_ARGS - (buf->numargs + 1 + ((4 - (buf->numargs + 1)) % 4)));
  
}

int osc_copyPaddedString(char* dest, char* src) {
  
  int len, pad, i;

  strcpy(dest, src);
  len = strlen(src) + 1;
  pad = (4 - len % 4) % 4;
  
  for( i = len; pad > 0; pad--)
    dest[i++] = '\0';

  return i;
}

int osc_stringLength( char* str) {
  
  int len, pad;
  len = strlen( str) + 1;
  pad = (4 - len % 4) % 4;
  return len + pad;
}