//------------------------------------------------------------------------------
// name: sndview.c
// desc: View class derived from Point
//
// author: Perry R. Cook (prc@cs.princeton.edu)
// date: today
//------------------------------------------------------------------------------
#ifdef __MACOSX_CORE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
// Or some variant of these, depending on architecture and install location
//    link with glut32.lib or whatever (Frameworks on MacOSX)
//              -lgl on Fedora

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//  #include <conio.h>       // for getch() function on some compilers

char file_name[256];

#define RAW 0
#define WAV 1
#define SND 2
#define FLT 3

#define MIN_WIN 16
#define MAX_WIN 65536
#define MAX_WIN_TYPE 5

int Size;
int Beginning;
int WinType = 0;
int FileType = 0;	// RAW=0, WAV=1, SND=2, or FLT=3
int SwapMode = 0;	// 0 = don't swap
long HdrLength;
long Srate;
long Location=0;
long CursorPos=0;
float FreqScale;
FILE *MyFile;
int CalcSpectrum;
int ByteMode=0;
long MySize;

float drawC[4] = {0,1,0,0};
float clearC[4] = {0,0,0,0};

void swaplong(long *along)	{
    unsigned char *temp,temp2;
    temp = (unsigned char *) along;
    temp2 = temp[0];
    temp[0] = temp[3];
    temp[3] = temp2;
    temp2 = temp[1];
    temp[1] = temp[2];
    temp[2] = temp2;
}

void swapshort(short *ashort)	{
    unsigned char *temp,temp2;
    temp = (unsigned char *) ashort;
    temp2 = temp[0];
    temp[0] = temp[1];
    temp[1] = temp2;
}

SndView(int InitLocation, int InitSize,
        int InitByteMode, int InitSwapMode, char *InitFileName)
{
    long i = 0;
    int k = 1;
    short data[256];
    Size = InitSize;
    FreqScale = 1.0;
    Location = InitLocation;
    CalcSpectrum = 1;
    ByteMode = InitByteMode;
    SwapMode = InitSwapMode;
    if (ByteMode) {
        while(k) {
            k = fread(&data,1,256,MyFile);
            i += k;
        }
    }
    else {
        while(k) {
            k = fread(&data,2,256,MyFile);
            i += k;
        }
    }

    MySize = i;
    CursorPos = 32;
    Srate = -1;
    WinType = 3;
    Beginning = 0;
    if( file_name[strlen(InitFileName)-4] == '.'
        && file_name[strlen(InitFileName)-3] == 'w') {
        FileType = WAV;
        Beginning = 44;
        fseek(MyFile,24,0);
        fread(&Srate,4,1,MyFile);
#ifdef BIGENDIAN
        swaplong(&Srate);
#endif
    }
    else if( file_name[strlen(InitFileName)-4] == '.'
        && file_name[strlen(InitFileName)-3] == 's') {
        FileType = SND;
        fseek(MyFile,4,0);
        fread(&HdrLength,4,1,MyFile);
        fseek(MyFile,16,0);
        fread(&Srate,4,1,MyFile);
#ifdef LITTLENDIAN
        swaplong(&HdrLength);
        swaplong(&Srate);
#endif
        Beginning = HdrLength;
    }
    return 1;
};


#define PI 3.141592654782
#define SQRT_TWO 1.414213562
#define TWO_PI 6.283185309564

//  #define MAX_WIN 5
void rectangle(int size,float* array)
{
    //  Dummy, don't do anything
    ;
}

void triangle(int size,float* array)
{
    int i;
    float w = 0.0, delta;
    delta = 2.0 / size;
    for (i=0;i<size/2;i++) {
        array[i] *= w;
        array[size-i] *= w;
        w += delta;
    }
}

void hanning(int size,float* array)
{
    int i;
    float w,win_frq;
    win_frq = TWO_PI / size;
    for (i=0;i<size/2;i++) {
        w = 0.5 - 0.5 * cos(i * win_frq);
        array[i] *= w;
        array[size-i] *= w;
    }
}

void hamming(int size,float* array)
{
    int i;
    float w,win_frq;
    win_frq = TWO_PI / size;
    for (i=0;i<size/2;i++) {
        w = 0.54 - 0.46 * cos(i * win_frq);
        array[i] *= w;
        array[size-i] *= w;
    }
}

void blackman3(int size,float* array)
{
    int i;
    float w,win_frq;
    win_frq = TWO_PI / size;
    for (i=0;i<size/2;i++) {
        w = (0.42 - 0.5 * cos(i * win_frq) + 0.08 * cos(2 * i * win_frq));
        array[size-i] *= w;
        array[i] *= w;
    }
}

void blackman4(int size,float* array)
{
    int i;
    float w,win_frq;
    win_frq = TWO_PI / size;
    for (i=0;i<size/2;i++) {
        w = (0.35875 - 0.48829 * cos(i * win_frq) +
             0.14128 * cos(2 * i * win_frq) - 0.01168 * cos(3 * i * win_frq));
        array[i] *= w;
        array[size-i] *= w;
    }
}

int last_length = 0;

void fhtRX4(int powerOfFour, float *array)
{
    /*  In place Fast Hartley Transform of floating point data in array.
	Size of data array must be power of four. Lots of sets of four
	inline code statements, so it is verbose and repetitive, but fast.
	A 1024 point FHT takes approximately 80 milliseconds on the NeXT computer
	(not in the DSP 56001, just in compiled C as shown here).

	The Fast Hartley Transform algorithm is patented, and is documented
	in the book "The Hartley Transform", by Ronald N. Bracewell.
	This routine was converted to C from a BASIC routine in the above book,
	that routine Copyright 1985, The Board of Trustees of Stanford University       */

    register int j=0,i=0,k=0,L=0;
    int n=0,n4=0,d1=0,d2=0,d3=0,d4=0,d5=1,d6=0,d7=0,d8=0,d9=0;
    int L1=0,L2=0,L3=0,L4=0,L5=0,L6=0,L7=0,L8=0;
    float r=0.0;
    float a1=0,a2=0,a3=0;
    float t=0.0,t1=0.0,t2 =0.0,t3=0.0,t4=0.0,t5=0.0,t6=0.0,t7=0.0,t8=0.0;
    float t9=0.0,t0=0.0;
    float c1,c2,c3,s1,s2,s3;

    n = pow(4.0 , (double) powerOfFour);
    if (n!=last_length) {
//      make_sines(n);
        last_length = n;
    }
    n4 = n / 4;
    r = SQRT_TWO;
    j = 1;
    i = 0;
    while (i<n-1) {
        i++;
        if (i<j) {
            t = array[j-1];
            array[j-1] = array[i-1];
            array[i-1] = t;
        }

        k = n4;
        while ((3*k)<j) {
            j -= 3 * k;
            k /= 4;
        }
        j += k;
    }

    for (i=0;i<n;i += 4) {
        t5 = array[i];
        t6 = array[i+1];
        t7 = array[i+2];
        t8 = array[i+3];
        t1 = t5 + t6;
        t2 = t5 - t6;
        t3 = t7 + t8;
        t4 = t7 - t8;
        array[i] = t1 + t3;
        array[i+1] = t1 - t3;
        array[i+2] = t2 + t4;
        array[i+3] = t2 - t4;
    }

    for (L=2;L<=powerOfFour;L++) {
        d1 = pow(2.0 , L+L-3.0);
        d2=d1+d1;
        d3=d2+d2;
        d4=d2+d3;
        d5=d3+d3;
        for (j=0;j<n;j += d5) {
            t5 = array[j];
            t6 = array[j+d2];
            t7 = array[j+d3];
            t8 = array[j+d4];
            t1 = t5+t6;
            t2 = t5-t6;
            t3 = t7+t8;
            t4 = t7-t8;
            array[j] = t1 + t3;
            array[j+d2] = t1 - t3;
            array[j+d3] = t2 + t4;
            array[j+d4] = t2 - t4;
            d6 = j+d1;
            d7 = j+d1+d2;
            d8 = j+d1+d3;
            d9 = j+d1+d4;
            t1 = array[d6];
            t2 = array[d7] * r;
            t3 = array[d8];
            t4 = array[d9] * r;
            array[d6] = t1 + t2 + t3;
            array[d7] = t1 - t3 + t4;
            array[d8] = t1 - t2 + t3;
            array[d9] = t1 - t3 - t4;
            for (k=1;k<d1;k++) {
                L1 = j + k;
                L2 = L1 + d2;
                L3 = L1 + d3;
                L4 = L1 + d4;
                L5 = j + d2 - k;
                L6 = L5 + d2;
                L7 = L5 + d3;
                L8 = L5 + d4;
                a1 = (float) k / (float) d3 * PI;
                a2 = a1 + a1;
                a3 = a1 + a2;
                c1 = cos(a1);
                c2 = cos(a2);
                c3 = cos(a3);
                s1 = sin(a1);
                s2 = sin(a2);
                s3 = sin(a3);
                t5 = array[L2] * c1 + array[L6] * s1;
                t6 = array[L3] * c2 + array[L7] * s2;
                t7 = array[L4] * c3 + array[L8] * s3;
                t8 = array[L6] * c1 - array[L2] * s1;
                t9 = array[L7] * c2 - array[L3] * s2;
                t0 = array[L8] * c3 - array[L4] * s3;
                t1 = array[L5] - t9;
                t2 = array[L5] + t9;
                t3 = - t8 - t0;
                t4 = t5 - t7;
                array[L5] = t1 + t4;
                array[L6] = t2 + t3;
                array[L7] = t1 - t4;
                array[L8] = t2 - t3;
                t1 = array[L1] + t6;
                t2 = array[L1] - t6;
                t3 = t8 - t0;
                t4 = t5 + t7;
                array[L1] = t1 + t4;
                array[L2] = t2 + t3;
                array[L3] = t1 - t4;
                array[L4] = t2 - t3;
            }
        }
    }
}

void logMag(int size, float *array, float floor, float ceiling)
{
    int i;
    float t1=0.0,t=0.0,t2=-12;
    t2 = floor / 10.0;
    t1 = ceiling;
    array[0] = 2 * fabs(array[0]);
    if (ceiling>0.0) {
        if (array[0] > t1)
        t1 = array[0];
    }

    for (i=1;i<size/2;i++) {
        t = array[i]*array[i] + array[size-i]*array[size-i];
        array[i] = t;
        if (ceiling>0.0) {
            if (t>t1)
                t1 = t;
        }
    }
//    printf("%f\n",t1);
    if (t1>0.0) {
      for (i=0;i<size/2;i++) {
        if (array[i]>0.0) {
          t = log10(array[i]/t1);
          if (t<t2)
            t = t2;
          array[i] = 1 - t/t2;
        }
        else array[i] = 0.0;
      }
    }
    else for (i=0;i<size/2;i++) array[i] = 0.0;
}

float fitParabola(float leftSample,float midSample,float rightSample)
{
    float p;
    p = (leftSample - (2 * midSample) + rightSample);
    if (p!=0.0) p = (leftSample - rightSample) / p  * 0.5;
    return p;
}

static float xscale = 1.0/320.0;
static float yscale = 1.0/240.0;

void line(int x, int y, int x2, int y2)
{
    glBegin(GL_LINES);
    glVertex2f(((float) x * xscale) - 1.0, 1.0 - (float) y * yscale);
    glVertex2f(((float) x2 * xscale) - 1.0, 1.0 - (float) y2 * yscale);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(((float) x * xscale) - 1.0, 1.0 - (float) (y+1) * yscale);
    glVertex2f(((float) x2 * xscale) - 1.0, 1.0 - (float) (y2+1) * yscale);
    glEnd();
}

void outtext(int x, int y, char* string) {
    glRasterPos2f(((float) x*xscale) - 1.0, 1.0 - ((float) y*yscale));
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,*string++);
    }
}

void Redraw(void) {
    long i,tempi;
    int j,k,x,y,xL,yL;
    short data[MAX_WIN];
    unsigned char bdata[MAX_WIN];
    float fdata[MAX_WIN];
    char out_string[256],temp_string[20];
    int xoffset,yoffset = 128,yoffset3 = yoffset*3.3,yfscale=160;
    float xscale,temp,temp2;
    int maxLoc = 0;

    if (ByteMode) {
        k = fseek(MyFile,Location,0);
        k = fread(&bdata,1,Size,MyFile);
        for (i=0;i<k;i++) {
            data[i] = (bdata[i] - 128) << 8;
        }
    }
    else {
        if (FileType == 3) {
            k = fseek(MyFile,Location*4,0);
            k = fread(&fdata,4,Size,MyFile);
            for (i=0;i<k;i++) data[i] = fdata[i];
        }
        else {
            k = fseek(MyFile,Location*2,0);
            k = fread(&data,2,Size,MyFile);
        }
    }
    if (k<Size)
        for (i=k;i<Size;i++) data[i] = 0;
#ifdef BIGENDIAN
        if (FileType==WAV) for (i=0;i<Size;i++) swapshort(&data[i]);
#endif
#ifdef LITTLENDIAN
        if (FileType==SND) for (i=0;i<Size;i++) swapshort(&data[i]);
#endif
    if (SwapMode==1) for (i=0;i<Size;i++) swapshort(&data[i]);

    line(20,yoffset,620,yoffset);
    line(10,1,630,1);
    line(10,yoffset*2,630,yoffset*2);
    line(10,1,10,yoffset*2);
    line(320,11,320,yoffset*2 - 30);
    line(630,1,630,yoffset*2);

    strcpy(out_string,"Block Size= ");
    sprintf(temp_string,"%i ",Size);
    strcat(out_string,temp_string);
    outtext(150,240,out_string);

    sprintf(temp_string,"%li ",Location);
    outtext(20,240,temp_string);
    tempi = Location + (Size / 2);
    if (FileType==RAW) {
        sprintf(temp_string,"%li ",tempi);
    }
    else {
        sprintf(temp_string,"%li (%.3f s.)",tempi, (float) tempi /(float) Srate);
    }
    outtext(310,240,temp_string);
    sprintf(temp_string,"%li ",Location + Size);
    outtext(590,240,temp_string);

    xscale = 600.0 / (float) Size;
    xL = 20;
    yL = yoffset - (data[0]>>8);
    for (i=1;i<Size;i++) {
        x = i * xscale + 20;
        y = yoffset-(data[i]>>8);
        line(xL,yL,x,y);
        xL=x;
        yL=y;
    }
    if (CalcSpectrum) {
        k = 64;
        j = 3;
        while (k<Size) {
            j += 1;
            k *= 4;
        }
        for (i=0;i<Size;i++) fdata[i] = data[i];
        if (k>Size) for (i=Size;i<k;i++) fdata[i] = 0.0;
        xoffset = 50;
        line(xoffset-5,yoffset3,630,yoffset3);
        line(xoffset-5,yoffset3-yfscale*0.33,630,yoffset3-yfscale*0.33);
        line(xoffset-5,yoffset3-yfscale*0.66,630,yoffset3-yfscale*0.66);
        line(xoffset-5,yoffset3-yfscale,630,yoffset3-yfscale);
        line(xoffset,yoffset3,xoffset,yoffset3-yfscale);
        line(630,yoffset3,630,yoffset3-yfscale);
        if (WinType == 0) ; // Do Nothing, Rectangular Window
        else if (WinType == 1) triangle(Size,fdata);
        else if (WinType == 2) hanning(Size,fdata);
        else if (WinType == 3) hamming(Size,fdata);
        else if (WinType == 4) blackman3(Size,fdata);
        else if (WinType == 5) blackman4(Size,fdata);
        fhtRX4(j,fdata);
        logMag(k,fdata,-90.0,32768.0*32768.0*k);
        xscale = 1160.0 / (FreqScale * Size);
        xL = xoffset;
        yL = yoffset3 - (int) (fdata[0] * yfscale);
        for (i=1;i<FreqScale*Size*0.5;i++) {
            x = i * xscale + xoffset;
            y = yoffset3 - (int) (fdata[i] * yfscale);
            line(xL,yL,x,y);
            xL = x;
            yL = y;
        }
        x = (xscale * CursorPos) + xoffset;
        line(x,yoffset3,x,yoffset3-yfscale);
        outtext(0,265,"  0dB");
        outtext(0,315,"-30dB");
        outtext(0,370,"-60dB");
        outtext(0,420,"-90dB");
        sprintf(temp_string,"%i",(int) (2.0 / FreqScale));
        strcpy(out_string,"SR/");
        strcat(out_string,temp_string);
        outtext(600,435,out_string);
//      temp = 0.0;
//      j = 1;
//      while(fdata[j] > fdata[j+1]) j++;
//      for (i=j;i<FreqScale*Size*0.5;i++) {
//      if (fdata[i]>temp) {
//         temp = fdata[i];
//         maxLoc = i;
//      }
//      }
//      temp = maxLoc + fitParabola(fdata[maxLoc-1],fdata[maxLoc],fdata[maxLoc+1]);
//      sprintf(temp_string,"Peak Location = %f ",temp);
//      outtext(0,440,temp_string);
        temp = 90.0 * (fdata[CursorPos] - 1.0);
        temp2 = (float) CursorPos / (float) Size;
        if (FileType==RAW) {
            sprintf(temp_string,"Value = %f dB at Cursor Location %f SRATE ",temp,temp2);
        }
        else {
            temp2 *= Srate;
            sprintf(temp_string,"Value = %f dB at Cursor Location %f ",temp,temp2);
        }
        outtext(0,440,temp_string);
        outtext(500,440,file_name);
        sprintf(temp_string,"Sample Rate: %li",Srate);
        outtext(500,450,temp_string);
        if (WinType == 0) sprintf(temp_string,"Rectangular Window");
        else if (WinType == 1) sprintf(temp_string,"Triangular Window");
        else if (WinType == 2) sprintf(temp_string,"Hanning Window");
        else if (WinType == 3) sprintf(temp_string,"Hamming Window");
        else if (WinType == 4) sprintf(temp_string,"Blackman3 Window");
        else if (WinType == 5) sprintf(temp_string,"Blackman4 Window");
        outtext(500,460,temp_string);
    }
    else {
        outtext(200,340,"SPECTRUM DISPLAY DISABLED");
    }
    outtext(0,460,"Wave Size=+/-  Loc=L/R arrow  Spect. Size=<>  Spect Y/N=S");
    outtext(0,470,"Cursor L/R={ }(fast=[ ])  Window=W  Color=C  Exit: ESC");
}

void Scale(float ScaleBy)
{
    Size *= ScaleBy;
    if (Location+Size > MySize) Location = MySize - Size;
    if (Location<Beginning) Location = Beginning;
//  Redraw();                       // draw new screen
    glutPostRedisplay();
}

void ToggleSpectrum()
{
    if (CalcSpectrum==1)
        CalcSpectrum = 0;
    else
        CalcSpectrum = 1;
//  Redraw();                       // draw new screen
    glutPostRedisplay();
}

void SetLocation(long NewLocation)
{
    Location = NewLocation;
    if (Location+Size > MySize) Location = MySize - Size;
    if (Location<Beginning) Location = Beginning;
//  Redraw();                       // draw new screen
    glutPostRedisplay();
}

long YourLocation()
{
    return Location;
}

void SetSize(int NewSize)
{
    Size = NewSize;
    if (Location+Size > MySize) Location = MySize - Size;
    if (Location<Beginning) Location = Beginning;
//  Redraw();                       // draw new screen
    glutPostRedisplay();
}

void SetFreqScale(float NewScale)
{
    FreqScale = NewScale;
//  Redraw();                       // draw new screen
    glutPostRedisplay();
}

void init(void)
{
}

long size=256;
long loc = 0;

void display(void) {
    glClear( GL_COLOR_BUFFER_BIT );
    Redraw();
    glutSwapBuffers();
}

static float fscale = 1.0;

void SpecialFunc(int key, int x, int y)
{
    if (key==0x1b) {
        exit(0);
    }
    else if (key==GLUT_KEY_RIGHT) {
        loc += (size>>1);
        SetLocation(loc);
        loc = YourLocation();
    }
    else if (key==GLUT_KEY_LEFT) {
        loc -= (size>>1);
        if (loc<0) loc = 0;
        SetLocation(loc);
    }
    else if (key==GLUT_KEY_HOME) {
        loc = 0;
        SetLocation(loc);
        loc = YourLocation();
    }
    else if (key==GLUT_KEY_END) {
        loc = MySize;
        if (loc<0) loc = 0;
        SetLocation(loc);
    }
}

void KeyFunc(unsigned char key, int x, int y)
{
    if (key==0x1b) {
        fclose(MyFile);
        exit(0);
    }
    else if (key=='+') {
        size *= 4;
        if (size>MAX_WIN) size = MAX_WIN;
        else {
            CursorPos *= 4;
            if (CursorPos >= (FreqScale * Size / 2))
                CursorPos = FreqScale * Size / 2;
        }
        SetSize(size);
    }
    else if (key=='-') {
        size /= 4;
        if (size<MIN_WIN) size = MIN_WIN;
        else CursorPos /= 4;
        SetSize(size);
    }
    else if (key=='<') {
        fscale *= 0.5;
        if (fscale*size<1) fscale = 1.0 / size;
        SetFreqScale(fscale);
    }
    else if (key=='>') {
        fscale *= 2.0;
        if (fscale>1.0) fscale=1.0;
        SetFreqScale(fscale);
    }
    else if (key==']') {
        CursorPos += Size / 100;
        if (CursorPos >= (FreqScale * Size / 2))
            CursorPos = FreqScale * Size / 2;
        glutPostRedisplay();
    }
    else if (key=='[') {
        CursorPos -= Size / 100;
        if (CursorPos < 0)
            CursorPos = 0;
        glutPostRedisplay();
    }
    else if (key=='}') {
        CursorPos += 1;
        if (CursorPos >= (FreqScale * Size / 2))
        CursorPos = FreqScale * Size / 2;
        glutPostRedisplay();
    }
    else if (key=='{') {
        CursorPos -= 1;
        if (CursorPos < 0)
            CursorPos = 0;
        glutPostRedisplay();
    }
    else if (key=='S') {
        ToggleSpectrum();
        glutPostRedisplay();
    }
    else if (key=='W') {
        WinType += 1;
        if (WinType > MAX_WIN_TYPE) WinType = 0;
        SetSize(size);
    }
    else if (key=='C') {
        if (clearC[0]==0) {
            clearC[0]=1; clearC[1]=1; clearC[2]=1; clearC[3]=1;
            drawC[0]=0; drawC[1]=0; drawC[2]=0; drawC[3]=0;
        }
        else {
            clearC[0]=0; clearC[1]=0; clearC[2]=0; clearC[3]=0;
            drawC[0]=0; drawC[1]=1; drawC[2]=0; drawC[3]=0;
        }
        glColor4f(drawC[0],drawC[1],drawC[2],drawC[3]);
        glClearColor(clearC[0],clearC[1],clearC[2],clearC[3]);

        glutPostRedisplay();
    }
}

//  #include "waveio.h"

main(int ac, char *av[]    )
{
    int k,bytemode=0, swapmode=0;
// bytemode=1 is 8 bit, indianess=1 is bigendian
    char titleString[512];

//    int graphdriver = DETECT, graphmode;

    long loc = 0;

    if (ac>1) {
        strcpy(file_name,av[1]);
        MyFile = fopen(file_name,"rb");
        if (MyFile) {
            k = 2;
            while(k<ac) {
                if (av[k][0]=='-') {
                    if (av[k][1]=='b')
                        bytemode = 1;
                    else if (av[k][1]=='s')
                        swapmode = 1;
                    else if (av[k][1]=='f')
                        FileType = 3;
                    else
                        printf("I don't understand your arguments\n");
                }
                k += 1;
            }
        }
        else {
            printf("Gimme a valid filename!!!\n");
            exit(0);
        }
    }
    else   {
        printf("Useage:     sndview fileName [-b] [-s]\n");
        printf("            where -b sets byte mode\n");
        printf("		  -f sets float mode\n");
        printf("              and -s forces byteswapped data\n");
        printf("                      (for raw, unknown, etc.)\n");
        exit(0);
    }
    glutInit(&ac, av);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    strcpy(titleString,"SndView: ");
    strcat(titleString,file_name);
    glutInitWindowSize(640,480);
    glutCreateWindow(titleString);

    SndView(0,256,bytemode,swapmode,file_name);
    SetLocation(loc);

    glutDisplayFunc(display);
    glutKeyboardFunc(KeyFunc);
    glutSpecialFunc(SpecialFunc);

    glColor4f(drawC[0],drawC[1],drawC[2],drawC[3]);
    glClearColor(clearC[0],clearC[1],clearC[2],clearC[3]);

    init();

    glutMainLoop();

//    initgraph(&graphdriver, &graphmode, "c:/tc/bgi");
//    setgraphmode(graphmode);
//    closegraph();
    fclose(MyFile);
    return 0;
}
