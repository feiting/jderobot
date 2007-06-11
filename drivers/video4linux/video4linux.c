/*
 *  Copyright (C) 2007 Antonio Pineda Cabello, Jose Maria Ca�as Plaza
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 *  jdec video4linux driver provides video images to color variables from video4linux devices, such as USB or BT878 cameras.
 *
 *  @file video4linux.c
 *  @author Antonio Pineda Cabello <apineda@gsyc.escet.urjc.es> and Jose Maria Ca�as Plaza <jmplaza@gsyc.escet.urjc.es>
 *  @version 4.1
 *  @date 30-05-2007
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "jde.h"
/** Using linux time headers.*/
#define _LINUX_TIME_H 1
/** Using linux device headers.*/
#define _DEVICE_H_ 1
/* before videodev.h. It avoids /usr/include/linux/time.h:9: redefinition of `struct timespec' */
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev.h>

#define CLIP         320
# define RED_NULL    128  /* 137 */
# define BLUE_NULL   128  /* 156 */
# define LUN_MUL     256  /* 360 */
# define RED_MUL     512
# define BLUE_MUL    512
#define GREEN1_MUL  (-RED_MUL/2)
#define GREEN2_MUL  (-BLUE_MUL/6)
#define RED_ADD     (-RED_NULL  * RED_MUL)
#define BLUE_ADD    (-BLUE_NULL * BLUE_MUL)
#define GREEN1_ADD  (-RED_ADD/2)
#define GREEN2_ADD  (-BLUE_ADD/6)


/** Color conversion, taken from xawtv.*/
#define CLIP        320
/** Color conversion, taken from xawtv.*/
# define RED_NULL    128  /* 137 */
/** Color conversion, taken from xawtv.*/
# define BLUE_NULL   128  /* 156 */
/** Color conversion, taken from xawtv.*/
# define LUN_MUL     256  /* 360 */
/** Color conversion, taken from xawtv.*/
#define RED_MUL     512
/** Color conversion, taken from xawtv.*/
#define BLUE_MUL    512
/** Color conversion, taken from xawtv.*/
#define GREEN1_MUL  (-RED_MUL/2)
/** Color conversion, taken from xawtv.*/
#define GREEN2_MUL  (-BLUE_MUL/6)
/** Color conversion, taken from xawtv.*/
#define RED_ADD     (-RED_NULL  * RED_MUL)
/** Color conversion, taken from xawtv.*/
#define BLUE_ADD    (-BLUE_NULL * BLUE_MUL)
/** Color conversion, taken from xawtv.*/
#define GREEN1_ADD  (-RED_ADD/2)
/** Color conversion, taken from xawtv.*/
#define GREEN2_ADD  (-BLUE_ADD/6)

/** Lookup tables for color conversion.*/
static unsigned int  ng_yuv_gray[256];
/** Lookup tables for color conversion.*/
static unsigned int  ng_yuv_red[256];
/** Lookup tables for color conversion.*/
static unsigned int  ng_yuv_blue[256];
/** Lookup tables for color conversion.*/
static unsigned int  ng_yuv_g1[256];
/** Lookup tables for color conversion.*/
static unsigned int  ng_yuv_g2[256];
/** Lookup tables for color conversion.*/
static unsigned int  ng_clip[256 + 2 * CLIP];

/** Macro to transform color to gray.*/
#define GRAY(val)		ng_yuv_gray[val]
/** Macro to transform color to red.*/
#define RED(gray,red)		ng_clip[ CLIP + gray + ng_yuv_red[red] ]
/** Macro to transform color to green.*/
#define GREEN(gray,red,blue)	ng_clip[ CLIP + gray + ng_yuv_g1[red] + ng_yuv_g2[blue] ]
/** Macro to transform color to blue.*/
#define BLUE(gray,blue)		ng_clip[ CLIP + gray + ng_yuv_blue[blue] ]
/** Max number of cameras detected by video4linux driver.*/
#define MAXCAM 4

/** video4linux driver pthread structure for video capture. */
pthread_t v4l_thread[MAXCAM];
/** Map local memory for v4l capture. */
static char *map[MAXCAM];
/** File descriptors for /dev/video cameras.*/
int fdv4l[MAXCAM];
/** Video capability option.*/
struct video_capability cap[MAXCAM];
/** Video channel option.*/
struct video_channel chan[MAXCAM];
/** Video picture option.*/
struct video_picture vpic[MAXCAM];
/** Video buffers. */
static struct video_mbuf gb_buffers[MAXCAM]; 
/** Video window option. */
struct video_window win[MAXCAM];
/** Video mmap option.*/
struct video_mmap gb[MAXCAM];
/** int variable to detect when a image was captured.*/
unsigned long int lastimage;
/** State structure for pthreads.*/
int state[MAXCAM];
/** Mutext for pthreads. */
pthread_mutex_t mymutex[MAXCAM];
/** Condition flags for pthreads. */
pthread_cond_t condition[MAXCAM];
/** Display fps on jdec shell option.*/
int display_fps=0;
/** Driver name.*/
char driver_name[256]="video4linux";
/** Matrix variable to know which devices serve which colors. First indexes 'device', second indexes 'colors using that device'.*/
int device_color[MAXCAM][MAXCAM];
/** Devices detected. If serve_device[0] = 1 then '/dev/video0' was detected in config file.*/
int serve_device[MAXCAM];
/** Device names. Example: "/dev/video0".*/
char v4l_filename[MAXCAM][256];
/** Colors detected in config file. If serve_color[0] = 1, then colorA was detected in config file.*/
int serve_color[MAXCAM];
/** Colors active in each moment. If color_active[0] = 1, then colorA is activated in gui.*/
int color_active[MAXCAM];
/** Which device serves which color.*/
int color_device[MAXCAM];
/** Option to detect when the pthreads must exit execution.*/
int video4linux_close_command=0;
/** colorA schema id.*/
int colorA_schema_id;
/** colorB schema id.*/
int colorB_schema_id;
/** colorC schema id.*/
int colorC_schema_id;
/** colorD schema id.*/
int colorD_schema_id;

/** Function will activate displaying fps in jdec shell.*/
void video4linux_display_fps(){
  display_fps=1;
}

/** Function will end execution of the driver, closing file descriptors and stopping devices.*/
void video4linux_close(){

  int i;
  for(i=0;i<MAXCAM;i++){if(serve_device[i]) close(fdv4l[i]);}
  printf("driver video4linux off\n");
}

/** colorA resume function following jdec platform API schemas.
 *  @param father Father id for this schema.
 *  @param brothers Brothers for this schema.
 *  @param arbitration function for this schema.
 *  @return integer resuming result.*/
int mycolorA_resume(int father, int *brothers, arbitration fn){
  if((serve_color[0]==1)&&(color_active[0]==0)){
    color_active[0]=1;

    /* if any other color is not using the same thread we activate that thread*/
    if((color_active[1])&&(color_device[0]==color_device[1])) return 0;
    if((color_active[2])&&(color_device[0]==color_device[2])) return 0;
    if((color_active[3])&&(color_device[0]==color_device[3])) return 0;

    printf("colorA schema resume (video4linux driver)\n");
    all[colorA_schema_id].father = father;
    all[colorA_schema_id].fps = 0.;
    all[colorA_schema_id].k =0;
    put_state(colorA_schema_id,winner);

    pthread_mutex_lock(&mymutex[color_device[0]]);
    state[color_device[0]]=winner;
    pthread_cond_signal(&condition[color_device[0]]);
    pthread_mutex_unlock(&mymutex[color_device[0]]);
  }
  return 0;
}

/** colorA suspend function following jdec platform API schemas.
 *  @return integer suspending result.*/
int mycolorA_suspend(){
      
  if((serve_color[0]==1)&&(color_active[0])){
    color_active[0]=0;
    printf("colorA schema suspend (video4linux driver)\n");

    /* if any other color is using that thread it goes to sleep */
    if((color_active[1])&&(color_device[0]==color_device[1])) return 0;
    if((color_active[2])&&(color_device[0]==color_device[2])) return 0;
    if((color_active[3])&&(color_device[0]==color_device[3])) return 0;

    put_state(colorA_schema_id,slept);
    pthread_mutex_lock(&mymutex[color_device[0]]);
    state[color_device[0]]=slept;
    pthread_mutex_unlock(&mymutex[color_device[0]]);
  }
  return 0;
}


/** colorB resume function following jdec platform API schemas.
 *  @param father Father id for this schema.
 *  @param brothers Brothers for this schema.
 *  @param arbitration function for this schema.
 *  @return integer resuming result.*/
int mycolorB_resume(int father, int *brothers, arbitration fn){
  if((serve_color[1]==1)&&(color_active[1]==0)){
    color_active[1]=1;

    /* if any other color is not using the same thread we activate that thread*/
    if((color_active[0])&&(color_device[1]==color_device[0])) return 0;
    if((color_active[2])&&(color_device[1]==color_device[2])) return 0;
    if((color_active[3])&&(color_device[1]==color_device[3])) return 0;

    printf("colorB schema resume (video4linux driver)\n");
    all[colorB_schema_id].father = father;
    all[colorB_schema_id].fps = 0.;
    all[colorB_schema_id].k =0;
    put_state(colorB_schema_id,winner);

    pthread_mutex_lock(&mymutex[color_device[1]]);
    state[color_device[1]]=winner;
    pthread_cond_signal(&condition[color_device[1]]);
    pthread_mutex_unlock(&mymutex[color_device[1]]);
  }
  return 0;
}

/** colorB suspend function following jdec platform API schemas.
 *  @return integer suspending result.*/
int mycolorB_suspend(){
      
  if((serve_color[1]==1)&&(color_active[1])){
    color_active[1]=0;
    printf("colorB schema suspend (video4linux driver)\n");

    /* if any other color is using that thread it goes to sleep */
    if((color_active[0])&&(color_device[1]==color_device[0])) return 0;
    if((color_active[2])&&(color_device[1]==color_device[2])) return 0;
    if((color_active[3])&&(color_device[1]==color_device[3])) return 0;

    put_state(colorB_schema_id,slept);
    pthread_mutex_lock(&mymutex[color_device[1]]);
    state[color_device[1]]=slept;
    pthread_mutex_unlock(&mymutex[color_device[1]]);
  }
  return 0;
}

/** colorC resume function following jdec platform API schemas.
 *  @param father Father id for this schema.
 *  @param brothers Brothers for this schema.
 *  @param arbitration function for this schema.
 *  @return integer resuming result.*/
int mycolorC_resume(int father, int *brothers, arbitration fn){
  if((serve_color[2]==1)&&(color_active[2]==0)){
    color_active[2]=1;

    /* if any other color is not using the same thread we activate that thread*/
    if((color_active[0])&&(color_device[2]==color_device[0])) return 0;
    if((color_active[1])&&(color_device[2]==color_device[1])) return 0;
    if((color_active[3])&&(color_device[2]==color_device[3])) return 0;

    printf("colorC schema resume (video4linux driver)\n");
    all[colorC_schema_id].father = father;
    all[colorC_schema_id].fps = 0.;
    all[colorC_schema_id].k =0;
    put_state(colorC_schema_id,winner);

    pthread_mutex_lock(&mymutex[color_device[2]]);
    state[color_device[2]]=winner;
    pthread_cond_signal(&condition[color_device[2]]);
    pthread_mutex_unlock(&mymutex[color_device[2]]);
  }
  return 0;
}

/** colorC suspend function following jdec platform API schemas.
 *  @return integer suspending result.*/
int mycolorC_suspend(){
      
  if((serve_color[2]==1)&&(color_active[2])){
    color_active[2]=0;
    printf("colorC schema suspend (video4linux driver)\n");

    /* if any other color is using that thread it goes to sleep */
    if((color_active[0])&&(color_device[2]==color_device[0])) return 0;
    if((color_active[1])&&(color_device[2]==color_device[1])) return 0;
    if((color_active[3])&&(color_device[2]==color_device[3])) return 0;

    put_state(colorC_schema_id,slept);
    pthread_mutex_lock(&mymutex[color_device[2]]);
    state[color_device[2]]=slept;
    pthread_mutex_unlock(&mymutex[color_device[2]]);
  }
  return 0;
}

/** colorD resume function following jdec platform API schemas.
 *  @param father Father id for this schema.
 *  @param brothers Brothers for this schema.
 *  @param arbitration function for this schema.
 *  @return integer resuming result.*/
int mycolorD_resume(int father, int *brothers, arbitration fn){
  if((serve_color[3]==1)&&(color_active[3]==0)){
    color_active[3]=1;

    /* if any other color is not using the same thread we activate that thread*/
    if((color_active[0])&&(color_device[3]==color_device[0])) return 0;
    if((color_active[1])&&(color_device[3]==color_device[1])) return 0;
    if((color_active[2])&&(color_device[3]==color_device[2])) return 0;

    printf("colorD schema resume (video4linux driver)\n");
    all[colorD_schema_id].father = father;
    all[colorD_schema_id].fps = 0.;
    all[colorD_schema_id].k =0;
    put_state(colorD_schema_id,winner);

    pthread_mutex_lock(&mymutex[color_device[3]]);
    state[color_device[3]]=winner;
    pthread_cond_signal(&condition[color_device[3]]);
    pthread_mutex_unlock(&mymutex[color_device[3]]);
  }
  return 0;
}

/** colorD suspend function following jdec platform API schemas.
 *  @return integer suspending result.*/
int mycolorD_suspend(){
      
  if((serve_color[3]==1)&&(color_active[3])){
    color_active[3]=0;
    printf("colorD schema suspend (video4linux driver)\n");

    /* if any other color is using that thread it goes to sleep */
    if((color_active[0])&&(color_device[3]==color_device[0])) return 0;
    if((color_active[1])&&(color_device[3]==color_device[1])) return 0;
    if((color_active[2])&&(color_device[3]==color_device[2])) return 0;

    put_state(colorD_schema_id,slept);
    pthread_mutex_lock(&mymutex[color_device[3]]);
    state[color_device[3]]=slept;
    pthread_mutex_unlock(&mymutex[color_device[3]]);
  }
  return 0;
}

/** video4linux driver pthread function for device /dev/video0.*/
void *video4linux1_thread(){
  int cam=0;
  int f,c,yy,j,n;
  unsigned char y,u,v;
  unsigned char *src,*dest;
  static float fps=0, fpsold=0;
  struct timeval t;
  static unsigned long int then;
  unsigned long int now;

  printf("video4linux: video4linux1 thread started\n");

  do{

    pthread_mutex_lock(&mymutex[0]);

    if (state[0]==slept){
      printf("video4linux: video4linux1 thread goes sleep mode\n");
      pthread_cond_wait(&condition[0],&mymutex[0]);
      printf("video4linux: video4linux1 thread woke up\n");
      pthread_mutex_unlock(&mymutex[0]);
    }else{
      
      pthread_mutex_unlock(&mymutex[0]);

      /* if displaying fps needed */
      if(display_fps==1){
	gettimeofday(&t,NULL);
	now=t.tv_sec*1000000+t.tv_usec;
	lastimage = now;
	if ((now-then)>2000000) /* periodo de integracion, de cuenta de imagenes: 2 segundos, mas o menos */
	  {fps=fps*1000000/(now-then); 
	    if ((fps>fpsold+1)||(fps<fpsold-1)) 
	      {printf("video 0: %.1f fps\n",fps);
		fpsold=fps;}
	    fps=0.;
	    then=now;
	  }
	else fps++;
      }

      /* video4linux image capture */
      src = map[cam]+gb_buffers[cam].offsets[gb[cam].frame];

      for(n=0;n<MAXCAM;n++){

	if((device_color[0][n])&&(color_active[n])){

	  if(n==0){dest=(unsigned char*)colorA; speedcounter(colorA_schema_id);  /*for(j=0;j<imageA_users;j++) if (imageA_callbacks[j]!=NULL) imageA_callbacks[j]();*/}
	  else if(n==1){ dest=(unsigned char*)colorB; speedcounter(colorB_schema_id); /*for(j=0;j<imageB_users;j++) if (imageB_callbacks[j]!=NULL) imageB_callbacks[j]();*/}
	  else if(n==2){ dest=(unsigned char*)colorC; speedcounter(colorC_schema_id); /*for(j=0;j<imageC_users;j++) if (imageC_callbacks[j]!=NULL) imageC_callbacks[j]();*/}
	  else if(n==3){ dest=(unsigned char*)colorD; speedcounter(colorD_schema_id); /*for(j=0;j<imageD_users;j++) if (imageD_callbacks[j]!=NULL) imageD_callbacks[j]();*/}

	  if (vpic[cam].palette==VIDEO_PALETTE_YUV420P){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      f=j/SIFNTSC_COLUMNS; c=j%SIFNTSC_COLUMNS;
	      y=src[j];
	      u=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      v=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS*5/4+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      yy = GRAY(y);
	      dest[3*j]=BLUE(yy,u);
	      dest[3*j+1]=GREEN(yy,v,u);
	      dest[3*j+2]=RED(yy,v);	
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB565){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      dest[3*j]=  (0x1F&src[2*j])<<3; /* blue */
	      dest[3*j+1]= ((0x07&src[2*j+1])<<5) | ((0xE0&src[2*j])>>3); /* green */
	      dest[3*j+2]= (0xF8&(src[2*j+1]));  /* red */
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB24){
	    memcpy(dest,src,SIFNTSC_COLUMNS*SIFNTSC_ROWS*3);
	  }
	}

	/* asks for next capture */
	gb[cam].frame=(gb[cam].frame+1)%2;
	if (-1 == ioctl(fdv4l[cam],VIDIOCMCAPTURE,&gb[cam])) {perror("VIDIOCMCAPTURE");}
      
	/* "libera" ese frame para que el driver pueda cargar ahi nueva imagen.*/
	if (-1 == ioctl(fdv4l[cam],VIDIOCSYNC,&gb[cam].frame)) {
	  perror("VIDIOCSYNC");
	}

	/*usleep(300000);*/
      }
    }
  }while(video4linux_close_command==0);
  pthread_exit(0);
}

/** video4linux driver pthread function for device /dev/video1.*/
void *video4linux2_thread(){
  int cam=0;
  int f,c,yy,j,n;
  unsigned char y,u,v;
  unsigned char *src,*dest;
  static float fps=0, fpsold=0;
  struct timeval t;
  static unsigned long int then;
  unsigned long int now;

  printf("video4linux: video4linux2 thread started\n");

  do{

    pthread_mutex_lock(&mymutex[1]);

    if (state[1]==slept){
      printf("video4linux: video4linux2 thread goes sleep mode\n");
      pthread_cond_wait(&condition[1],&mymutex[1]);
      printf("video4linux: video4linux2 thread woke up\n");
      pthread_mutex_unlock(&mymutex[1]);    
    }else{
      
      pthread_mutex_unlock(&mymutex[1]);

      /* if displaying fps needed */
      if(display_fps==1){
	gettimeofday(&t,NULL);
	now=t.tv_sec*1000000+t.tv_usec;
	lastimage = now;
	if ((now-then)>2000000) /* periodo de integracion, de cuenta de imagenes: 2 segundos, mas o menos */
	  {fps=fps*1000000/(now-then); 
	    if ((fps>fpsold+1)||(fps<fpsold-1)) 
	      {printf("video 1: %.1f fps\n",fps);
		fpsold=fps;}
	    fps=0.;
	    then=now;
	  }
	else fps++;
      }

      /* video4linux image capture */
      src = map[cam]+gb_buffers[cam].offsets[gb[cam].frame];

      for(n=0;n<MAXCAM;n++){

	if((device_color[1][n])&&(color_active[n])){

	  if(n==0){dest=(unsigned char*)colorA; speedcounter(colorA_schema_id);  /*for(j=0;j<imageA_users;j++) if (imageA_callbacks[j]!=NULL) imageA_callbacks[j]();*/}
	  else if(n==1){ dest=(unsigned char*)colorB; speedcounter(colorB_schema_id); /*for(j=0;j<imageB_users;j++) if (imageB_callbacks[j]!=NULL) imageB_callbacks[j]();*/}
	  else if(n==2){ dest=(unsigned char*)colorC; speedcounter(colorC_schema_id); /*for(j=0;j<imageC_users;j++) if (imageC_callbacks[j]!=NULL) imageC_callbacks[j]();*/}
	  else if(n==3){ dest=(unsigned char*)colorD; speedcounter(colorD_schema_id); /*for(j=0;j<imageD_users;j++) if (imageD_callbacks[j]!=NULL) imageD_callbacks[j]();*/}

	  if (vpic[cam].palette==VIDEO_PALETTE_YUV420P){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      f=j/SIFNTSC_COLUMNS; c=j%SIFNTSC_COLUMNS;
	      y=src[j];
	      u=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      v=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS*5/4+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      yy = GRAY(y);
	      dest[3*j]=BLUE(yy,u);
	      dest[3*j+1]=GREEN(yy,v,u);
	      dest[3*j+2]=RED(yy,v);	
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB565){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      dest[3*j]=  (0x1F&src[2*j])<<3; /* blue */
	      dest[3*j+1]= ((0x07&src[2*j+1])<<5) | ((0xE0&src[2*j])>>3); /* green */
	      dest[3*j+2]= (0xF8&(src[2*j+1]));  /* red */
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB24){
	    memcpy(dest,src,SIFNTSC_COLUMNS*SIFNTSC_ROWS*3);
	  }
	}

	/* asks for next capture */
	gb[cam].frame=(gb[cam].frame+1)%2;
	if (-1 == ioctl(fdv4l[cam],VIDIOCMCAPTURE,&gb[cam])) {perror("VIDIOCMCAPTURE");}
      
	/* "libera" ese frame para que el driver pueda cargar ahi nueva imagen.*/
	if (-1 == ioctl(fdv4l[cam],VIDIOCSYNC,&gb[cam].frame)) {
	  perror("VIDIOCSYNC");
	}

	/*usleep(300000);*/
      }
    }
  }while(video4linux_close_command==0);
  pthread_exit(0);
}
/** video4linux driver pthread function for device /dev/video2.*/
void *video4linux3_thread(){
  int cam=0;
  int f,c,yy,j,n;
  unsigned char y,u,v;
  unsigned char *src,*dest;
  static float fps=0, fpsold=0;
  struct timeval t;
  static unsigned long int then;
  unsigned long int now;

  printf("video4linux: video4linux3 thread started\n");

  do{

    pthread_mutex_lock(&mymutex[2]);

    if (state[2]==slept){
      printf("video4linux: video4linux3 thread goes sleep mode\n");
      pthread_cond_wait(&condition[2],&mymutex[2]);
      printf("video4linux: video4linux3 thread woke up\n");
      pthread_mutex_unlock(&mymutex[2]);
    }else{
      
      pthread_mutex_unlock(&mymutex[2]);

      /* if displaying fps needed */
      if(display_fps==1){
	gettimeofday(&t,NULL);
	now=t.tv_sec*1000000+t.tv_usec;
	lastimage = now;
	if ((now-then)>2000000) /* periodo de integracion, de cuenta de imagenes: 2 segundos, mas o menos */
	  {fps=fps*1000000/(now-then); 
	    if ((fps>fpsold+1)||(fps<fpsold-1)) 
	      {printf("video 1: %.1f fps\n",fps);
		fpsold=fps;}
	    fps=0.;
	    then=now;
	  }
	else fps++;
      }

      /* video4linux image capture */
      src = map[cam]+gb_buffers[cam].offsets[gb[cam].frame];

      for(n=0;n<MAXCAM;n++){

	if((device_color[2][n])&&(color_active[n])){

	  if(n==0){dest=(unsigned char*)colorA; speedcounter(colorA_schema_id);  /*for(j=0;j<imageA_users;j++) if (imageA_callbacks[j]!=NULL) imageA_callbacks[j]();*/}
	  else if(n==1){ dest=(unsigned char*)colorB; speedcounter(colorB_schema_id); /*for(j=0;j<imageB_users;j++) if (imageB_callbacks[j]!=NULL) imageB_callbacks[j]();*/}
	  else if(n==2){ dest=(unsigned char*)colorC; speedcounter(colorC_schema_id); /*for(j=0;j<imageC_users;j++) if (imageC_callbacks[j]!=NULL) imageC_callbacks[j]();*/}
	  else if(n==3){ dest=(unsigned char*)colorD; speedcounter(colorD_schema_id); /*for(j=0;j<imageD_users;j++) if (imageD_callbacks[j]!=NULL) imageD_callbacks[j]();*/}

	  if (vpic[cam].palette==VIDEO_PALETTE_YUV420P){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      f=j/SIFNTSC_COLUMNS; c=j%SIFNTSC_COLUMNS;
	      y=src[j];
	      u=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      v=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS*5/4+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      yy = GRAY(y);
	      dest[3*j]=BLUE(yy,u);
	      dest[3*j+1]=GREEN(yy,v,u);
	      dest[3*j+2]=RED(yy,v);	
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB565){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      dest[3*j]=  (0x1F&src[2*j])<<3; /* blue */
	      dest[3*j+1]= ((0x07&src[2*j+1])<<5) | ((0xE0&src[2*j])>>3); /* green */
	      dest[3*j+2]= (0xF8&(src[2*j+1]));  /* red */
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB24){
	    memcpy(dest,src,SIFNTSC_COLUMNS*SIFNTSC_ROWS*3);
	  }
	}

	/* asks for next capture */
	gb[cam].frame=(gb[cam].frame+1)%2;
	if (-1 == ioctl(fdv4l[cam],VIDIOCMCAPTURE,&gb[cam])) {perror("VIDIOCMCAPTURE");}
      
	/* "libera" ese frame para que el driver pueda cargar ahi nueva imagen.*/
	if (-1 == ioctl(fdv4l[cam],VIDIOCSYNC,&gb[cam].frame)) {
	  perror("VIDIOCSYNC");
	}

	/*usleep(300000);*/
      }
    }
  }while(video4linux_close_command==0);
  pthread_exit(0);
}

/** video4linux driver pthread function for device /dev/video3.*/
void *video4linux4_thread(void *not_used){
  int cam=0;
  int f,c,yy,j,n;
  unsigned char y,u,v;
  unsigned char *src,*dest;
  static float fps=0, fpsold=0;
  struct timeval t;
  static unsigned long int then;
  unsigned long int now;

  printf("video4linux: video4linux4 thread started\n");

  do{

    pthread_mutex_lock(&mymutex[3]);

    if (state[3]==slept){
      printf("video4linux: video4linux4 thread goes sleep mode\n");
      pthread_cond_wait(&condition[3],&mymutex[3]);
      printf("video4linux: video4linux4 thread woke up\n");
      pthread_mutex_unlock(&mymutex[3]);      
    }else{
      
      pthread_mutex_unlock(&mymutex[3]);

      /* if displaying fps needed */
      if(display_fps==1){
	gettimeofday(&t,NULL);
	now=t.tv_sec*1000000+t.tv_usec;
	lastimage = now;
	if ((now-then)>2000000) /* periodo de integracion, de cuenta de imagenes: 2 segundos, mas o menos */
	  {fps=fps*1000000/(now-then); 
	    if ((fps>fpsold+1)||(fps<fpsold-1)) 
	      {printf("video 1: %.1f fps\n",fps);
		fpsold=fps;}
	    fps=0.;
	    then=now;
	  }
	else fps++;
      }

      /* video4linux image capture */
      src = map[cam]+gb_buffers[cam].offsets[gb[cam].frame];

      for(n=0;n<MAXCAM;n++){

	if((device_color[3][n])&&(color_active[n])){

	  if(n==0){dest=(unsigned char*)colorA; speedcounter(colorA_schema_id);  /*for(j=0;j<imageA_users;j++) if (imageA_callbacks[j]!=NULL) imageA_callbacks[j]();*/}
	  else if(n==1){ dest=(unsigned char*)colorB; speedcounter(colorB_schema_id); /*for(j=0;j<imageB_users;j++) if (imageB_callbacks[j]!=NULL) imageB_callbacks[j]();*/}
	  else if(n==2){ dest=(unsigned char*)colorC; speedcounter(colorC_schema_id); /*for(j=0;j<imageC_users;j++) if (imageC_callbacks[j]!=NULL) imageC_callbacks[j]();*/}
	  else if(n==3){ dest=(unsigned char*)colorD; speedcounter(colorD_schema_id); /*for(j=0;j<imageD_users;j++) if (imageD_callbacks[j]!=NULL) imageD_callbacks[j]();*/}
	 
	  if (vpic[cam].palette==VIDEO_PALETTE_YUV420P){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      f=j/SIFNTSC_COLUMNS; c=j%SIFNTSC_COLUMNS;
	      y=src[j];
	      u=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      v=src[SIFNTSC_COLUMNS*SIFNTSC_ROWS*5/4+(f/2)*SIFNTSC_COLUMNS/2+c/2];
	      yy = GRAY(y);
	      dest[3*j]=BLUE(yy,u);
	      dest[3*j+1]=GREEN(yy,v,u);
	      dest[3*j+2]=RED(yy,v);	
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB565){
	    for (j = 0; j < SIFNTSC_COLUMNS*SIFNTSC_ROWS; j++) {
	      dest[3*j]=  (0x1F&src[2*j])<<3; /* blue */
	      dest[3*j+1]= ((0x07&src[2*j+1])<<5) | ((0xE0&src[2*j])>>3); /* green */
	      dest[3*j+2]= (0xF8&(src[2*j+1]));  /* red */
	    }
	
	  }else if (vpic[cam].palette==VIDEO_PALETTE_RGB24){
	    memcpy(dest,src,SIFNTSC_COLUMNS*SIFNTSC_ROWS*3);
	  }
	}

	/* asks for next capture */
	gb[cam].frame=(gb[cam].frame+1)%2;
	if (-1 == ioctl(fdv4l[cam],VIDIOCMCAPTURE,&gb[cam])) {perror("VIDIOCMCAPTURE");}
      
	/* "libera" ese frame para que el driver pueda cargar ahi nueva imagen.*/
	if (-1 == ioctl(fdv4l[cam],VIDIOCSYNC,&gb[cam].frame)) {
	  perror("VIDIOCSYNC");
	}

	/*usleep(300000);*/
      }
    }
  }while(video4linux_close_command==0);
  pthread_exit(0);
}

/** video4linux driver parse configuration file function.
 *  @param configfile path and name to the config file.
 *  @return 0 if parsing was successful or -1 if something went wrong.*/
int video4linux_parseconf(char *configfile){

  int end_parse=0; int end_section=0; int driver_config_parsed=0;
  FILE *myfile;
  const int limit = 256;

  if ((myfile=fopen(configfile,"r"))==NULL){
    printf("video4linux: cannot find config file\n");
    return -1;
  }

  do{
    
    char word[256],word2[256],buffer_file[256];
    int i=0; int j=0;

    buffer_file[0]=fgetc(myfile);
    
    /* end of file */
    if (feof(myfile)){
      end_section=1;
      end_parse=1;
      
      /* line comment */
    }else if (buffer_file[0]=='#') {
      while(fgetc(myfile)!='\n');
      
      /* white spaces */
    }else if (buffer_file[0]==' ') {
      while(buffer_file[0]==' ') buffer_file[0]=fgetc(myfile);

      /* tabs */
    }else if(buffer_file[0]=='\t') {
      while(buffer_file[0]=='\t') buffer_file[0]=fgetc(myfile);
      /* storing line in buffer */
    }else{
      
      while(buffer_file[i]!='\n') buffer_file[++i]=fgetc(myfile);
      buffer_file[++i]='\0';

      if (i >= limit-1) { 
	printf("%s...\n", buffer_file); 
	printf ("video4linux: line too long in config file!\n"); 
	exit(-1);
      }
      
      /* first word of the line */     
      if (sscanf(buffer_file,"%s",word)==1){
	if (strcmp(word,"driver")==0) {
	  while((buffer_file[j]!='\n')&&(buffer_file[j]!=' ')&&(buffer_file[j]!='\0')&&(buffer_file[j]!='\t')) j++;
	  sscanf(&buffer_file[j],"%s",word2);
	  
	  /* checking if this section matchs our driver name */
	  if (strcmp(word2,driver_name)==0){
	    /* the sections match */
	    do{
	      
	      char buffer_file2[256],word3[256],word4[256],word5[256];
	      int k=0; int z=0;

	      buffer_file2[0]=fgetc(myfile);
	      
	      /* end of file */
	      if (feof(myfile)){
		end_section=1;
		end_parse=1;
		
		/* line comment */
	      }else if (buffer_file2[0]=='#') {
		while(fgetc(myfile)!='\n');
	      
		/* white spaces */
	      }else if (buffer_file2[0]==' ') {
		while(buffer_file2[0]==' ') buffer_file2[0]=fgetc(myfile);

		/* tabs */
	      }else if(buffer_file2[0]=='\t') {
		while(buffer_file2[0]=='\t') buffer_file2[0]=fgetc(myfile);

		/* storing line in buffer */
	      }else{
		
		while(buffer_file2[k]!='\n') buffer_file2[++k]=fgetc(myfile);
		buffer_file2[++k]='\0';
		
		/* first word of the line */
		if (sscanf(buffer_file2,"%s",word3)==1){
		  if (strcmp(word3,"end_driver")==0) {
		    while((buffer_file2[z]!='\n')&&(buffer_file2[z]!=' ')&&(buffer_file2[z]!='\0')&&(buffer_file2[z]!='\t')) z++;
		    driver_config_parsed=1;
		    end_section=1;
		    end_parse=1;
		    
		  }else if (strcmp(word3,"driver")==0) {
		    while((buffer_file2[z]!='\n')&&(buffer_file2[z]!=' ')&&(buffer_file2[z]!='\0')&&(buffer_file2[z]!='\t')) z++;
		    printf("video4linux: error in config file.\n'end_section' keyword required before starting new driver section.\n");
		    end_section=1; end_parse=1;

		  }else if(strcmp(word3,"provides")==0){
		    while((buffer_file2[z]!='\n')&&(buffer_file2[z]!=' ')&&(buffer_file2[z]!='\0')&&(buffer_file2[z]!='\t')) z++;
		    if(sscanf(buffer_file2,"%s %s %s",word3,word4,word5)>2){
		      
		      if(strcmp(word5,"/dev/video0")==0){
			serve_device[0]=1;
			if((strcmp(word4,"colorA")==0)&&(serve_color[0]==0)){device_color[0][0]=1; serve_color[0]=1; color_device[0]=0;}
			else if((strcmp(word4,"colorB")==0)&&(serve_color[1]==0)){device_color[0][1]=1; serve_color[1]=1;color_device[1]=0;}
			else if((strcmp(word4,"colorC")==0)&&(serve_color[2]==0)){device_color[0][2]=1; serve_color[2]=1;color_device[2]=0;}
			else if((strcmp(word4,"colorD")==0)&&(serve_color[3]==0)){device_color[0][3]=1; serve_color[3]=1;color_device[3]=0;}
			
		      }else if(strcmp(word5,"/dev/video1")==0){
			serve_device[1]=1;
			if((strcmp(word4,"colorA")==0)&&(serve_color[0]==0)){device_color[1][0]=1; serve_color[0]=1; color_device[0]=1;}
			else if((strcmp(word4,"colorB")==0)&&(serve_color[1]==0)){device_color[1][1]=1; serve_color[1]=1; color_device[1]=1;}
			else if((strcmp(word4,"colorC")==0)&&(serve_color[2]==0)){device_color[1][2]=1; serve_color[2]=1; color_device[2]=1;}
			else if((strcmp(word4,"colorD")==0)&&(serve_color[3]==0)){device_color[1][3]=1; serve_color[3]=1; color_device[3]=1;}

		      }else if(strcmp(word5,"/dev/video2")==0){
			serve_device[2]=1;
			if((strcmp(word4,"colorA")==0)&&(serve_color[0]==0)){device_color[2][0]=1; serve_color[0]=1; color_device[0]=2;}
			else if((strcmp(word4,"colorB")==0)&&(serve_color[1]==0)){device_color[2][1]=1; serve_color[1]=1; color_device[1]=2;}
			else if((strcmp(word4,"colorC")==0)&&(serve_color[2]==0)){device_color[2][2]=1; serve_color[2]=1; color_device[2]=2;}
			else if((strcmp(word4,"colorD")==0)&&(serve_color[3]==0)){device_color[2][3]=1; serve_color[3]=1; color_device[3]=2;}

		      }else if(strcmp(word5,"/dev/video3")==0){
			serve_device[3]=1;
			if((strcmp(word4,"colorA")==0)&&(serve_color[0]==0)){device_color[3][0]=1; serve_color[0]=1; color_device[0]=3;}
			else if((strcmp(word4,"colorB")==0)&&(serve_color[1]==0)){device_color[3][1]=1; serve_color[1]=1; color_device[1]=3;}
			else if((strcmp(word4,"colorC")==0)&&(serve_color[2]==0)){device_color[3][2]=1; serve_color[2]=1; color_device[2]=3;}
			else if((strcmp(word4,"colorD")==0)&&(serve_color[3]==0)){device_color[3][3]=1; serve_color[3]=1; color_device[3]=3;}
		      }

		    }else{
		      printf("video4linux: provides line incorrect\n");
		    }
		  }else printf("video4linux: i don't know what to do with '%s'\n",buffer_file2);
		}
	      }
	    }while(end_section==0);
	    end_section=0; 
	  }
	}
      }
    }
  }while(end_parse==0);
  
  /* checking if a driver section was read */
  if(driver_config_parsed==1){
    if((serve_color[0]==0)&&(serve_color[1]==0)&&(serve_color[2]==0)&&(serve_color[3]==0)){
      printf("video4linux: warning! no color provided.\n");
    }
    return 0;
  }else return -1;
}

/** video4linux driver init function. It will start all video4linux required devices and setting them the default configuration.
 *  @return 0 if initialitation was successful or -1 if something went wrong.*/
int video4linux_init(){

  int i,k,n;

  /* recorremos todos los posibles dispositivos */
  for(n=0;n<MAXCAM;n++){

    /* inicializamos la apertura de todos los dispositivos */
    if(serve_device[n]){

      /* Apertura e inicializacion del dispositivo de videoforlinux */
      fdv4l[n] = open(v4l_filename[n], O_RDWR);

      if (fdv4l[n] < 0) {perror(v4l_filename[n]); return -1;}
      if (ioctl(fdv4l[n], VIDIOCGCAP, &cap[n]) < 0) {perror("VIDIOGCAP"); return -1;}
      else {
	printf("CAPABILITY: \n name=%s \n type=%d \n channels=%d \n audios=%d \n maxwidth=%d \n maxheight=%d \n minwidth=%d \n minheight=%d \n\n", cap[n].name, cap[n].type, cap[n].channels, cap[n].audios, cap[n].maxwidth, cap[n].maxheight, cap[n].minwidth, cap[n].minheight);
	printf("%s detected\n",cap[n].name);
      }
	
      
      /*  fcntl(fdv4l[n],F_SETFD,FD_CLOEXEC); */
      for(i=0; i<cap[n].channels; i++){
	chan[n].channel=(int) i;
	if (ioctl(fdv4l[n], VIDIOCGCHAN, &chan[n]) == -1) {
	  perror("VIDIOCGCHAN");
	  close(fdv4l[n]);
	  return -1;
	}else printf(" CHANNEL %d: \n name=%s \n tuners=%d \n type=%d \n\n",chan[n].channel,chan[n].name,chan[n].tuners,chan[n].type);
      }

      /*image size*/
      win[n].width=SIFNTSC_COLUMNS;
      win[n].height=SIFNTSC_ROWS;
      win[n].x=0;
      win[n].y=0; 
      win[n].chromakey=0;
      win[n].clipcount=0;
  
      if ((strcmp(cap[n].name,"BT878(Hauppauge new)")==0) && (cap[n].channels == 3)){
	/* Selecciona el canal composite-video (=1) como canal activo. Si pongo canal Television (=0) tambien funciona. Si pongo s-video (=2) se ve, pero en niveles de gris nada mas */
	chan[n].channel=1;
	if (ioctl(fdv4l[n], VIDIOCSCHAN, &chan[n]) == -1) { perror("VIDIOCSCHAN"); return(-1);}

      }else if((strcmp(cap[n].name,"Philips 740 webcam")==0) || (strcmp(cap[n].name,"Philips 730 webcam")==0)){
#ifdef PWCX
	/* the driver with compression (pwcx) allows higher frame rates */
	win[n].flags = 30 << 16; 
#else
	/* Philips 740 webcam need to configure framerate at 5 to deliver 320x240 images. For higher speeds (including default ones) the maximum image size is 160x120. This doesn't affect to other v4l devices, as long as they don't use bits 16..22 of flags field */
	/* withour pwcx, requiring 30 fps causes small image sizes */
	win[n].flags = 5 << 16;
#endif 
	
      }else win[n].flags =0;
	  
      if (ioctl(fdv4l[n], VIDIOCSWIN, &win[n]) < 0) perror("VIDIOCSWIN");
      if (ioctl(fdv4l[n], VIDIOCGWIN, &win[n]) < 0) {perror("VIDIOCGWIN");return -1;}
      else{

	printf("WINDOW \n x=%d y=%d \n width=%d, height=%d \n chromakey=%d \n flags=%d \n clipcount=%d \n\n", win[n].x, win[n].y, win[n].width, win[n].height, win[n].chromakey, win[n].flags, win[n].clipcount);

      }

      printf("video4linux: capture window = %d x %d \n",win[n].width,win[n].height);
	  
	  
      /* Caracteristicas de la imagen, incluyendo formato de pixels!!. Se solicitan al driver y este nos dice que si puede darnoslo o no */
      if (ioctl(fdv4l[n], VIDIOCGPICT, &vpic[n]) < 0) {perror("VIDIOCGPICT"); return -1;}
	  
      /*
	vpic[n].depth=8;
	vpic[n].palette=VIDEO_PALETTE_YUV420P;
	if(ioctl(fdv4l[n], VIDIOCSPICT, &vpic[n]) < 0) 
	{perror("v4l: unable to find a supported capture format"); return -1;}
      */
	  
      vpic[n].depth=8;
      vpic[n].palette=VIDEO_PALETTE_YUV420P;
      if(ioctl(fdv4l[n], VIDIOCSPICT, &vpic[n])<0) {
	vpic[n].depth=16;
	vpic[n].palette=VIDEO_PALETTE_RGB565;
	if(ioctl(fdv4l[n], VIDIOCSPICT, &vpic[n]) < 0) {
	  vpic[n].depth=24;
	  vpic[n].palette=VIDEO_PALETTE_RGB24;
	  if(ioctl(fdv4l[n], VIDIOCSPICT, &vpic[n]) < 0) {perror("v4l: unable to find a supported capture format"); return -1;}
	}
      }
	  
      printf("VIDEO_PICTURE \n brightness=%d \n hue=%d \n colour=%d \n contrast=%d \n whiteness=%d \n depth=%d \n palette=%d \n\n",vpic[n].brightness,vpic[n].hue,vpic[n].colour,vpic[n].contrast,vpic[n].whiteness,vpic[n].depth,vpic[n].palette);
      printf("video4linux: capture palette %d\n",vpic[n].palette);
	  
      gb_buffers[n].frames=2;
      if (cap[n].type & VID_TYPE_CAPTURE) {
	/* map grab buffer */
	if (-1 == ioctl(fdv4l[n],VIDIOCGMBUF,&gb_buffers[n])) {perror("ioctl VIDIOCGMBUF"); return -1;}
	
	map[n] = mmap(0,gb_buffers[n].size,PROT_READ|PROT_WRITE,MAP_SHARED,fdv4l[n],0);
	if ((char*)-1 != map[n]) 
	  printf("Memory mapped: %d Bytes for %d frames\n",gb_buffers[n].size,gb_buffers[n].frames);
	else { perror("mmap"); return -1;}
      }

      gb[n].frame=0;
      gb[n].width=win[n].width;
      gb[n].height=win[n].height;
      gb[n].format=vpic[n].palette;
	  
      /* init Lookup tables for color conversion */
      for (k = 0; k < 256; k++) {
	ng_yuv_gray[k] = k * LUN_MUL >> 8;
	ng_yuv_red[k]  = (RED_ADD    + k * RED_MUL)    >> 8;
	ng_yuv_blue[k] = (BLUE_ADD   + k * BLUE_MUL)   >> 8;
	ng_yuv_g1[k]   = (GREEN1_ADD + k * GREEN1_MUL) >> 8;
	ng_yuv_g2[k]   = (GREEN2_ADD + k * GREEN2_MUL) >> 8;
      }
      
      for (k = 0; k < CLIP; k++)
	ng_clip[k] = 0;
      for (; k < CLIP + 256; k++)
	ng_clip[k] = k - CLIP;
      for (; k < 2 * CLIP + 256; k++)
	ng_clip[k] = 255;

	  
      if(n==0) pthread_create(&v4l_thread[0],NULL,video4linux1_thread,NULL); 
      else if(n==1) pthread_create(&v4l_thread[1],NULL,video4linux2_thread,NULL);
      else if(n==2) pthread_create(&v4l_thread[2],NULL,video4linux3_thread,NULL);
      else if(n==3) pthread_create(&v4l_thread[3],NULL,video4linux4_thread,NULL);
    }
  }

  /* inicializamos todos los esquemas de imagenes utilizados. */
  if(serve_color[0]){
    all[num_schemas].id = (int *) &colorA_schema_id;
    strcpy(all[num_schemas].name,"colorA");
    all[num_schemas].resume = (resumeFn) mycolorA_resume;
    all[num_schemas].suspend = (suspendFn) mycolorA_suspend;
    printf("%s schema loaded (id %d)\n",all[num_schemas].name,num_schemas);
    (*(all[num_schemas].id)) = num_schemas;
    all[num_schemas].fps = 0.;
    all[num_schemas].k =0;
    all[num_schemas].state=slept;
    all[num_schemas].close = NULL;
    all[num_schemas].handle = NULL;
    num_schemas++;
  }

  if(serve_color[1]){
    all[num_schemas].id = (int *) &colorB_schema_id;
    strcpy(all[num_schemas].name,"colorB");
    all[num_schemas].resume = (resumeFn) mycolorB_resume;
    all[num_schemas].suspend = (suspendFn) mycolorB_suspend;
    printf("%s schema loaded (id %d)\n",all[num_schemas].name,num_schemas);
    (*(all[num_schemas].id)) = num_schemas;
    all[num_schemas].fps = 0.;
    all[num_schemas].k =0;
    all[num_schemas].state=slept;
    all[num_schemas].close = NULL;
    all[num_schemas].handle = NULL;
    num_schemas++;
  }

  if(serve_color[2]){
    all[num_schemas].id = (int *) &colorC_schema_id;
    strcpy(all[num_schemas].name,"colorC");
    all[num_schemas].resume = (resumeFn) mycolorC_resume;
    all[num_schemas].suspend = (suspendFn) mycolorC_suspend;
    printf("%s schema loaded (id %d)\n",all[num_schemas].name,num_schemas);
    (*(all[num_schemas].id)) = num_schemas;
    all[num_schemas].fps = 0.;
    all[num_schemas].k =0;
    all[num_schemas].state=slept;
    all[num_schemas].close = NULL;
    all[num_schemas].handle = NULL;
    num_schemas++;
  }

  if(serve_color[3]){
    all[num_schemas].id = (int *) &colorD_schema_id;
    strcpy(all[num_schemas].name,"colorD");
    all[num_schemas].resume = (resumeFn) mycolorD_resume;
    all[num_schemas].suspend = (suspendFn) mycolorD_suspend;
    printf("%s schema loaded (id %d)\n",all[num_schemas].name,num_schemas);
    (*(all[num_schemas].id)) = num_schemas;
    all[num_schemas].fps = 0.;
    all[num_schemas].k =0;
    all[num_schemas].state=slept;
    all[num_schemas].close = NULL;
    all[num_schemas].handle = NULL;
    num_schemas++;
  }

  return 0;
}

/** video4linux driver startup function following jdec platform API for drivers.
 *  @param configfile path and name to the config file of this driver.*/
void video4linux_startup(char *configfile)
{
  int i,j;

  /* reseting serve color array and setting default options */
  for(i=0;i<MAXCAM;i++){serve_device[i]=0; serve_color[i]=0; state[i]=slept; color_device[i]=-1;}
  for(i=0;i<MAXCAM;i++){for(j=0;j<MAXCAM;j++){device_color[i][j]=0;}}
  strcpy(v4l_filename[0],"/dev/video0");
  strcpy(v4l_filename[1],"/dev/video1");
  strcpy(v4l_filename[2],"/dev/video2");
  strcpy(v4l_filename[3],"/dev/video3");

  /* we call the function to parse the config file */
  if(video4linux_parseconf(configfile)==-1){
    printf("video4linux: cannot initiate driver. configfile parsing error.\n");
    exit(-1);
  }

  /* video4linux driver init */
  if(video4linux_init()!=0){
    printf("video4linux: cannot initiate driver. cameras not ready or not supported.\n");
    exit(-1);
  }

  printf("video4linux driver started up\n");
}
