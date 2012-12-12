//-----------------------------------------------------------------------------
// file: midiio_win32.cpp
// desc: midi io
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include "midiio_win32.h"




//-----------------------------------------------------------------------------
// global variables
//-----------------------------------------------------------------------------
UINT g_midi_device;
#define BUFFER_SIZE 8192




//-----------------------------------------------------------------------------
// name: MidiOut()
// desc: constructor
//-----------------------------------------------------------------------------
MidiOut::MidiOut()
{
    m_device_num = 0;
    m_midi_out = NULL;
}




//-----------------------------------------------------------------------------
// name: ~MidiOut()
// desc: destructor
//-----------------------------------------------------------------------------
MidiOut::~MidiOut()
{
    if( m_midi_out ) this->close();
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 1 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status )
{
    return this->send( status, 0, 0 );
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 2 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status, BYTE data1 )
{
    return this->send( status, data1, 0 );
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send 3 byte midi message
//-----------------------------------------------------------------------------
UINT MidiOut::send( BYTE status, BYTE data1, BYTE data2 )
{
    MidiMsg msg;

    msg.data[0] = status;
    msg.data[1] = data1;
    msg.data[2] = data2;
    msg.data[3] = 0;

    return midiOutShortMsg( m_midi_out, msg.dw );
}




//-----------------------------------------------------------------------------
// name: send()
// desc: send
//-----------------------------------------------------------------------------
UINT MidiOut::send( const MidiMsg * msg )
{
    return midiOutShortMsg( m_midi_out, msg->dw );
}




//-----------------------------------------------------------------------------
// name: open
// desc: open midi output
//-----------------------------------------------------------------------------
BOOL MidiOut::open( UINT device_num )
{
    // close if already opened
    if( m_midi_out )
        this->close();

    // copy
    m_device_num = device_num;

    // open the midi out
    return midiOutOpen( &m_midi_out, m_device_num,
                        (DWORD)cb_midi_output, NULL, CALLBACK_FUNCTION ) == 0;
}




//-----------------------------------------------------------------------------
// name: close( )
// desc: close midi output
//-----------------------------------------------------------------------------
BOOL MidiOut::close( )
{
    if( !m_midi_out )
        return FALSE;

    // turn off everything
    midiOutReset( m_midi_out );

    // close
    midiOutClose( m_midi_out );

    // set to NULL
    m_midi_out = NULL;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: noteon()
// desc: note on message
//-----------------------------------------------------------------------------
UINT MidiOut::noteon( UINT channel, UINT note, UINT velocity )
{ return this->send( (BYTE)(MIDI_NOTEON + channel), note, velocity ); }




//-----------------------------------------------------------------------------
// name: noteoff()
// desc: note off message
//-----------------------------------------------------------------------------
UINT MidiOut::noteoff( UINT channel, UINT note, UINT velocity )
{ return this->send( (BYTE)(MIDI_NOTEOFF + channel), note, velocity ); }




//-----------------------------------------------------------------------------
// name: polypress()
// desc: polypress message
//-----------------------------------------------------------------------------
UINT MidiOut::polypress( UINT channel, UINT note, UINT pressure )
{ return this->send( (BYTE)(MIDI_POLYPRESS + channel), note, pressure ); }




//-----------------------------------------------------------------------------
// name: ctrlchange()
// desc: ctrl change message
//-----------------------------------------------------------------------------
UINT MidiOut::ctrlchange( UINT channel, UINT ctrl_num, UINT ctrl_val )
{ return this->send( (BYTE)(MIDI_CTRLCHANGE + channel), ctrl_num, ctrl_val ); }




//-----------------------------------------------------------------------------
// name: progchange()
// desc: prog change message
//-----------------------------------------------------------------------------
UINT MidiOut::progchange( UINT channel, UINT patch )
{ return this->send( (BYTE)(MIDI_PROGCHANGE + channel), patch, 0 ); }




//-----------------------------------------------------------------------------
// name: chanpress()
// desc: chan press
//-----------------------------------------------------------------------------
UINT MidiOut::chanpress( UINT channel, UINT pressure )
{ return this->send( (BYTE)(MIDI_CHANPRESS + channel), pressure, 0 ); }




//-----------------------------------------------------------------------------
// name: pitchbend()
// desc: pitch bend
//-----------------------------------------------------------------------------
UINT MidiOut::pitchbend( UINT channel, UINT bend_val )
{
    return this->send( (BYTE)(MIDI_PITCHBEND + channel),
                       (BYTE)(HIBYTE( bend_val << 1 )),
                       (BYTE)(LOBYTE( bend_val & 0x7f )) );
}




//-----------------------------------------------------------------------------
// name: allnotesoff()
// desc: allnotesoff
//-----------------------------------------------------------------------------
UINT MidiOut::allnotesoff( UINT channel )
{
    return this->send( (BYTE)(MIDI_CTRLCHANGE + channel), 
                     (BYTE)(MIDI_ALLNOTESOFF), 0 );
}




//-----------------------------------------------------------------------------
// name: cb_midi_output
// desc: call back
//-----------------------------------------------------------------------------
void CALLBACK MidiOut::cb_midi_output( HMIDIOUT hm_out, UINT msg,
                       DWORD instance, DWORD param1, DWORD param2 )
{
    MMRESULT result;
    MIDIHDR * mhdr = NULL;

    switch( msg )
    {
    case MOM_OPEN:
        break;
    case MOM_CLOSE:
        break;
    case MOM_DONE:
        mhdr = (MIDIHDR*)param1;

        result = midiOutUnprepareHeader( hm_out, mhdr, sizeof(MIDIHDR) );

        if( result != 0 )
        {
            // TODO: handle the error
            break;
        }

        if( mhdr->lpData )
            free( mhdr->lpData );

        break;
    case MOM_POSITIONCB:
        mhdr = (MIDIHDR*)param2;
        break;
    }
}




//-----------------------------------------------------------------------------
// name: MidiIn()
// desc: constructor
//-----------------------------------------------------------------------------
MidiIn::MidiIn()
{
    m_midi_in = NULL;
    m_device_num = 0;
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
BOOL MidiIn::open( UINT device_num )
{
    MMRESULT result;
    if( m_midi_in )
        this->close();

    m_device_num = device_num;

    // open the midi
    result = midiInOpen( &m_midi_in, m_device_num, (DWORD)cb_midi_input,
                         (DWORD)this, CALLBACK_FUNCTION );

    // check result
    if( result != 0 )
        return FALSE;

    // allocate the buffer
    if( !m_buffer.initialize( BUFFER_SIZE, sizeof(MidiMsg) ) )
    {
        this->close();
        return FALSE;
    }

    return midiInStart( m_midi_in ) == 0;
}




//-----------------------------------------------------------------------------
// name: close()
// desc: close
//-----------------------------------------------------------------------------
BOOL MidiIn::close()
{
    if( !m_midi_in )
        return FALSE;

    // midi reset
    midiInReset( m_midi_in );
    
    // midi close
    midiInClose( m_midi_in );

    // set to NULL
    m_midi_in = NULL;

    // deallocate the buffer
    m_buffer.cleanup();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: get()
// desc: get message
//-----------------------------------------------------------------------------
UINT MidiIn::recv( MidiMsg * msg )
{
    return m_buffer.get( msg, 1 );
}




//-----------------------------------------------------------------------------
// name: cb_midi_input()
// desc: cb
//-----------------------------------------------------------------------------
void CALLBACK MidiIn::cb_midi_input( HMIDIIN hm_in, UINT msg, DWORD instance,
                                     DWORD param1, DWORD param2 )
{
    MidiMsg m;
    MidiIn * midi_in = (MidiIn *)instance;

    switch( msg )
    {
    case MIM_OPEN:
        break;

    case MIM_CLOSE:
        break;

    case MIM_DATA:
        m.dw = param1;

        // put in the buffer, make sure not active sensing
        if( m.data[0] != 0xfe )
        {
            midi_in->m_buffer.put( &m, 1 );
        }
        break;

    case MIM_ERROR:
        break;

    case MIM_LONGDATA:
        break;

    case MIM_LONGERROR:
        break;
    }
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
        return FALSE;

    m_data_width = width;
    m_read_offset = 0;
    m_write_offset = 0;
    m_max_elem = num_elem;

    return TRUE;
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
    return i;
}
