/*
 *  Copyright (C) 2006 Jose Antonio Santos Cadenas, Javier Martín Ramos
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
 *  Authors : Jose Antonio Santos Cadenas <santoscadenas@gmail.com>
 *            Javier Martin Ramos <xaverbrennt@yahoo.es>
 *            Jose Maria Cañas <jmplaza@gsyc.escet.urjc.es>
 */

/**
 * jdec networkserver driver provides sensorial information (such as color,
 * laser or us) to remote clients.
 *
 *  @file networkserver.c
 *  @author Jose Antonio Santos Cadenas <santoscadenas@gmail.com> and Jose Maria Cañas Plaza <jmplaza@gsyc.escet.urjc.es>
 *  @version 1.0
 *  @date 2007-12-7
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "jde.h"
#include "jdemessages.h"

      /* constants for devices */
#define COLORA_DEVICE 0
#define COLORB_DEVICE 1
#define COLORC_DEVICE 2
#define COLORD_DEVICE 3
#define PANTILT_ENCODERS_DEVICE 4
#define PANTILT_MOTORS_DEVICE 5
#define LASER_DEVICE 6
#define SONARS_DEVICE 7
#define ENCODERS_DEVICE 8
#define MOTORS_DEVICE 9
      /* number of devices */
#define MAXDEVICE 10


/* networkserver driver API options */
/** The driver name*/
char driver_name[256]="networkserver";

/** Maximum number of clients waiting for server's accept*/
#define MAX_QUEUE 5


/** Client structure type definition*/
struct client{
   /** Socket referencing the conection with the client*/
   int cs;
   /** Shows to what services it's subscribed*/
   char subscriptions[MAXDEVICE];
   /** Client name*/
   char name[256];
   /** Stores the clock of the last data served*/
   unsigned long clocks[MAXDEVICE];
};
/** Client iteration cycle*/
#define CLIENT_TH_CYCLE 33 /*para servir las imágenes en tiempo real*/

/** Image nuber of channels*/
#define CANALES 3

/** port to bind the server */
int networkserver_port=0;

/** devices detected (the ones that are going to be served)*/
int serve_device[MAXDEVICE];
/** devfices network id */
int device_network_id[MAXDEVICE];

/*Import section*/
/**Imported variables from sensors schemas*/
void *variables[MAXDEVICE];
/** Sensor schemas resume functon*/
resumeFn resume[MAXDEVICE];
/** Sensor schemas suspend functon*/
suspendFn suspend[MAXDEVICE];

/** Mutex to wait until the client socket id is stored at client structure*/
pthread_mutex_t socketmutex;

/*PID's*/
/** Listen thread identifier*/
pthread_t listen_pid;


/** Close function following the jdec driver api*/
void networkserver_close(){

   printf("driver networkserver off\n");
}

/**
 * Networkclient driver parse configuration file function. Determines wich
 * devices are server and the connection port.
 * 
 * @param configfile path and name to the config file.
 * @return 0 if parsing was successful or -1 if something went wrong.*/
int networkserver_parseconf(char *configfile){

   int end_parse=0; int end_section=0; int driver_config_parsed=0;
   FILE *myfile;
   const int limit = 256;

   if ((myfile=fopen(configfile,"r"))==NULL){
      printf("networkserver: cannot find config file\n");
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
            printf ("networkserver: line too long in config file!\n");
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
                              printf("networkserver: error in config file.\n'end_section' keyword required before starting new driver section.\n");
                              end_section=1; end_parse=1;

                           }else if(strcmp(word3,"serves")==0){
                              while((buffer_file2[z]!='\n')&&(buffer_file2[z]!=' ')&&(buffer_file2[z]!='\0')&&(buffer_file2[z]!='\t')) z++;
                              if(sscanf(buffer_file2,"%s %s %s",word3,word4,word5)>2){
		      
                                 if((strcmp(word4,"colorA")==0)&&(serve_device[COLORA_DEVICE]==0)){
                                    serve_device[COLORA_DEVICE]=1;
                                    device_network_id[COLORA_DEVICE]=atoi(word5);
                                 }
                                 else if((strcmp(word4,"colorB")==0)&&(serve_device[COLORB_DEVICE]==0)){
                                    serve_device[COLORB_DEVICE]=1;
                                    device_network_id[COLORB_DEVICE]=atoi(word5);
                                 }
                                 else if((strcmp(word4,"colorC")==0)&&(serve_device[COLORC_DEVICE]==0)){
                                    serve_device[COLORC_DEVICE]=1;
                                    device_network_id[COLORC_DEVICE]=atoi(word5);
                                 }
                                 else if((strcmp(word4,"colorD")==0)&&(serve_device[COLORD_DEVICE]==0)){
                                    serve_device[COLORD_DEVICE]=1;
                                    device_network_id[COLORD_DEVICE]=atoi(word5);
                                 }
                                 else{
                                    printf("networkserver: serves line incorrect\n");
                                 }
                              }
                              else if(sscanf(buffer_file2,"%s %s",word3,word4)>1){
		      
                                 if((strcmp(word4,"laser")==0)&&(serve_device[LASER_DEVICE]==0)){serve_device[LASER_DEVICE]=1;}
                                 else if((strcmp(word4,"sonars")==0)&&(serve_device[SONARS_DEVICE]==0)){serve_device[SONARS_DEVICE]=1;}
                                 else if((strcmp(word4,"encoders")==0)&&(serve_device[ENCODERS_DEVICE]==0)){serve_device[ENCODERS_DEVICE]=1;}
                                 else if((strcmp(word4,"motors")==0)&&(serve_device[MOTORS_DEVICE]==0)){serve_device[MOTORS_DEVICE]=1;}
                                 else if((strcmp(word4,"pantiltencoders")==0)&&(serve_device[PANTILT_ENCODERS_DEVICE]==0)){serve_device[PANTILT_ENCODERS_DEVICE]=1;}
                                 else if((strcmp(word4,"pantiltmotors")==0)&&(serve_device[PANTILT_MOTORS_DEVICE]==0)){serve_device[PANTILT_MOTORS_DEVICE]=1;}
                                 else{printf("networkserver: serves line incorrect\n");}
                              }

                           }else if(strcmp(word3,"socket")==0){
                              while((buffer_file2[z]!='\n')&&(buffer_file2[z]!=' ')&&(buffer_file2[z]!='\0')&&(buffer_file2[z]!='\t')) z++;
                              if(sscanf(buffer_file2,"%s %s",word3,word4)>1){
                                 networkserver_port=atoi(word4);
                                 printf("networkserver: server will be started using port %d\n",networkserver_port);
                              }else{
                                 printf("networkserver: socket line incorrect\n");
                              }
                           }else printf("networkserver: i don't know what to do with '%s'\n",buffer_file2);
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
      if((serve_device[COLORA_DEVICE]==0)&&(serve_device[COLORB_DEVICE]==0)&&(serve_device[COLORC_DEVICE]==0)&&(serve_device[COLORD_DEVICE]==0)){
         if((serve_device[PANTILT_MOTORS_DEVICE]==0)&&(serve_device[PANTILT_ENCODERS_DEVICE]==0)){
            if((serve_device[MOTORS_DEVICE]==0)&&(serve_device[LASER_DEVICE]==0)&&(serve_device[ENCODERS_DEVICE]==0)&&(serve_device[SONARS_DEVICE]==0)){
               printf("networkserver: warning! no device served.\n");
            }
         }
      }
      return 0;
   }else return -1;
}

/** Initialization function*/
void init(){
   int i;
   for (i=0;i<MAXDEVICE; i++){
      /*Do myimport's*/
      if (serve_device[i]){
         switch (i){
            case COLORA_DEVICE:
               variables[i]=myimport ("colorA", "colorA");
               resume[i]=(resumeFn) myimport ("colorA", "resume");
               suspend[i]=(suspendFn) myimport ("colorA", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'colorA', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case COLORB_DEVICE:
               variables[i]=myimport ("colorB", "colorB");
               resume[i]=(resumeFn) myimport ("colorB", "resume");
               suspend[i]=(suspendFn) myimport ("colorB", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'colorB', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case COLORC_DEVICE:
               variables[i]=myimport ("colorC", "colorC");
               resume[i]=(resumeFn) myimport ("colorC", "resume");
               suspend[i]=(suspendFn) myimport ("colorC", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'colorC', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case COLORD_DEVICE:
               variables[i]=myimport ("colorD", "colorD");
               resume[i]=(resumeFn) myimport ("colorD", "resume");
               suspend[i]=(suspendFn) myimport ("colorD", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'colorD', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case LASER_DEVICE:
               variables[i]=myimport ("laser", "laser");
               resume[i]=(resumeFn) myimport("laser", "resume");
               suspend[i]=(suspendFn) myimport("laser", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'laser', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case ENCODERS_DEVICE:
               variables[i]=myimport ("encoders", "jde_robot");
               resume[i]=(resumeFn) myimport("encoders", "resume");
               suspend[i]=(suspendFn) myimport("encoders", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'encoders', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case SONARS_DEVICE:
               variables[i]=myimport ("sonars", "us");
               resume[i]=(resumeFn) myimport("sonars", "resume");
               suspend[i]=(suspendFn) myimport("sonars", "suspend");
               if (!variables[i]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'sonars', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            case MOTORS_DEVICE:
            {
               static float *motors[2];
               variables[i]=motors;
               motors[0]=(float *)myimport ("motors", "v");
               motors[1]=(float *)myimport ("motors", "w");
               resume[i]=(resumeFn) myimport("motors", "resume");
               suspend[i]=(suspendFn) myimport("motors", "suspend");
               if (!motors[0] || !motors[1]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'motors', it'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            }
            case PANTILT_ENCODERS_DEVICE:
            {
               static float *pantilt[2];
               variables[i]=pantilt;
               pantilt[0]=myimport ("ptencoders", "pan_angle");
               pantilt[1]=myimport ("ptencoders", "tilt_angle");
               resume[i]=(resumeFn) myimport("ptencoders", "resume");
               suspend[i]=(suspendFn) myimport("ptencoders", "suspend");
               if (!pantilt[0] || !pantilt[1]){
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'pantilt_encoders', they'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            }
            case PANTILT_MOTORS_DEVICE:
            {
               static float *pantilt_motors[4];
               variables[i]=pantilt_motors;
               pantilt_motors[0]=myimport ("ptmotors", "longitude");
               pantilt_motors[1]=myimport ("ptencoders", "latitude");
               pantilt_motors[2]=myimport ("ptmotors", "longitude_speed");
               pantilt_motors[3]=myimport ("ptencoders", "latitude_speed");
               resume[i]=(resumeFn) myimport("ptmotors", "resume");
               suspend[i]=(suspendFn) myimport("ptmotors", "suspend");
               if (!pantilt_motors[0] || !pantilt_motors[1] ||
                    !pantilt_motors[2] || !pantilt_motors[3])
               {
                  serve_device[i]=0;
                  fprintf (stderr, "I can't fetch 'pantilt_motors', they'll not be served\n");
               }
               else{
                  resume[i](-1, NULL, NULL);
               }
               break;
            }
         }
      }
   }
}

/**
 * Initializes network connection
 * @param s Pointer to the socket storage
 * @param addr_service Pointer to the sockaddr_in structure
 * @param port The port were the server will wait for connections
 */
void bootstrap(int *s, struct sockaddr_in *addr_service, unsigned short port) {
   *s = socket(AF_INET, SOCK_STREAM, 0);
   if (*s < 0) {
      perror("");
      jdeshutdown(0);
   }

   addr_service->sin_family = AF_INET;
   addr_service->sin_port = htons(port);
   addr_service->sin_addr.s_addr = INADDR_ANY;

   if (bind(*s, (struct sockaddr *) addr_service, sizeof(*addr_service)) < 0){
      perror("");
      jdeshutdown(0);
   }

   if (listen(*s, MAX_QUEUE) < 0) {
      perror("");
      jdeshutdown(0);
   }
}

/**
 * @brief Special write function to ignore the non block socket.
 * When a EAGAIN error apear it is ignored and the write operation is redone.
 *
 * @param fd File descriptor where the write operacion will done
 * @param buf Data to write
 * @param count Number of bytes to write from buf
 */
void my_write(int fd, const void *buf, size_t count) { 
   int escritos = 0;
   int total_escritos = 0;
   do {
      escritos = write(fd, (char *) buf + total_escritos, count-total_escritos);
      if (escritos < 0) {
         if (errno != EAGAIN){
            perror("I can't write in client socket");
            pthread_exit(0);
         }
      }
      else{
         total_escritos += escritos;
      }
   } while (total_escritos < count);
}

/**
 * @brief Receives a petition from a particular client, and dispatch it
 * The petition can be an image petition, a subscription petition or a motors
 * order. In the first case the image response will be send by de client socket,
 * in the second case the subscription will be registered and in the last one
 * the motors variable will be actualized.
 * @param info The client information structure.
 * @param petition The message recieved from client
 */
void dispatch_petition(struct client *info, char *petition) {
   long int codigo_mensaje;
   char output_buffer[MAX_MESSAGE];

   if (info->name[0]=='\0'){
      strncpy(info->name, petition, 256);
   }
   if (sscanf(petition,"%d",(int *)&codigo_mensaje)==EOF){
      printf("No entiendo el mensaje (%s) del cliente (%s)\n",
             petition,info->name);
      return;
   }
   if (codigo_mensaje==NETWORKSERVER_rgb24bpp_sifntsc_image_query){
      /*El cliente pide una imagen*/
      int cam;
      char *image=NULL;
      unsigned long int *clock;
      unsigned long int time;

      if (sscanf(petition,"%ld %d",&codigo_mensaje,&cam)!=2){
         return;
      }
      /*Comprobar si el código de cámara está servido*/
      if (serve_device[COLORA_DEVICE]==1 && device_network_id[COLORA_DEVICE]==cam){
         image=*(char **)variables[COLORA_DEVICE];
         clock=(unsigned long int *)myimport("colorA", "clock");
         while (info->clocks[COLORA_DEVICE]==*clock){
            usleep(1000);/*Espero una nueva imagen durante 1ms*/
         }
         info->clocks[COLORA_DEVICE]=*clock;
         time=info->clocks[COLORA_DEVICE];
      }
      else if (serve_device[COLORB_DEVICE]==1 && device_network_id[COLORB_DEVICE]==cam){
         image=*(char **)variables[COLORB_DEVICE];
         clock=(unsigned long int *)myimport("colorB", "clock");
         while (info->clocks[COLORB_DEVICE]==*clock){
            usleep(1000);/*Espero una nueva imagen durante 1ms*/
         }
         info->clocks[COLORB_DEVICE]=*clock;
         time=info->clocks[COLORB_DEVICE];
      }
      else if (serve_device[COLORC_DEVICE]==1 && device_network_id[COLORC_DEVICE]==cam){
         image=*(char **)variables[COLORC_DEVICE];
         clock=(unsigned long int *)myimport("colorC", "clock");
         while (info->clocks[COLORC_DEVICE]==*clock){
            usleep(1000);/*Espero una nueva imagen durante 1ms*/
         }
         info->clocks[COLORC_DEVICE]=*clock;
         time=info->clocks[COLORC_DEVICE];
      }
      else if (serve_device[COLORD_DEVICE]==1 && device_network_id[COLORD_DEVICE]==cam){
         image=*(char **)variables[COLORD_DEVICE];
         clock=(unsigned long int *)myimport("colorD", "clock");
         while (info->clocks[COLORD_DEVICE]==*clock){
            usleep(1000);/*Espero una nueva imagen durante 1ms*/
         }
         info->clocks[COLORD_DEVICE]=*clock;
         time=info->clocks[COLORD_DEVICE];
      }

      if (image!=NULL)
      {
         char myimage[SIFNTSC_COLUMNS*SIFNTSC_ROWS*CANALES];

         /*Hago una copia local de la imagen para evitar posibles cambios en
         ella durante la escritura*/
         memcpy(myimage, image, SIFNTSC_COLUMNS*SIFNTSC_ROWS*CANALES);
         sprintf(output_buffer,"%d %lu %d %d %d %d\n",
                 NETWORKSERVER_rgb24bpp_sifntsc_image, time, cam, SIFNTSC_COLUMNS,
                 SIFNTSC_ROWS,CANALES);
         my_write(info->cs,output_buffer,strlen(output_buffer));
         my_write(info->cs,myimage, SIFNTSC_COLUMNS*SIFNTSC_ROWS*CANALES);
      }
   }


   else if (codigo_mensaje==NETWORKSERVER_subscribe_laser){
      /*Subscripción al laser*/
      if (serve_device[LASER_DEVICE]==1)
         info->subscriptions[LASER_DEVICE]=1;
   }
   else if (codigo_mensaje==NETWORKSERVER_unsubscribe_laser){
      /*Subscripción al laser*/
      info->subscriptions[LASER_DEVICE]=0;
   }


   else if (codigo_mensaje==NETWORKSERVER_subscribe_encoders){
      /*Subscripción a los encoders*/
      if (serve_device[ENCODERS_DEVICE]==1)
         info->subscriptions[ENCODERS_DEVICE]=1;
   }
   else if (codigo_mensaje==NETWORKSERVER_unsubscribe_encoders){
      /*Subscripción a los encoders*/
      info->subscriptions[ENCODERS_DEVICE]=0;
   }


   else if (codigo_mensaje==NETWORKSERVER_subscribe_us){
      /*Subscripción a los sonars*/
      if (serve_device[SONARS_DEVICE]==1)
         info->subscriptions[SONARS_DEVICE]=1;
   }
   else if (codigo_mensaje==NETWORKSERVER_unsubscribe_us){
      /*Subscripción a los sonars*/
      info->subscriptions[SONARS_DEVICE]=0;
   }


   else if (codigo_mensaje==NETWORKSERVER_drive_speed){
      /*Cambio de la velocidad lineal*/
      float vel, ac;
      if (serve_device[MOTORS_DEVICE]==1){
         if (sscanf(petition,"%ld %f %f",&codigo_mensaje,&vel,&ac)!=3)
            printf ("No entiendo el mensaje de cambio de velocidad.\n");
         else
            *(((float **)(variables[MOTORS_DEVICE]))[0])=vel;
      }
   }
   else if (codigo_mensaje==NETWORKSERVER_steer_speed){
      /*Cambio de la velocidad de giro*/
      float velw, ac;
      if (serve_device[MOTORS_DEVICE]==1){
         if (sscanf(petition,"%ld %f %f",&codigo_mensaje,&velw,&ac)!=3)
            printf ("No entiendo el mensaje de cambio de velocidad de giro.\n");
         else
            *(((float **)(variables[MOTORS_DEVICE]))[1])=velw;
      }
   }


   else if (codigo_mensaje==NETWORKSERVER_subscribe_pantilt_encoders){
      if (serve_device[PANTILT_ENCODERS_DEVICE])
          info->subscriptions[PANTILT_ENCODERS_DEVICE]=1;
   }
   else if (codigo_mensaje==NETWORKSERVER_unsubscribe_pantilt_encoders){
      info->subscriptions[PANTILT_ENCODERS_DEVICE]=0;
   }


   else if (codigo_mensaje==NETWORKSERVER_pantilt_position){
      float lat, longt, lat_sp, longt_sp;
      if (serve_device[PANTILT_MOTORS_DEVICE]==1){
         if (sscanf(petition,"%ld %f %f %f %f",&codigo_mensaje,&longt, &lat,
             &longt_sp, &lat_sp)!=5)
            printf ("No entiendo el mensaje de motores del pantilt.\n");
         else{
            *(((float **)(variables[PANTILT_MOTORS_DEVICE]))[0])=longt;
            *(((float **)(variables[PANTILT_MOTORS_DEVICE]))[1])=lat;
            *(((float **)(variables[PANTILT_MOTORS_DEVICE]))[2])=longt_sp;
            *(((float **)(variables[PANTILT_MOTORS_DEVICE]))[4])=lat_sp;
         }
      }
   }


   else if (codigo_mensaje==NETWORKSERVER_goodbye ||
            codigo_mensaje==NETWORKSERVER_goodbye_images)
   {
      close(info->cs);
      printf("Cierro cliente %s, por su mensaje de goodbye\n",info->name);
      pthread_exit(0);
   }
   else
      printf("Mensaje NO reconocido del cliente %s: (%s)\n", info->name,petition);
}

/**
 * @brief Dispatch all client subscriptions.
 * It checks all the subscriptions and if the subscription is active, the
 * appropriate information will be sent through the client socket.
 * @param info The client information structure.
 */
void dispatch_subscriptions(struct client * info) {
   int i, j;
   char buff[MAX_MESSAGE];
   
   for (i = 0; i < MAXDEVICE; i++) {
      if (info->subscriptions[i]) {
         switch(i) {
            case LASER_DEVICE:
            {
               unsigned long int *clock;
               clock=(unsigned long int *)myimport("laser", "clock");
               if (info->clocks[LASER_DEVICE]!=*clock){
                  info->clocks[LASER_DEVICE]=*clock;
                  /*Componer el mensaje*/
                  sprintf(buff,"%d %lu",NETWORKSERVER_laser,
                          info->clocks[LASER_DEVICE]);
                  variables[i]=myimport("laser", "laser");
                  for (j=0;j<NUM_LASER; j++){
                     sprintf(buff+strlen(buff)," %d",
                             (int)(((int *)variables[i])[j]));
                  }
                  sprintf(buff+strlen(buff),"\n");
                  /*Envío del mensaje*/
                  my_write(info->cs,buff,strlen(buff));
               }
               break;
            }
            case ENCODERS_DEVICE:
            {
               unsigned long int *clock;
               clock=(unsigned long int *)myimport("encoders", "clock");
               if (info->clocks[ENCODERS_DEVICE]!=*clock){
                  info->clocks[ENCODERS_DEVICE]=*clock;
                  /*Composición del mensaje*/
                  sprintf(buff, "%d %lu", NETWORKSERVER_encoders,
                          info->clocks[ENCODERS_DEVICE]);
                  sprintf(buff+strlen(buff)," %1.1f %1.1f %1.5f %lu\n",
                          (float)((float *)variables[i])[0],
                          (float)((float *)variables[i])[1],
                          (float)((float *)variables[i])[2] * RADTODEG,
                          info->clocks[ENCODERS_DEVICE]);
                  /*Envío del mensaje*/
                  my_write(info->cs, buff, strlen(buff));
               }
               break;
            }
            case SONARS_DEVICE:
            {
               unsigned long int *clock;
               clock=(unsigned long int *)myimport("sonars", "clock");
               //printf ("%ld, %lu, %lu", info->clocks[SONARS_DEVICE], clock, clock[0]);
               if (info->clocks[SONARS_DEVICE]!=clock[0]){
                  info->clocks[SONARS_DEVICE]=clock[0];
                  /*Componer el mensaje*/
                  sprintf(buff,"%d %lu",NETWORKSERVER_sonars,
                          info->clocks[SONARS_DEVICE]);
                  variables[i]=myimport("sonars", "us");
                  for (j=0;j<NUM_SONARS; j++){
                     sprintf(buff+strlen(buff)," %d %1.1f", j,
                             (float)(((float *)variables[i])[j]));
                  }
                  sprintf(buff+strlen(buff),"\n");
                  /*Envío del mensaje*/
                  my_write(info->cs,buff,strlen(buff));
               }
               break;
            }
            case PANTILT_ENCODERS_DEVICE:
            {
               unsigned long int *clock;
               clock=(unsigned long int *)myimport("ptencoders", "clock");
               if (info->clocks[PANTILT_ENCODERS_DEVICE]!=*clock){
                  info->clocks[PANTILT_ENCODERS_DEVICE]=*clock;
                  /*Composición del mensaje*/
                  sprintf(buff, "%d %lu", NETWORKSERVER_pantilt_encoders,
                          info->clocks[ENCODERS_DEVICE]);
                  sprintf(buff+strlen(buff)," %1.1f %1.1f\n",
                          (float)((float *)variables[i])[0],
                           (float)((float *)variables[i])[1]);
                  /*Envía tipo, hora, pan, tilt*/
                  /*Envío del mensaje*/
                  my_write(info->cs, buff, strlen(buff));
               }
               break;
            }
         }
      }
   }
}

/**
 * @brief The client thread
 * It will recieve all the petitions from the client and will dispatch it. It will
 * attend to the subscription too.
 * @param pcs The client socket that will be stored in the client structure.
 */
void *client_thread(void *pcs) {
   char buf1[MAX_MESSAGE];
   char buf2[MAX_MESSAGE];
   char *buffer = buf1;
   char *buffer_tmp = buf2;
   char *buffer_aux;
   char *msg, *msg_end;
   int bytes_read;
   int nbytes = 0; // number of valid bytes in buffer
   int continuar;

   struct timeval a, b;
   long n=0; /* iteration */
   long next,bb,aa;

   struct client info;
   info.cs = *(int *)pcs;
   pthread_mutex_unlock(&socketmutex);
   memset(info.subscriptions, 0, MAXDEVICE);

   if ( fcntl(info.cs, F_SETFL, O_NONBLOCK) ) {
         pthread_exit(0);
   }

   /*Leer el nombre*/
   do{
      bytes_read = read(info.cs, buffer + nbytes, MAX_MESSAGE - nbytes);
      if (bytes_read < 0) {
         if (errno == EAGAIN) {
            continuar=1;
         }
         else{
            pthread_exit(0);
         }
      }
      else{
         continuar=0;
      }
   }while (continuar);
   
   strncpy (info.name, buffer, 256);
   printf ("Nuevo cliente: %s.\n", info.name);
   
   gettimeofday(&a, NULL);
   aa=a.tv_sec*1000000+a.tv_usec;
   n=0;

   for (;;) {
      n++;
      bytes_read = read(info.cs, buffer + nbytes, MAX_MESSAGE - nbytes);
      if (bytes_read < 0) {
         if (errno != EAGAIN) {
            pthread_exit(0);
         }
      }
      else if (bytes_read == 0) {
         pthread_exit(0);
      }
      else {
         nbytes += bytes_read;
      }

      if (nbytes > 0) {
         // Possible message/s to dispatch
         buffer[nbytes] = '\0';
         msg = buffer;
         msg_end = strstr(buffer, "\n");
         while (msg_end != NULL) { // For each message in buffer
            *msg_end = '\0';
            msg_end++; // msg_end now points to the folling messages
            nbytes -= msg_end - msg;/* the spare bytes in buffer for the
               following messages */
            dispatch_petition(&info, msg);
            /*printf (msg);*/
            msg = msg_end;
            msg_end = strstr(msg, "\n");
         }
         memcpy(buffer_tmp, msg, nbytes); /* save those bytes of the last
            message, which is incomplete */
         buffer_aux = buffer;
         buffer = buffer_tmp;
         buffer_tmp = buffer_aux;
      }

      dispatch_subscriptions(&info);

      gettimeofday(&b,NULL);
      bb=b.tv_sec*1000000+b.tv_usec;
      next=aa+(n+1)*(long)CLIENT_TH_CYCLE*1000-bb;
      if (next>5000) {
         usleep(next-5000);
         /* discounts 5ms taken by calling usleep itself, on average */
      }
   }
   pthread_exit(0);
}

/**
 * @brief The main thread
 * It's the main driver thread. It will initialize the socket and will accept
 * client connections, then will create a @ref client_thread for each client.
 */
void *listen_thread() {
   int s; /* FD socket escucha */
   int cs; /* FD socket despacho cliente */
   struct sockaddr_in addr_control, addr_remoto;
   socklen_t length_addr_remoto;
   pthread_t pid;

   bootstrap(&s, &addr_control, networkserver_port);
   for (;;) {
      memset(&addr_remoto, 0, sizeof(addr_remoto));
      length_addr_remoto = sizeof(addr_remoto);
      pthread_mutex_lock(&socketmutex);
      cs = accept(s, (struct sockaddr *) &addr_remoto, &length_addr_remoto);
      if (cs < 0) {
         perror("");
         jdeshutdown(0);
      }
      pthread_create(&pid,NULL,client_thread,(void*)&cs);
   }
}

/**
 * The startup function following the drivers jde api.
 * @param configfile path and name to the config file of this driver.
 */
void networkserver_startup(char *configfile)
{
   int i;

   /* reseting serve color array and setting default options */
   for(i=0;i<MAXDEVICE;i++){
      serve_device[i]=0;
      device_network_id[i]=0;
      variables[i]=NULL;
   }

   /* we call the function to parse the config file */
   if(networkserver_parseconf(configfile)==-1){
      printf("networkserver: cannot initiate driver. configfile parsing error.\n");
      exit(-1);
   }

   /*To import variables and resume schemas that export them*/
   init();
   pthread_create(&listen_pid,NULL,listen_thread,(void*)NULL);
   printf("networkserver driver started up\n");
}
