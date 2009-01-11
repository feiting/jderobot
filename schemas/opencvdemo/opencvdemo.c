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
 *  Authors : Eduardo Perdices García <edupergar@gmail.com>
 */

#include "jde.h"
#include "opencvdemo.h"
#include <graphics_gtk.h>
#include <colorspaces.h>
#include <cv.h>

#include <glade/glade.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>

#define PI 3.141592654
#define SQUARE(a) (a)*(a)

int opencvdemo_id=0;
int opencvdemo_brothers[MAX_SCHEMAS];
arbitration opencvdemo_callforarbitration;

/*Imported variables*/
int* width;
int* height;
char** color;
resumeFn color_resume;
suspendFn color_suspend;
int image_ok=0;
int show_image=0;


int radio_original=1;
int radio_gray=0;
int radio_color=0;
int radio_canny=0;
int radio_sobel=0;
int radio_opflow=0;
int radio_pyramid=0;
int radio_convol=0;

int opflow_first=1;
IplImage *previous;

/*Configuration parameters*/

float hmax = 2*PI;
float hmin = 0.0;
float smax = 1.0;
float smin = 0.0;
float vmax = 255.0;
float vmin= 0.0;
int sizekernel = 0;
int modulo = 1;
int offset = 0;
float * kernel = NULL;

float threshold = 1;

/*Global variables*/
char *image;
char *image_aux;
pthread_mutex_t main_mutex;

registerdisplay myregister_displaycallback;
deletedisplay mydelete_displaycallback;

/* exported variables */
int opencvdemo_cycle=80;

/*GUI Variables*/
GladeXML *xml;
GtkWidget *win;

void load_image();
void free_image();

/*Callbacks*/
gboolean on_delete_window (GtkWidget *widget, GdkEvent *event, gpointer user_data){
   gdk_threads_leave();
   opencvdemo_guisuspend();
   gdk_threads_enter();
   return TRUE;
}

void on_active_image_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
   if (image_ok){
      if (gtk_check_menu_item_get_active(menu_item)){
		color_resume(opencvdemo_id,NULL,NULL);
		load_image();
         show_image=1;
      }
      else{
		free_image();
		color_suspend();
         show_image=0;
      }
   }
   gtk_window_resize (GTK_WINDOW(win),1,1);
}

void on_active_original_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_original)
		radio_original=0;
	else 
		radio_original=1;
}

void on_active_gray_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_gray)
		radio_gray=0;
	else 
		radio_gray=1;
}

void on_active_color_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_color) {
		radio_color=0;
		gtk_widget_hide(glade_xml_get_widget(xml, "tableColor"));
	} else {
		radio_color=1;
		gtk_widget_show(glade_xml_get_widget(xml, "tableColor"));
	}
}

void on_active_canny_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_canny) {
		radio_canny=0;
		gtk_widget_hide(glade_xml_get_widget(xml, "tableCanny"));
	} else {
		radio_canny=1;
		gtk_widget_show(glade_xml_get_widget(xml, "tableCanny"));
	}
}

void on_active_sobel_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_sobel)
		radio_sobel=0;
	else 
		radio_sobel=1;
}

void on_active_opflow_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_opflow)
		radio_opflow=0;
	else {
		radio_opflow=1;
		opflow_first=1;
	}
}

void on_active_pyramid_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_pyramid)
		radio_pyramid=0;
	else 
		radio_pyramid=1;
}

void on_active_convol_toggled (GtkCheckMenuItem *menu_item, gpointer user_data){
	if(radio_convol)
		radio_convol=0;
	else {
		radio_convol=1;
	}
}

/* Load images */
void load_image() {
	pthread_mutex_lock(&main_mutex);
	image=(char *)malloc(width[0]*height[0]*3);
	image_aux=(char *)malloc(width[0]*height[0]*3);
	{
		GdkPixbuf *imgBuff;
		GtkImage *img=(GtkImage *)glade_xml_get_widget(xml, "image");
		imgBuff = gdk_pixbuf_new_from_data((unsigned char *)image,
                                             GDK_COLORSPACE_RGB,0,8,
                                             width[0],height[0],
                                             width[0]*3,NULL,NULL);
		gtk_image_clear(img);
		gtk_image_set_from_pixbuf(img, imgBuff);
		gtk_widget_queue_draw(GTK_WIDGET(img));

		GdkPixbuf *imgBuff_aux;
		GtkImage *img_aux=(GtkImage *)glade_xml_get_widget(xml, "image_aux");
		imgBuff_aux = gdk_pixbuf_new_from_data((unsigned char *)image_aux,
                                             GDK_COLORSPACE_RGB,0,8,
                                             width[0],height[0],
                                             width[0]*3,NULL,NULL);
		gtk_image_clear(img_aux);
		gtk_image_set_from_pixbuf(img_aux, imgBuff_aux);
		gtk_widget_queue_draw(GTK_WIDGET(img_aux));

	}
	pthread_mutex_unlock(&main_mutex);
}

/* Free images */
void free_image() {
	pthread_mutex_lock(&main_mutex);
	if(image!=NULL) {
		free(image);
		free(image_aux);
		image = NULL;
		image_aux = NULL;
	}
	pthread_mutex_unlock(&main_mutex);
}

/*Check HSV values*/
int valuesOK(double H, double S, double V) {

	if(!((S <= smax) && (S >= smin) && (V <= vmax) && (V >= vmin)))
		return 0;

	H = H*PI/180.0;

	if(hmin < hmax) {
		if((H <= hmax) && (H >= hmin))
			return 1;
	} else {
		if(((H >= 0.0) && (H <= hmax)) || ((H <= 2*PI) && (H >= hmin))) 
			return 1;
	}

	return 0;
}

void colorFilter() {

	struct HSV* myHSV;
	double r,g,b;
	int i;

	for (i=0;i<width[0]*height[0]; i++){
			r = (float)(unsigned int)(unsigned char) image[i*3];
			g = (float)(unsigned int)(unsigned char) image[i*3+1];
			b = (float)(unsigned int)(unsigned char) image[i*3+2];

			myHSV = (struct HSV*) RGB2HSV_getHSV((int)r,(int)g,(int)b);	

			if(valuesOK(myHSV->H, myHSV->S, myHSV->V)) {
				image_aux[i*3] = image[i*3];
				image_aux[i*3+1] = image[i*3+1];
				image_aux[i*3+2] = image[i*3+2];
			}  else {
	    		/* Gray Scale */
				image_aux[i*3] = (unsigned char) (myHSV->V*100/255);
				image_aux[i*3+1] = (unsigned char) (myHSV->V*100/255);
				image_aux[i*3+2] = (unsigned char) (myHSV->V*100/255);
			}
	}
}

void grayScale() {

	CvMat vector1;
	CvMat* vector2;
	CvMat vector3;

	cvInitMatHeader(&vector1, width[0],height[0], CV_8UC3, image, CV_AUTOSTEP);
	vector2 = cvCreateMat( width[0],height[0], CV_8UC1);
	cvInitMatHeader(&vector3, width[0],height[0], CV_8UC3, image_aux, CV_AUTOSTEP);

	cvCvtColor(&vector1, vector2, CV_RGB2GRAY);
	cvCvtColor(vector2, &vector3, CV_GRAY2RGB);

	cvReleaseMat(&vector2);
}

void cannyFilter() {
	
	IplImage *src;  
	IplImage *gray; 
	IplImage *edge; 
	IplImage *cedge; 

	src = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	cedge = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	gray = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);
	edge = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);

	memcpy(src->imageData, image, src->width*src->height*src->nChannels);  

	cvCvtColor(src, gray, CV_RGB2GRAY);	
	cvCanny(gray, edge, threshold, threshold*3, 3);
 	cvZero(cedge);
	cvCopy(src, cedge, edge );
		
	memcpy(image_aux, cedge->imageData, src->width*src->height*src->nChannels);  

	cvReleaseImage(&src);
	cvReleaseImage(&gray);
	cvReleaseImage(&edge);
	cvReleaseImage(&cedge);
}

void sobelFilter() {
	
	IplImage *src;  
	IplImage *gray; 
	IplImage *edge; 
	IplImage *cedge; 

	src = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	cedge = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	gray = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);
	edge = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);

	memcpy(src->imageData, image, src->width*src->height*src->nChannels); 

	cvCvtColor(src, gray, CV_RGB2GRAY);
	cvSobel(gray, edge, 1, 1, 3);
	cvCvtColor(edge, cedge, CV_GRAY2RGB);

	memcpy(image_aux, cedge->imageData, src->width*src->height*src->nChannels);

	cvReleaseImage(&src);
	cvReleaseImage(&gray);
	cvReleaseImage(&edge);
	cvReleaseImage(&cedge);
}

void opticalFlow() {

	IplImage *src;
	IplImage *img1, *img2, *aux1, *aux2,  *aux3, *aux4;

	if(opflow_first) {
		if(previous==NULL)
			previous = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);

		memcpy(previous->imageData, image, previous->width*previous->height*previous->nChannels);
		memcpy(image_aux, previous->imageData, previous->width*previous->height*previous->nChannels);
		opflow_first = 0; 
		return;
	}

	src = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	memcpy(src->imageData, image, src->width*src->height*src->nChannels);

	/* Images with feature points */
	img1 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);
	img2 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);

	/*Temp images for algorithms*/
	aux1 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_32F, 1);
	aux2 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_32F, 1);
	aux3 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);
	aux4 = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 1);

	cvConvertImage(previous, img1);
	cvConvertImage(src, img2);

	int i;
	int numpoints = 400;
	CvPoint2D32f points1[numpoints];	/*Freature points from img1*/
	CvPoint2D32f points2[numpoints];	/*Location of the feature points in img2*/
	char foundPoint[numpoints];			
	float errors[numpoints];
	CvSize sizeWindow = cvSize(5,5);
	CvTermCriteria termCriteria;
																
	termCriteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );

	/*	Shi and Tomasi algorithm, get feature points from img1 */	
	cvGoodFeaturesToTrack(img1, aux1, aux2, points1, &numpoints, 0.05, 0.1, NULL, 3, 0, 0.04);

	/* Pyramidal Lucas Kanade Optical Flow algorithm, search feature points in img2 */
	cvCalcOpticalFlowPyrLK(img1, img2, aux3, aux4, points1, points2, numpoints, sizeWindow, 5, foundPoint, errors, termCriteria, 0);

	/* Draw arrows*/
	for(i = 0; i < numpoints; i++)	{
			if ( foundPoint[i] == 0 )	
				continue;

			int line_thickness = 1;
			CvScalar line_color = CV_RGB(255,0,0);
	
			CvPoint p,q;
			p.x = (int) points1[i].x;
			p.y = (int) points1[i].y;
			q.x = (int) points2[i].x;
			q.y = (int) points2[i].y;

			double angle = atan2( (double) p.y - q.y, (double) p.x - q.x );
			double hypotenuse = sqrt(SQUARE(p.y - q.y) + SQUARE(p.x - q.x));

			if(hypotenuse < 2 || hypotenuse > 60)
				continue;

			/*Line*/
			q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
			q.y = (int) (p.y - 3 * hypotenuse * sin(angle));
			cvLine(src, p, q, line_color, line_thickness, CV_AA, 0 );
		
			/*Arrow*/
			p.x = (int) (q.x + 9 * cos(angle + PI / 4));
			p.y = (int) (q.y + 9 * sin(angle + PI / 4));
			cvLine(src, p, q, line_color, line_thickness, CV_AA, 0 );
			p.x = (int) (q.x + 9 * cos(angle - PI / 4));
			p.y = (int) (q.y + 9 * sin(angle - PI / 4));
			cvLine(src, p, q, line_color, line_thickness, CV_AA, 0 );
	}

	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&aux1);
	cvReleaseImage(&aux2);
	cvReleaseImage(&aux3);
	cvReleaseImage(&aux4);

	memcpy(previous->imageData, image, previous->width*previous->height*previous->nChannels);
	memcpy(image_aux, src->imageData, src->width*src->height*src->nChannels);
	
	cvReleaseImage(&src);
}

void pyramid() {

	IplImage *src;
	IplImage *div2;
	IplImage *div4;
	IplImage *div8;
	IplImage *div16;
	IplImage *dst;
	int i,w,w2,tmp;

	src = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	div2 = cvCreateImage(cvSize(width[0]/2,height[0]/2), IPL_DEPTH_8U, 3);
	div4 = cvCreateImage(cvSize(width[0]/4,height[0]/4), IPL_DEPTH_8U, 3);
	div8 = cvCreateImage(cvSize(width[0]/8,height[0]/8), IPL_DEPTH_8U, 3);
	div16 = cvCreateImage(cvSize(width[0]/16,height[0]/16), IPL_DEPTH_8U, 3);
	dst = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);

	memcpy(src->imageData, image, src->width*src->height*src->nChannels);

	/*Pyramids*/
	cvPyrDown(src, div2, CV_GAUSSIAN_5x5);
	cvPyrDown(div2, div4, CV_GAUSSIAN_5x5);
	cvPyrDown(div4, div8, CV_GAUSSIAN_5x5);
	cvPyrDown(div8, div16, CV_GAUSSIAN_5x5);

	/*Copy pyramids to dst*/
 	cvZero(dst);
	w = width[0]*3;
	for(i=0; i<height[0]/2; i++) {
		w2 = div2->width*div2->nChannels;
		memcpy((dst->imageData)+w*i, (div2->imageData)+w2*i, w2);
		if(i<height[0]/4) {
			tmp = (width[0]/2)*3;
			w2 = div4->width*div4->nChannels;
			memcpy((dst->imageData)+w*i+tmp, (div4->imageData)+w2*i, w2);
		}
		if(i<height[0]/8) {
			tmp = (width[0]/2 + width[0]/4)*3;
			w2 = div8->width*div8->nChannels;
			memcpy((dst->imageData)+w*i+tmp, (div8->imageData)+w2*i, w2);
		}
		if(i<height[0]/16) {
			tmp = (width[0]/2 + width[0]/4 + width[0]/8)*3;
			w2 = div16->width*div16->nChannels;
			memcpy((dst->imageData)+w*i+tmp, (div16->imageData)+w2*i, w2);
		}
	}

	memcpy(image_aux, dst->imageData, dst->width*dst->height*dst->nChannels);

	cvReleaseImage(&src);
	cvReleaseImage(&div2);
	cvReleaseImage(&div4);
	cvReleaseImage(&div8);
	cvReleaseImage(&div16);
	cvReleaseImage(&dst);

}

void convolution() {

	if(kernel==NULL) {
		printf("Error initializing kernel\n");
		return;
	}

	CvMat *mask; 
	IplImage *src, *tmp, *dst;
	int i, h, w;
	int elems = sizekernel*sizekernel;

	src = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	tmp = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	dst = cvCreateImage(cvSize(width[0],height[0]), IPL_DEPTH_8U, 3);
	mask = cvCreateMat(sizekernel, sizekernel, CV_32FC1);

	/* Save mask in CvMat format*/
	for(i=0;i<elems;i++) {
		h = i/sizekernel;
		w = i%sizekernel;
		if(modulo > 1)
			cvSet2D(mask,h, w, cvScalar(kernel[i]/modulo,0,0,0));
		else
			cvSet2D(mask,h, w, cvScalar(kernel[i],0,0,0));
	}

	memcpy(src->imageData, image, src->width*src->height*src->nChannels);

	if (offset != 0) {
		cvFilter2D(src, tmp, mask, cvPoint(-1,-1));
		/*Add offset*/
		cvAddS(tmp, cvScalarAll(offset), dst, 0);
	} else
		cvFilter2D(src, dst, mask, cvPoint(-1,-1));

	memcpy(image_aux, dst->imageData, src->width*src->height*src->nChannels);

	cvReleaseMat(&mask);
	cvReleaseImage(&src);
	cvReleaseImage(&tmp);
	cvReleaseImage(&dst);
}

void opencvdemo_iteration(){
	int i;

	speedcounter(opencvdemo_id);
	pthread_mutex_lock(&main_mutex);

	if(image_ok && image!=NULL) {
		for (i=0;i<width[0]*height[0]; i++){
			image[i*3]=(*color)[i*3+2];
			image[i*3+1]=(*color)[i*3+1];
			image[i*3+2]=(*color)[i*3];

			if(radio_original) {
				image_aux[i*3]=image[i*3];
				image_aux[i*3+1]=image[i*3+1];
				image_aux[i*3+2]=image[i*3+2];
			}
		}
		if(radio_color) 
			colorFilter();
		else if(radio_gray)
			grayScale();
		else if(radio_canny)
			cannyFilter();
		else if(radio_sobel)
			sobelFilter();
		else if(radio_opflow)
			opticalFlow();
		else if(radio_pyramid)
			pyramid();
		else if(radio_convol)
			convolution();
	}
	pthread_mutex_unlock(&main_mutex);
}


void opencvdemo_imports(){
	width=myimport("varcolorA","width");
	height=myimport("varcolorA","height");
	color=myimport("varcolorA","varcolorA");
	color_resume=(resumeFn)myimport("varcolorA","resume");
	color_suspend=(suspendFn)myimport("varcolorA","suspend");

	image_ok=!(width==NULL || height==NULL || color==NULL || color_resume==NULL || color_suspend==NULL);
   if (!image_ok){
      printf ("Can't load image\n");
		jdeshutdown(1);
   }
}

void opencvdemo_exports(){

   myexport("opencvdemo", "id", &opencvdemo_id);
   myexport("opencvdemo","cycle",&opencvdemo_cycle);
   myexport("opencvdemo","resume",(void *)opencvdemo_resume);
   myexport("opencvdemo","suspend",(void *)opencvdemo_suspend);
}

void opencvdemo_init(){
   if (myregister_displaycallback==NULL){
      if ((myregister_displaycallback=
           (registerdisplay)myimport ("graphics_gtk",
            "register_displaycallback"))==NULL)
      {
         printf ("I can't fetch register_displaycallback from graphics_gtk\n");
         jdeshutdown(1);
      }
      if ((mydelete_displaycallback=
           (deletedisplay)myimport ("graphics_gtk", "delete_displaycallback"))
           ==NULL)
      {
         printf ("I can't fetch delete_displaycallback from graphics_gtk\n");
         jdeshutdown(1);
      }
   }
}

void opencvdemo_end(){
}

void opencvdemo_stop(){
}

void opencvdemo_suspend()
{
  pthread_mutex_lock(&(all[opencvdemo_id].mymutex));
  put_state(opencvdemo_id,slept);
  printf("opencvdemo: off\n");
  pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
  opencvdemo_end();
}


void opencvdemo_resume(int father, int *brothers, arbitration fn)
{
  int i;

  /* update the father incorporating this schema as one of its children */
  if (father!=GUIHUMAN && father!=SHELLHUMAN)
    {
      pthread_mutex_lock(&(all[father].mymutex));
      all[father].children[opencvdemo_id]=TRUE;
      pthread_mutex_unlock(&(all[father].mymutex));
    }

  pthread_mutex_lock(&(all[opencvdemo_id].mymutex));
  /* this schema resumes its execution with no children at all */
  for(i=0;i<MAX_SCHEMAS;i++) all[opencvdemo_id].children[i]=FALSE;
  all[opencvdemo_id].father=father;
  if (brothers!=NULL)
    {
      for(i=0;i<MAX_SCHEMAS;i++) opencvdemo_brothers[i]=-1;
      i=0;
      while(brothers[i]!=-1) {opencvdemo_brothers[i]=brothers[i];i++;}
    }
  opencvdemo_callforarbitration=fn;
  put_state(opencvdemo_id,notready);
  printf("opencvdemo: on\n");

  RGB2HSV_init();
  RGB2HSV_createTable();

  pthread_cond_signal(&(all[opencvdemo_id].condition));
  pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
  opencvdemo_imports();
}

void *opencvdemo_thread(void *not_used)
{
   struct timeval a,b;
   long n=0; /* iteration */
   long next,bb,aa;

   for(;;)
   {
      pthread_mutex_lock(&(all[opencvdemo_id].mymutex));

      if (all[opencvdemo_id].state==slept)
      {
	 pthread_cond_wait(&(all[opencvdemo_id].condition),&(all[opencvdemo_id].mymutex));
	 pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
      }
      else
      {
	 /* check preconditions. For now, preconditions are always satisfied*/
	 if (all[opencvdemo_id].state==notready)
	    put_state(opencvdemo_id,ready);
	 /* check brothers and arbitrate. For now this is the only winner */
	 if (all[opencvdemo_id].state==ready)
	 {put_state(opencvdemo_id,winner);
	 gettimeofday(&a,NULL);
	 aa=a.tv_sec*1000000+a.tv_usec;
	 n=0;
	 }

	 if (all[opencvdemo_id].state==winner)
	    /* I'm the winner and must execute my iteration */
	 {
	    pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
	    /*      gettimeofday(&a,NULL);*/
	    n++;
	    opencvdemo_iteration();
	    gettimeofday(&b,NULL);
	    bb=b.tv_sec*1000000+b.tv_usec;
	    next=aa+(n+1)*(long)opencvdemo_cycle*1000-bb;

	    if (next>5000)
	    {
	       usleep(next-5000);
	       /* discounts 5ms taken by calling usleep itself, on average */
	    }
	    else  ;
	 }
	 else
	    /* just let this iteration go away. overhead time negligible */
	 {
	    pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
	    usleep(opencvdemo_cycle*1000);
	 }
      }
   }
}

int opencvdemo_parseconf(char *configfile){

	int end_parse=0; int end_section=0; int schema_config_parsed=0;
	int row_parsed=0;
   	FILE *myfile;
   	const int limit = 256;

	if ((myfile=fopen(configfile,"r"))==NULL){
		printf("Opencvdemo: cannot find config file\n");
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
		/* white spaces or tabs*/
		}else if (buffer_file[0]==' ' || buffer_file[0]=='\t') {
			while(buffer_file[0]==' ' || buffer_file[0]=='\t') 
				buffer_file[0]=fgetc(myfile);
		/* storing line in buffer */
		}else{
      
			while(buffer_file[i]!='\n') buffer_file[++i]=fgetc(myfile);
			buffer_file[++i]='\0';

			if (i >= limit-1) {
				printf("%s...\n", buffer_file);
				printf ("Opencvdemo: line too long in config file!\n");
				exit(-1);
			}
      
			/* first word of the line */
			if (sscanf(buffer_file,"%s",word)==1){
				if (strcmp(word,"schema")==0) {
					while((buffer_file[j]!='\n')&&(buffer_file[j]!=' ')&&(buffer_file[j]!='\0')&&(buffer_file[j]!='\t')) j++;
					sscanf(&buffer_file[j],"%s",word2);
	  
					/* checking if this section matchs our driver name */
					if (strcmp(word2,"opencvdemo")==0){
						/* the sections match */
						do{
	      
							char buffer_file2[256],word3[256],word4[256];
							int k=0;
							int words=0;

							buffer_file2[0]=fgetc(myfile);
	      
							/* end of file */
							if (feof(myfile)){
								end_section=1;
								end_parse=1;
							/* line comment */
							}else if (buffer_file2[0]=='#') {
								while(fgetc(myfile)!='\n');
	      
                        	/* white spaces or tabs*/
							}else if (buffer_file2[0]==' ' || buffer_file2[0]=='\t') {
								while(buffer_file2[0]==' ' || buffer_file2[0]=='\t') 
									buffer_file2[0]=fgetc(myfile);
                        	/* storing line in buffer */
							}else{
		
								while(buffer_file2[k]!='\n') buffer_file2[++k]=fgetc(myfile);
									buffer_file2[++k]='\0';
		
								/* first word of the line */
								if (sscanf(buffer_file2,"%s",word3)==1){
									if (strcmp(word3,"end_schema")==0) {
										schema_config_parsed=1;
										end_section=1;
										end_parse=1;
				
									}else if (strcmp(word3,"schema")==0) {
										printf("Opencvdemo: error in config file.\n'end_section' "
												"keyword required before starting new schema section.\n");
										end_section=1; end_parse=1;

									}else if(strcmp(word3,"hmax")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											hmax=atof(word4);
										else
											printf("Opencvdemo: 'hmax' line incorrect\n");
									}else if(strcmp(word3,"hmin")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											hmin=atof(word4);
										else
											printf("Opencvdemo: 'hmin' line incorrect\n");
									}else if(strcmp(word3,"smax")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											smax=atof(word4);
										else
											printf("Opencvdemo: 'smax' line incorrect\n");
									}else if(strcmp(word3,"smin")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											smin=atof(word4);
										else
											printf("Opencvdemo: 'smin' line incorrect\n");
									}else if(strcmp(word3,"vmax")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											vmax=atof(word4);
										else
											printf("Opencvdemo: 'vmax' line incorrect\n");
									}else if(strcmp(word3,"vmin")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											vmin=atof(word4);
										else
											printf("Opencvdemo: 'vmin' line incorrect\n");
									}else if(strcmp(word3,"size")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											sizekernel=atoi(word4);
										else
											printf("Opencvdemo: 'size' line incorrect\n");

										if(sizekernel!= 3 && sizekernel !=5) {
											printf("Opencvdemo: Size must be 3 or 5\n");
											sizekernel = 0;
										} else {
											kernel=(float *)malloc(sizeof(float)*sizekernel*sizekernel);
											memset(kernel, 0, sizeof(float)*sizekernel*sizekernel);
										}
									}else if(strcmp(word3,"modulo")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											modulo=atoi(word4);
										else
											printf("Opencvdemo: 'modulo' line incorrect\n");	
									}else if(strcmp(word3,"offset")==0){
										words=sscanf(buffer_file2,"%s %s",word3, word4);
										if (words==2) 
											offset=atoi(word4);
										else
											printf("Opencvdemo: 'offset' line incorrect\n");	
									}else if(strcmp(word3,"row")==0){
										if(sizekernel==0 || kernel==NULL)
											printf("Opencvdemo: Kernel size not set\n");
										else {
											char val[5][256];
											int fila;
											int z;
											
											if(sizekernel==3)
												words=sscanf(buffer_file2,"%s %s %s %s %s",word3, word4, val[0], val[1], val[2]);
											else if(sizekernel==5)
												words=sscanf(buffer_file2,"%s %s %s %s %s %s %s",word3, word4, val[0], val[1], val[2], val[3], val[4]);
	
											if (words==(sizekernel+2)) {
												fila = atoi(word4);
												row_parsed = 1;
												if(fila<1 || fila>sizekernel)
													printf("Opencvdemo: Row number out of range\n");
												else {
													fila--;
													for(z=0;z<sizekernel;z++)
														kernel[fila*sizekernel+z] = atof(val[z]);
												}
											} else
												printf("Opencvdemo: 'offset' line incorrect\n");
										}
									}else						
										printf("Opencvdemo: i don't know what to do with '%s'\n", buffer_file2);
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
	if(schema_config_parsed==1){
		if(sizekernel==0 || kernel==NULL || !row_parsed) {
			sizekernel = 3;
			kernel=(float *)malloc(sizeof(float)*3*3);
			kernel[0] = 1;
			kernel[1] = 1;
			kernel[2] = 1;
			kernel[3] = 1;
			kernel[4] = -7;
			kernel[5] = 1;
			kernel[6] = 1;
			kernel[7] = 1;
			kernel[8] = 1;
		}

		return 0;
	}else 
		return -1;
}

void opencvdemo_startup(char *configfile)
{
	/*Read the configfile*/
   	if(opencvdemo_parseconf(configfile)==-1){
		printf("Opencvdemo: cannot initiate schema. configfile parsing error.\n");
		jdeshutdown(-1);
	}	

	pthread_mutex_lock(&(all[opencvdemo_id].mymutex));
  	printf("opencvdemo schema started up\n");
  	opencvdemo_exports();
  	put_state(opencvdemo_id,slept);
  	pthread_create(&(all[opencvdemo_id].mythread),NULL,opencvdemo_thread,NULL);
  	pthread_mutex_unlock(&(all[opencvdemo_id].mymutex));
  	opencvdemo_init();
}

void opencvdemo_guidisplay(){
   pthread_mutex_lock(&main_mutex);
   if (show_image){
		GtkImage *img = GTK_IMAGE(glade_xml_get_widget(xml, "image"));
		GtkImage *img_aux = GTK_IMAGE(glade_xml_get_widget(xml, "image_aux"));

		gdk_threads_enter();
		gtk_widget_queue_draw(GTK_WIDGET(img));
		gtk_widget_queue_draw(GTK_WIDGET(img_aux));

		if(radio_color) {
			hmax = gtk_range_get_value(glade_xml_get_widget(xml, "hmax"));
			hmin = gtk_range_get_value(glade_xml_get_widget(xml, "hmin"));
			smax = gtk_range_get_value(glade_xml_get_widget(xml, "smax"));
			smin = gtk_range_get_value(glade_xml_get_widget(xml, "smin"));
			vmax = gtk_range_get_value(glade_xml_get_widget(xml, "vmax"));
			vmin = gtk_range_get_value(glade_xml_get_widget(xml, "vmin"));

			if(smin > smax) {
				smax = smin;
				gtk_range_set_value(glade_xml_get_widget(xml, "smax"),smax);
			} 

			if(vmin > vmax) {
				vmax = vmin;
				gtk_range_set_value(glade_xml_get_widget(xml, "vmax"),vmax);
			}
		}

		if(radio_canny) {
			threshold = gtk_range_get_value(glade_xml_get_widget(xml, "threshold"));
		}

		gdk_threads_leave();
   }
   pthread_mutex_unlock(&main_mutex);
}


void opencvdemo_guisuspend(void){
   mydelete_displaycallback(opencvdemo_guidisplay);
   if (win!=NULL){
      gdk_threads_enter();
      gtk_widget_hide(win);
      gdk_threads_leave();
   }
   all[opencvdemo_id].guistate=pending_off;
}

void opencvdemo_guiresume(void){
   static int cargado=0;
   static pthread_mutex_t opencvdemo_gui_mutex;

   pthread_mutex_lock(&opencvdemo_gui_mutex);
   if (!cargado){
      loadglade ld_fn;
      cargado=1;
      pthread_mutex_unlock(&opencvdemo_gui_mutex);
      /*Cargar la ventana desde el archivo xml .glade*/
      gdk_threads_enter();
      if ((ld_fn=(loadglade)myimport("graphics_gtk","load_glade"))==NULL){
         fprintf (stderr,"I can't fetch 'load_glade' from 'graphics_gtk'.\n");
         jdeshutdown(1);
      }
      xml = ld_fn ("opencvdemo.glade");
      if (xml==NULL){
         fprintf(stderr, "Error loading graphical interface\n");
         jdeshutdown(1);
      }
      win = glade_xml_get_widget(xml, "window");
      /*Conectar los callbacks*/
      {
         g_signal_connect(G_OBJECT(win), "delete-event",
                          G_CALLBACK(on_delete_window), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "active_image")),
                          "toggled", G_CALLBACK(on_active_image_toggled), NULL);

         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_original")),
                          "toggled", G_CALLBACK(on_active_original_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_gray")),
                          "toggled", G_CALLBACK(on_active_gray_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_color")),
                          "toggled", G_CALLBACK(on_active_color_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_canny")),
                          "toggled", G_CALLBACK(on_active_canny_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_sobel")),
                          "toggled", G_CALLBACK(on_active_sobel_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_opflow")),
                          "toggled", G_CALLBACK(on_active_opflow_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_pyramid")),
                          "toggled", G_CALLBACK(on_active_pyramid_toggled), NULL);
         g_signal_connect(GTK_WIDGET(glade_xml_get_widget(xml, "radio_convol")),
                          "toggled", G_CALLBACK(on_active_convol_toggled), NULL);
      }

		/*Set values to sliders*/
		gtk_range_set_value(glade_xml_get_widget(xml, "hmax"),hmax);
		gtk_range_set_value(glade_xml_get_widget(xml, "hmin"),hmin);
		gtk_range_set_value(glade_xml_get_widget(xml, "smax"),smax);
		gtk_range_set_value(glade_xml_get_widget(xml, "smin"),smin);
		gtk_range_set_value(glade_xml_get_widget(xml, "vmax"),vmax);
		gtk_range_set_value(glade_xml_get_widget(xml, "vmin"),vmin);
		gtk_range_set_value(glade_xml_get_widget(xml, "threshold"),threshold);

		/*Hide frames until they are displayed*/
		gtk_widget_hide(glade_xml_get_widget(xml, "tableColor"));
		gtk_widget_hide(glade_xml_get_widget(xml, "tableCanny"));

      if (win==NULL){
         fprintf(stderr, "Error loading graphical interface\n");
         jdeshutdown(1);
      }
      else{
         gtk_widget_show(win);
         gtk_widget_queue_draw(GTK_WIDGET(win));
      }

      gdk_threads_leave();
   }
   else{
      pthread_mutex_unlock(&opencvdemo_gui_mutex);
      gdk_threads_enter();
      gtk_widget_show(win);
      gtk_widget_queue_draw(GTK_WIDGET(win));
      gdk_threads_leave();
   }

   myregister_displaycallback(opencvdemo_guidisplay);
   all[opencvdemo_id].guistate=pending_on;
}

