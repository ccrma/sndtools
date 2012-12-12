//-----------------------------------------------------------------------------
// file: midiio_osx.h
// desc: midi io header for osx
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//         Ari Lazier (alazier@cs.princeton.edu)
//-----------------------------------------------------------------------------
#ifndef __MIDI_IO_H__
#define __MIDI_IO_H__


#include <CoreMidi/CoreMidi.h>
#include <pthread.h>


#define DWORD__                unsigned int
#define UINT__                 DWORD__
#define BOOL__                 DWORD__
#define FLOAT__                float
#define BYTE__                 unsigned char

#ifndef TRUE
#define TRUE	1
#define FALSE   0
#endif


#ifndef CALLBACK
#define CALLBACK
#endif


//-----------------------------------------------------------------------------
// definitions
//-----------------------------------------------------------------------------
union MidiMsg
{
    BYTE__  data[4];
    DWORD__ dw;
};




//-----------------------------------------------------------------------------
// name: class MidiOut
// desc: midi out
//-----------------------------------------------------------------------------
class MidiOut
{
public:
    MidiOut();
    ~MidiOut();

public:
    BOOL__ open( int device_num = 0 );
    BOOL__ close();
    const char * get_last_error() const;

public:
    UINT__ send( BYTE__ status );
    UINT__ send( BYTE__ status, BYTE__ data1 );
    UINT__ send( BYTE__ status, BYTE__ data1, BYTE__ data2 );
    UINT__ send( const MidiMsg * msg, DWORD__ length = 3 );
    BOOL__ drain();

protected:
    UINT__ m_device_num;
	MIDIClientRef m_midi_out;
	MIDIEndpointRef m_midi_endpoint;
	MIDIPortRef m_midi_port;
    char m_msg[1024];
};




//-----------------------------------------------------------------------------
// name: class CBuffer
// desc: circular buffer
//-----------------------------------------------------------------------------
class CBuffer
{
public:
    CBuffer();
    ~CBuffer();

public:
    BOOL__ initialize( UINT__ num_elem, UINT__ width );
    void cleanup();

public:
    UINT__ get( void * data, UINT__ num_elem );
    void put( void * data, UINT__ num_elem );

protected:
    BYTE__ * m_data;
    UINT__   m_data_width;
    UINT__   m_read_offset;
    UINT__   m_write_offset;
    UINT__   m_max_elem;
};




//-----------------------------------------------------------------------------
// name: class MidiIn
// desc: midi
//-----------------------------------------------------------------------------
class MidiIn
{
public:
    MidiIn();
    ~MidiIn();

public:
    BOOL__ open( int device_num = 0 );
    BOOL__ close();
    const char * get_last_error() const;

public:
    UINT__ recv( MidiMsg * msg );

public:
    static void midi_in_cb( const MIDIPacketList * list,
							void * port_data,
							void * connect_data );

protected:
    UINT__ m_device_num;
    CBuffer m_buffer;
	MIDIClientRef m_midi_in;
	MIDIEndpointRef m_midi_endpoint;
	MIDIPortRef m_midi_port;

    pthread_t m_cb_thread_id;
    pthread_mutex_t m_mutex;
    
    char m_msg[1024];
};




#endif
