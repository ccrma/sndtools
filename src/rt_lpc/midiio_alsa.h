//-----------------------------------------------------------------------------
// file: midiio_alsa.h
// desc: midi io header for alsa
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//-----------------------------------------------------------------------------
#ifndef __MIDI_IO_H__
#define __MIDI_IO_H__


#include <alsa/asoundlib.h>
#include <pthread.h>


#ifndef DWORD
#define DWORD unsigned long
#define BYTE  unsigned char
#define UINT  unsigned int
#define BOOL  DWORD
#endif

#ifndef CALLBACK
#define CALLBACK
#endif


//-----------------------------------------------------------------------------
// definitions
//-----------------------------------------------------------------------------
union MidiMsg
{
    BYTE  data[4];
    DWORD dw;
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
    BOOL open( int device_num = 0 );
    BOOL close();

public:
    UINT send( BYTE status );
    UINT send( BYTE status, BYTE data1 );
    UINT send( BYTE status, BYTE data1, BYTE data2 );
    UINT send( const MidiMsg * msg );
    BOOL drain();

protected:
    UINT m_device_num;
    snd_rawmidi_t * m_midi_out;
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
    BOOL initialize( UINT num_elem, UINT width );
    void cleanup();

public:
    UINT get( void * data, UINT num_elem );
    void put( void * data, UINT num_elem );

protected:
    BYTE * m_data;
    UINT   m_data_width;
    UINT   m_read_offset;
    UINT   m_write_offset;
    UINT   m_max_elem;
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
    BOOL open( int device_num = 0 );
    BOOL close();

public:
    UINT recv( MidiMsg * msg );

public:
    static void * midi_in_cb( void * min );

protected:
    UINT m_device_num;
    snd_rawmidi_t * m_midi_in;
    CBuffer m_buffer;

    pthread_t m_cb_thread_id;
    pthread_mutex_t m_mutex;
};




#endif
