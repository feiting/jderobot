/*
 *  Copyright (C) 2006 José María Cañas Plaza 
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
 *
 *  Authors : José María Cañas Plaza <jmplaza@gsyc.escet.urjc.es>
 */


#ifndef tvoxel
#define tvoxel
typedef struct voxel{
  float x;
  float y;}Tvoxel;
#endif

extern void us2xy(int numsensor, float d,float phi, Tvoxel *point);
extern void laser2xy(int reading, float d, Tvoxel *point);
extern void init_pioneer(void);

/***************** Robot Configuration ***************/
#define NUM_LASER 180
#define NUM_SONARS 16
#define NUM_BUMPERS 10
#define MAX_VEL 1000 /* mm/sec, hardware limit: 1800 */
#define MAX_RVEL 180 /* deg/sec, hardware limit: 360 */
/* SIF image size */
#define SIFNTSC_ROWS 240
#define SIFNTSC_COLUMNS 320
/* directed perception pantilt limits */
#define MAX_PAN_ANGLE 158. /* degrees */
#define MIN_PAN_ANGLE -158. /* degrees */
#define MAX_TILT_ANGLE 30. /* degrees */
#define MIN_TILT_ANGLE -46. /* degrees */
#define MAX_SPEED_PANTILT 205.89

extern float laser_coord[5]; /* laser sensor position */
extern float us_coord[NUM_SONARS][5];/* us sensor positions */
extern float camera_coord[5]; /* camera position */


/***************** API of variables ***************/
extern float jde_robot[5]; /* odometry information */
extern unsigned long int encoders_clock;

extern int jde_laser[NUM_LASER];
extern unsigned long int laser_clock;

extern float us[NUM_SONARS];
extern unsigned long int us_clock[NUM_SONARS];

extern char *colorA; /* sifntsc image itself */
extern unsigned long int imageA_clock;

extern char *colorB; /* sifntsc image itself */
extern unsigned long int imageB_clock;

extern char *colorC; /* sifntsc image itself */
extern unsigned long int imageC_clock;

extern char *colorD; /* sifntsc image itself */
extern unsigned long int imageD_clock;

extern float pan_angle, tilt_angle;  /* degs */
extern unsigned long int pantiltencoders_clock;

extern float v; /* mm/s */
extern float w; /* deg/s*/
extern int motors_cycle;

extern float longitude; /* degs, pan angle */
extern float latitude; /* degs, tilt angle */
extern float longitude_speed;
extern float latitude_speed;
extern int pantiltmotors_cycle;


