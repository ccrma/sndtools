#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "RtAudio.h"
#include "waveio.h"

#define SRATE 44100.0
#define BASE_FREQ 880.0
#define TWO_PI 6.28

bool finished;
bool ready;
char synthBuffer[512];
int syntIn=0,syntOut=0;
int tonecount=0;
int silencecount=0;
FILE *fileOut;
long numWrote=0;
float freqMult=1.0;
float baseFreq = 880.0;
float dotTime = 0.04;
int dotLength= (int) (0.04*SRATE);
int dashLength= (int) (3*0.04*SRATE);
int charSpace= (int) (3*0.04*SRATE);
int wordSpace= (int) (7*0.04*SRATE);

/************************************************/
/*  Circular synthesis input buffer. 		*/
/*  Four types of characters should be in here */
/*  dot '.'    dash '-'  space ' '  and zero	*/
/*  Use push from anywhere to put in a character*/
/*  pop is used by the callback to do synthesis */
/*  Since all timings are derived from the dot  */
/*  all works out nicely (I think).		*/
/*  Dot (·)= 1 unit	Dash (-)  = 3 unit	*/
/* Pause between elements of one character = 1 unit */
/* Pause between the characters of one word = 3 units */
/* Pause between each word = 7 units		*/
/*						*/
/*  So pushing a dot in this buffer causes a dot*/
/*  +1 dot's worth of silence to be synthesized	*/
/*  Pushing a dash causes a dash plus 1dot space*/
/*  Pushing a space causes two dots silence	*/
/*  So after a char, push a space and you get a */
/*  total of three dots silence			*/
/*  After a word, push two more spaces and you  */
/*  will get total 1+2+2+2 = 7 dots silence.	*/
/*						*/
/*  pushing a zero says there's no more	to do   */
/*  for now.					*/
/************************************************/


void pushchar(char myChar)	{
    synthBuffer[syntIn++] = myChar;
    if (syntIn >= 512) syntIn -= 512;
}

char popchar()	{
    char returnChar;
    returnChar = 0;
    if (syntIn != syntOut) {
	returnChar = synthBuffer[syntOut];
	syntOut += 1;
	if (syntOut >= 512) syntOut -= 512;
    }
    return returnChar;
}

/*  Main audio callback.  Synthesis is done here*/
/*  There's some pretty stupid signaling in here*/
/*  (ready is a bit that keeps things in order) */
/*  but it makes the timing reliable (I think)	*/

int callback(char* buffer, int buffer_size, void *data)
{
	float *samples = (float *)buffer;
	int i,newChar;
	char nextChar;
	newChar = 0;
	short shortData;
	for (i=0;i<buffer_size*2;i+=2) {
	    if (tonecount-- > 0) 	{	// COUNT THIS DOWN FIRST
		ready = 0;
		samples[i] = 0.5 * sin((float) tonecount*baseFreq*freqMult*TWO_PI/44100.0);
	    }
	    else if (silencecount-- > 0) {	// THEN COUNT THIS DOWN
		ready = 0;
		samples[i] = 0.0;
	    }
	    else	{			// DEFAULT IS SILENCE ANYWAY
		if (syntIn != syntOut) newChar = 1;
		samples[i] = 0.0;
	    }
	    samples[i+1] = samples[i];
	    if (fileOut)	{
		shortData = (short) (samples[i] * 32767.0);
		fwrite(&shortData,2,1,fileOut);
		numWrote += 1;
	    }
	}
	if (newChar)	{			// do synth based on next char
	    nextChar = popchar();
	    if (nextChar == '.') {
		printf(".");
		tonecount = dotLength; 
		silencecount = dotLength;
	    }
	    else if (nextChar == '-') {
		printf("-");
		tonecount = dashLength;
		silencecount = dashLength;
	    }
	    else if (nextChar == ' ') {
		printf(" ");
		silencecount = wordSpace;
	    }
	    else {
		ready = 1;
		printf("\n");
	    }
	    fflush(stdout);
	}
	return 0;
}

/*  Main */

#define NUM_SYMBOLS 48

int main(int argc, char *argv[])
{
    int i,j,k;
    char inString[512];
    char MorseChars[NUM_SYMBOLS][2] = 
        {"A" ,"B"   ,"C"   ,"D"  ,"E","F"   ,"G"  ,"H"   ,"I" ,
         "J"   , "K" ,"L"   ,"M" ,"N" ,"O"  ,"P"   ,"Q"   ,"R",
         "S"  ,"T","U"  ,"V"   ,"W"  ,"X"   ,"Y"   ,"Z"   ,
         "0"    ,"1"    ,"2"    ,"3"    ,"4",
         "5"    ,"6"    ,"7"    ,"8"    ,"9"    ,
	 "."    ,","    ,"?",   "-", ":", "'", 
	 "(", ")", "=", "@", "+", "/"};  
    char MorseCode[NUM_SYMBOLS][7] = 
        {".-","-...","-.-.","-..",".","..-.","--.","....","..",
         ".---","-.-",".-..","--","-.","---",".--.","--.-",".-.",
         "...","-","..-","...-",".--","-..-","-.--","--..",
         "-----",".----","..---","...--","....-",
         ".....","-....","--...","---..","----.",
	 ".-.-.-","--..--","..--..","-....-","---...",".----.",
	 "-.--.","-.--.-","-...-",".--.-.",".-.-.","-..-."};

/*  More than one symbol, so unimplemented	
Error· · · · · · · ·
Wait· - · · ·
End of Message· - · - ·
Prelim. Call- · - · -
Quotation Marks· - · · - ·
Warning· - · · -
Understood· · · - ·
Tx Received· - ·
Close Signal· · · - · -
Closing Down Tx- · - · · - ·
*/

    struct soundhdr hdr;
    int stream, bufferSize = 512;
    RtAudio *io;

    i = 1;
    if (argc>1)	{
    	while (i < argc)	{
	    if (argv[i][1]=='f')	{
	        fillheader(&hdr,44100);
	    	fileOut = 0;
	    	fileOut = opensoundout(argv[i+1],&hdr);
	    	if (!fileOut)	{
		    printf("Couldn't open your output file\n");
		    exit(0);
	        }
		i += 1;
            }
	    else if (argv[i][1]=='p')	{
	        freqMult = atof(argv[i+1]);
		printf("setting frequency multiplier to %f\n",freqMult);
		i += 1;
	    }
	    else if (argv[i][1]=='b')	{
	        baseFreq = atof(argv[i+1]);
		printf("setting base frequency to %f\n",baseFreq);
		i += 1;
	    }
	    else if (argv[i][1]=='d')	{
	        dotTime = atof(argv[i+1]);
		dotLength = (int) (dotTime*SRATE);
		if (dotLength < 1) dotLength = 1;
		dashLength= 3 * dotLength;
		charSpace= 3 * dotLength;
		wordSpace= 7 * dotLength;
		printf("setting dot time to %f\n",dotTime);
		i += 1;
	    }
            else {
	printf("useage: Morse [args]\n"); 
	printf("        -f filename.snd(wav)\n");
	printf("	-p pitchMult\n");
	printf("	-b base Frequency (default 880.0)\n");
	printf("	-d dotTime (default 0.04 sec.)\n");
	printf("	< filename.txt\n");
	printf("        Translates command line text to Morse code.\n");
	exit(0);
            }
                
	    i += 1;
	}
    }

	try { io = new RtAudio(&stream, 0, 2, 0, 2, RtAudio::RTAUDIO_FLOAT32, 44100, &bufferSize, 4); }
    catch(RtError &error) { return 0; }

	io->setStreamCallback(stream, &(callback), NULL);
	io->startStream(stream);

	finished = false;

    printf("Type characters (case insensitive)\n");
    printf("or invoke with < filename.txt\n");
    printf("can use -s filename.snd(wav) to synthesize a soundfile\n");
    printf("use '-h' to see list of options\n" );
    printf("'Exit' (case sensitive) at beginning of any line ends program\n");
    fflush(stdout); 

    fgets(inString,512,stdin);
    while(!finished)	{
        i = 0;
	while (inString[i])      {
            if (inString[i]>96) inString[i] -= 32;   	// make all upper case
	    if (inString[i]==' ') {
		pushchar(' ');
		pushchar(' ');
//		printf("  ");;
	    }
            j=0;
            while (j<NUM_SYMBOLS)        {
                if (MorseChars[j][0]==inString[i]) {	// lookup character
//                    printf(MorseCode[j]);		// print out code
//                    printf(" ");
//		    fflush(stdout);
                    k = 0;
//                    printf("%c",MorseCode[j][k]);
		    while(MorseCode[j][k]!=0)   {
                        if (MorseCode[j][k]=='.') pushchar('.');
                        if (MorseCode[j][k]=='-') pushchar('-');
                        k += 1;
                    }
		    pushchar(' ');
                }
                j++;
            }
	    i++;
        }
	pushchar(0);
//	printf("\n");
    	usleep(1000);	
        printf(inString);
	while (!ready)	{
	}
	if (fgets(inString,512,stdin) < 0) finished = 1;
        ready = 0;
	if (!strcmp(inString,"Exit\n")) finished = 1;
    }

    io->stopStream(stream);
    delete io;
    if (fileOut) 
	closesoundout(fileOut,numWrote);
}

