//-----------------------------------------------------------------------------
// file: midiio_alsa.cpp
// desc: midi io alsa implementation
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//-----------------------------------------------------------------------------
#include "midiio_alsa.h"
#include <stdio.h>




//-----------------------------------------------------------------------------
// name: MidiOut()
// desc: constructor
//-----------------------------------------------------------------------------
MidiOut::MidiOut()
{
    UINT m_device_num = 0;
    m_midi_out = NULL;
}




//-----------------------------------------------------------------------------
// name: ~MidiOut()
// desc: destructor
//-----------------------------------------------------------------------------
MidiOut::~MidiOut()
{
    this->close();
}




//-----------------------------------------------------------------------------
// name: open()
// desc: open a device
//-----------------------------------------------------------------------------
BOOL MidiOut::open( int device_num )
{
    int err = 0;
    char buffer[256];

    m_device_num = device_num;

    // open the midi
    sprintf( buffer, "hw:0,%d", m_device_num );    
    err = snd_rawmidi_open( NULL, &m_midi_out, buffer, 0 );
    
    return err == 0;
}




//-----------------------------------------------------------------------------
// name: close()
// desc: close the device
//-----------------------------------------------------------------------------
BOOL MidiOut::close( )
{
    // send everything in buffer
    snd_rawmidi_drain( m_midi_out );
    
    // close midi out
    snd_rawmidi_close( m_midi_out );

    return true;
}




//-----------------------------------------------------------------------------
// name: drain()
// desc: ...
//-----------------------------------------------------------------------------
BOOL MidiOut::drain()
{
    snd_rawmidi_drain( m_midi_out );
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 1 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status )
{
    // send
    snd_rawmidi_write( m_midi_out, &status, 1 );

    return true;
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 2 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status, BYTE data1 )
{
    // send
    snd_rawmidi_write( m_midi_out, &status, 1 );
    snd_rawmidi_write( m_midi_out, &data1, 1 );
    
    return true;
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 3 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status, BYTE data1, BYTE data2 )
{
    // send the three bytes
    snd_rawmidi_write( m_midi_out, &status, 1 );
    snd_rawmidi_write( m_midi_out, &data1, 1 );
    snd_rawmidi_write( m_midi_out, &data2, 1 );
    
    return true;
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( const MidiMsg * msg )
{
    return this->send( msg->data[0], msg->data[1], msg->data[2] );
}




//-----------------------------------------------------------------------------
// name: MidiIn()
// desc: constructor
//-----------------------------------------------------------------------------
MidiIn::MidiIn()
{
    m_midi_in = NULL;
    m_device_num = 0;
    pthread_mutex_init( &m_mutex, NULL );
}




//-----------------------------------------------------------------------------
// name: ~MidiIn()
// desc: destructor
//-----------------------------------------------------------------------------
MidiIn::~MidiIn( )
{
    this->close();
}




//-----------------------------------------------------------------------------
// name: open()
// desc: open
//-----------------------------------------------------------------------------
BOOL MidiIn::open( int device_num )
{
  int err = 0;
  char buffer[256];

  m_device_num = device_num;

  // open the raw midi
  sprintf( buffer, "hw:0,%d", m_device_num );  
  err = snd_rawmidi_open( &m_midi_in, NULL, buffer, 0 );

  // initialize the buffer
  m_buffer.initialize( 1024, sizeof( MidiMsg ) );

  // thread
  pthread_create( &m_cb_thread_id, NULL, midi_in_cb, this );

  return err == 0;
}




//-----------------------------------------------------------------------------
// name: close()
// desc: close
//-----------------------------------------------------------------------------
BOOL MidiIn::close()
{
    snd_rawmidi_drain( m_midi_in );
    snd_rawmidi_close( m_midi_in );

    pthread_cancel( m_cb_thread_id );
    pthread_mutex_destroy( &m_mutex );
}




//-----------------------------------------------------------------------------
// name: get()
// desc: get message
//-----------------------------------------------------------------------------
UINT MidiIn::recv( MidiMsg * msg )
{
    UINT r = 0;
    //pthread_mutex_lock( &m_mutex );
    r = m_buffer.get( msg, 1 );
    //pthread_mutex_unlock( &m_mutex );

    return r;
}




//-----------------------------------------------------------------------------
// name: Cbuffer()
// desc: constructor
//-----------------------------------------------------------------------------
CBuffer::CBuffer()
{
    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: ~CBuffer()
// desc: destructor
//-----------------------------------------------------------------------------
CBuffer::~CBuffer()
{
    this->cleanup();
}




//-----------------------------------------------------------------------------
// name: initialize()
// desc: initialize
//-----------------------------------------------------------------------------
BOOL CBuffer::initialize( UINT num_elem, UINT width )
{
    // cleanup
    cleanup();

    // allocate
    m_data = (BYTE *)malloc( num_elem * width );
    if( !m_data )
        return false;

    m_data_width = width;
    m_read_offset = 0;
    m_write_offset = 0;
    m_max_elem = num_elem;

    return true;
}




//-----------------------------------------------------------------------------
// name: cleanup()
// desc: cleanup
//-----------------------------------------------------------------------------
void CBuffer::cleanup()
{
    if( !m_data )
        return;

    free( m_data );

    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: put()
// desc: put
//-----------------------------------------------------------------------------
void CBuffer::put( void * data, UINT num_elem )
{
    UINT i, j;
    BYTE * d = (BYTE *)data;

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            m_data[m_write_offset*m_data_width+j] = d[i*m_data_width+j];
        }

        // move the write
        m_write_offset++;

        // wrap
        if( m_write_offset >= m_max_elem )
            m_write_offset = 0;
    }
}




//-----------------------------------------------------------------------------
// name: get()
// desc: get
//-----------------------------------------------------------------------------
UINT CBuffer::get( void * data, UINT num_elem )
{
    UINT i, j;
    BYTE * d = (BYTE *)data;

    // read catch up with write
    if( m_read_offset == m_write_offset )
        return 0;

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            d[i*m_data_width+j] = m_data[m_read_offset*m_data_width+j];
        }

        // move read
        m_read_offset++;
        
        // catch up
        if( m_read_offset == m_write_offset )
        {
            i++;
            break;
        }

        // wrap
        if( m_read_offset >= m_max_elem )
            m_read_offset = 0;
    }

    // return number of elems
    return 1;
}




//-----------------------------------------------------------------------------
// name: midi_in_cb()
// desc: ...
//-----------------------------------------------------------------------------
void * MidiIn::midi_in_cb( void * arg )
{
    MidiIn * min = (MidiIn *)arg;
    BYTE byte = 0;
    int n = 0, num_args = 0, num_left = 0;
    MidiMsg msg;

    while( true )
    {
        // get the next byte
        n = snd_rawmidi_read( min->m_midi_in, &byte, 1 );
        if( n < 0 )
        {
	  // encounter error
	  fprintf( stdout, "error: rawmidi_read...\n" );
	  continue;
        }

	while( n > 0 )
	  {
	    if( byte & 0x80 ) // status byte
	      {
		if( (byte & 0xf0) == 0xf0 ) // system msg
		  {
		    n--;
		    continue;
		  }

		if( ( (byte & 0xf0) == 0xc0 ) || ( (byte & 0xf0) == 0xd0 ) )
		  num_args = 1;
		else
		  num_args = 2;

		msg.data[0] = byte;
		msg.data[1] = 0;
		msg.data[2] = 0;
		num_left = num_args;
	      }
	    else // data byte
	      {
		if( num_left == num_args )
		  msg.data[1] = byte;
		else
		  msg.data[2] = byte;

		num_left--;

		if( !num_left )
		  {
		    if( ((msg.data[0] & 0xf0) == 0xc0) || ((msg.data[0] & 0xf0) == 0xd0) )
		      num_left = 1;
		    else
		      num_left = 2;

		    //pthread_mutex_lock( &min->m_mutex );
		    min->m_buffer.put( &msg, 1 );
		    //pthread_mutex_unlock( &min->m_mutex );
		  }
	      }

	    n--;
	  }
    }

    return NULL;
}
