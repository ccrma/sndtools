/***********************************************************/
/*     Wave I/O soundfile access routines                  */
/*      EDIT THIS FILE ONCE FOR YOUR ARCHITECTURE          */
/*      #define one of                                     */
/*              LITTLENDIAN (intel, Windows or Linux)      */
/*           or BIGENDIAN (some suns, NeXT, SGI, other)    */
/*      then compile the .c files and enjoy                */
/*                                                         */
/*      LITTLENDIAN will cause the programs to consume     */
/*          and yield .wav files                           */
/*      BIGENDIAN will cause the programs to consume       */
/*          and yield .snd files                           */
/*                                                         */
/*      (c) Perry R. Cook, 2002                            */
/***********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*    #define BIGENDIAN   */
/*    #define LITTLENDIAN   */

#define BIGENDIAN

#ifdef LITTLENDIAN

#define _EXTENSION_ .wav
#define _EXTENSION_STRING_ ".wav"

/*********     WAV BigEndian File Version ***********************/

struct soundhdr {
  char  riff[4];        /* "RIFF"                                  */
  long  flength;        /* file length in bytes                    */
  char  wave[4];        /* "WAVE"                                  */
  char  fmt[4];         /* "fmt "                                  */
  long  block_size;     /* in bytes (generally 16)                 */
  short format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
  short num_chans;      /* 1=mono, 2=stereo                        */
  long  srate;          /* Sampling rate in samples per second     */
  long  bytes_per_sec;  /* bytes per second                        */
  short bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
  short bits_per_samp;  /* Number of bits per sample               */
  char  data[4];        /* "data"                                  */
  long  dlength;        /* data length in bytes (filelength - 44)  */
};

FILE *opensoundin(char *filename,struct soundhdr *hdr)    {
    FILE *myfile;
    /*   check file name for .wav extension */
    if (filename[strlen(filename)-1] == 'v')  {
        myfile = fopen(filename,"rb");
        if (!myfile) {
            printf("Open Input File Error!!\n");
            myfile = 0;
        }
        else {
            fread(hdr,1,44,myfile);
        }
    }
    else {
        printf("You must specify a valid .wav file name\n");
        myfile = 0;
    }
    return myfile;
}

FILE *opensoundout(char *filename, struct soundhdr *hdr)    {
    FILE *myfile;
    /*   check file name for .wav or .snd extension */
    if (filename[strlen(filename)-1] == 'v')   {
        myfile = fopen(filename,"wb");
        if (!myfile) {
            printf("Open Output File Error!!\n");
            myfile = 0;
        }
        else     {
            fwrite(hdr,1,44,myfile);
        }
    }
    else {
        printf("You must specify a valid .wav file name\n");
        myfile = 0;
    }
    return myfile;
}

void fillheader(struct soundhdr *hdr, long srate)      {
    struct soundhdr myhdr = {"RIF",88244,"WAV","fmt",16,1,1,
                                 44100,88200,2,16,"dat",88200};
    myhdr.srate = srate;
    myhdr.bytes_per_sec = srate*2;
    myhdr.riff[3] = 'F';
    myhdr.wave[3] = 'E';
    myhdr.fmt[3] = ' ';
    myhdr.data[3] = 'a';
    *hdr = myhdr;

}

void closesoundout(FILE *file,long num_samples)      {
    long bytes;
    bytes = num_samples * 2;
    fseek(file,40,SEEK_SET); // jump to data length
    fwrite(&bytes,4,1,file);
    bytes = bytes + 44;
    fseek(file,4,SEEK_SET); // jump to total file size
    fwrite(&bytes,4,1,file);
    fclose(file);
}

void closesoundin(FILE *file)      {
    fclose(file);
}

#else

#define _EXTENSION_ .snd
#define _EXTENSION_STRING_ ".snd"

/********  NeXT/Sun BigEndian Soundfile Versions   *******/

struct soundhdr {
  char pref[4];
  long hdr_length;
  long dlength;         /* Data Length in bytes */
  long mode;
  long srate;
  long num_chans;
  char comment[20];
};

FILE *opensoundin(char *filename,struct soundhdr *hdr)    {
    FILE *myfile;
    /*   check file name for .wav or .snd extension */
    if (filename[strlen(filename)-1] == 'd')  {
        myfile = fopen(filename,"rb");
        if (!myfile) {
            printf("Open Input File Error!!\n");
            myfile = 0;
        }
        else {
            fread(hdr,8,1,myfile);
	    fread(&hdr->dlength,1,hdr->hdr_length-8,myfile);
        }
    }
    else {
        printf("You must specify a valid .snd file name\n");
        myfile = 0;
    }
    return myfile;
}

FILE *opensoundout(char *filename, struct soundhdr *hdr)    {
    FILE *myfile;
    /*   check file name for .snd extension */
    if (filename[strlen(filename)-1] == 'd')   {
        myfile = fopen(filename,"wb");
        if (!myfile) {
            printf("Open Output File Error!!\n");
            myfile = 0;
        }
        else     {
            fwrite(hdr,1,hdr->hdr_length,myfile);
        }
    }
    else {
        printf("You must specify a valid .snd file name\n");
        myfile = 0;
    }
    return myfile;
}

void fillheader(struct soundhdr *hdr, long srate)      {
    struct soundhdr myhdr = {".sn",44,0,3,44100,1,"Real Sound Synth. "};
    myhdr.srate = srate;
    myhdr.pref[3] = 'd';
    *hdr = myhdr;
}

void closesoundout(FILE *file,long totalCount)      {
    long bytes;

    fseek(file,8,SEEK_SET);
    bytes = totalCount * 2;
    fwrite(&bytes,4,1,file);
    fclose(file);
}

void closesoundin(FILE *file)      {
    fclose(file);
}

#endif
                          

