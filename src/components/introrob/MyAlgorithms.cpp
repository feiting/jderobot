/*
 *  Copyright (C) 1997-2011 JDERobot Developers Team
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
 *  Authors : Maikel González <m.gonzalezbai@gmail.com>,
 *
 */

#include "API.h"

/*
INTROROB API:
---------------------------------------------------------
Métodos para obtener los datos de los sensores:

this->getMotorV: Este método devuelve la velocidad lineal (mm./s.) del robot. 
EJEMPLO: float v = this->getMotorV();

this->getMotorW: Este método devuelve la velocidad rotacional (deg./s.) del robot. 
EJEMPLO: float w = this->getMotorW();

this->getLaserData: Este método devuelve una estructura con dos campos, por un lado el número de lasers del robot (180) y por otro un vector de 180 posiciones en cada una de las cuales se almacena la distancia obtenida por el láser, estando relacionada cada posición del robot con el ángulo del láser. Ejemplo: jderobot::LaserDataPtr laser = this->getLaserData() || 
USO: printf("laser[45]: %d\n", laser->distanceData[45]); || 
USO: printf("numLasers: %d\n", laser->numLaser);

this->getDistancesLaser: Este método devolvería únicamente el vector mencionado anteriormente. 
EJEMPLO: jderobot::IntSeq VectorDistances this->getDistancesLaser();

this->getNumLasers: Este método devuelve el otro campo mencionado, el número de grados del láser. 
EJEMPLO: int numLasers = this->getNumLasers();

this->getEncodersData: Este método devuelve una estructura con tres campos: robotx, roboty y robottheta, siendo la posición "x", "y" y su orientación "theta" respectivamente. 
EJEMPLO: jderobot::EncodersDataPtr myPosition getEncodersData(); || 
USO: printf("myPosition = [%f, %f]\n", myPosition->robotx, myPosition->roboty);

this->getImageCamera1: Este método devuelve la imagen obtenida por la cámara izquierda del Pioneer. La imagen ya se encuentra en formato "colorspace" con lo que es posible de manipular por medio de OpenCV.
EJEMPLO: colorspaces::Image* imageIzq this->getImageCamera1();

NOTA: Antes de usar getImageCameraX hay que llamar al método imageCameras2openCV, la cual obtiene las imágentes en formato "colorspace".

this->getImageCamera2: Idéntica al método anterior pero con la imagen derecha.

---------------------------------------------------------

Métodos para manipular los actuadores:

this->setMotorV(float V): Con este método definimos la velocidad lineal (mm./s.) del robot.
EJEMPLO: this->setMotorV(40.)

this->w=float W): Con este método definimos la velocidad rotacional (deg./s.) del robot.
EJEMPLO: this->setMotorW(10.)



---------------------------------------------------------

Métodos gráficos (mundo 3D)

pintaSegmento: Traza una línea entre dos puntos dados a partir de un color dado.
USO:
      CvPoint3D32f aa,bb;
      CvPoint3D32f color;

      bb.x=this->destino.x;
      bb.y=this->destino.y;
      bb.z=0.;
      
      aa.x=encodersData->robotx;
      aa.y=encodersData->roboty;
      aa.z=0;
      
      color.x = 1.; // Red
      color.y = 0.; // Green
      color.z = 0.; // Blue
      this->pintaSegmento (aa, bb, color); 

drawProjectionLines: Traza líneas desde el origen de coordenadas a un punto seleccionado (click izquierdo) en una de las cámaras del robot.
USO: this->drawProjectionLines();

---------------------------------------------------------

Otros:

destino: Variable que almacena las coordenadas de la pocición seleccionada en el mundo 3D con el botón central del ratón.
EJEMPLO: printf ("destPoint = [%f, %f]\n", this->destino.x, this->destino.y);

absolutas2relativas: Método que calcula la posicion relativa respecto del robot de un punto absoluto. El robot se encuentra en robotx, roboty con orientacion robotheta respecto al sistema de referencia absoluto.

relativas2absolutas: Método que calcula la posicion absoluta de un punto expresado en el sistema de coordenadas solidario al robot. El robot se encuentra en robotx, roboty con orientacion robotheta respecto al sistema de referencia absoluto.
USO:
        CvPoint3D32f aa,a,b;

	aa.x=0.; aa.y=0.;
	this->relativas2absolutas(aa,&a);
	aa.x = 1000.; aa.y = -2000.;  // en mm.	     		
	this->relativas2absolutas(aa,&b);

	**** PARA MAS INFO ACCEDER AL FICHERO API.CPP y API.H ****

*/

namespace introrob{
    void Api::RunNavigationAlgorithm(){
      double v, w, l, pan, tilt;
      jderobot::LaserDataPtr laser;
      CvPoint2D32f dest;
      jderobot::EncodersDataPtr encoders;
      
 
	/*A PARTIR DE AQUÍ SE PUEDE AÑADIR EL CÓDIGO DE NAVEGACIÓN IMPLEMENTADO POR EL ESTUDIANTE*/

	/*ALGUNOS EJEMPLOS*/      
      imageCameras2openCV();
      imageCamera1=getImageCamera1();
      imageCamera2=getImageCamera2();
      printf("myPosition = [%f, %f]\n", encodersData->robotx, encodersData->roboty);

	/* EJEMPLO SENCILLO DE UN BUMP AND GO */

	/* TOMA DE SENSORES */
	//Aqui tomamos el valor de los sensores para alojarlo en nuestras variables locales
      laser=getLaserData(); // Get the laser info
      printf("laser[45]: %d\n", laser->distanceData[45]);
      
      v=this->getMotorV();
      w = this->getMotorW();
      l = this->getMotorL();
      printf("v: %f , w: %f , l: %f\n", v, w, l);
      
      dest=this->getDestino();
      printf ("destPoint = [%f, %f]\n", dest.x, dest.y);
      
      encoders=this->getEncodersData();
      printf("myPosition = [%f, %f]\n", encoders->robotx, encoders->roboty);
	/* FIN TOMA DE SENSORES */
	    
	    v=0;
	    w=100;
	    //v=2000;

	
      
      
      /*
      switch(accion){
              
              case 0:		// Robot hacia adelante
                      
                      if(( laser->distanceData[45] < 1000.0) or ( laser->distanceData[90] < 1000.0) or ( laser->distanceData[135] < 1000.0)){
                              v=0.;
                              //if ((x_ant == myPoint.x) and (y_ant == myPoint.y) and (z_ant == myPoint.z)){
                              accion=1;
                              printf("### Activado hacia Atras\n");
                              //}
                      }
                      else
                        v=100;
                      break;

              case 1:		// Robot hacia atras
                      if ((laser->distanceData[45] < 1100) or (laser->distanceData[90] < 1100) or (laser->distanceData[135] < 1100)){
                              v=-50.;
                              printf("### Llendo hacia atras\n");
                      }
                      else{
                              v=0.;
                              accion=2;
                      }
                      break;


              case 2:		// Robot girando.
                      if((laser->distanceData[45] < 1300) or (laser->distanceData[90] < 1300) or (laser->distanceData[135] < 1300)){
                              if(sentido%2==0){
                                      w=50.;
                              }
                              else{
                                      w=50.*(-1);
                              }
                              printf("### Girando: %d \n", sentido);
                      }
                      else{
                              w=0.;
                              accion=0;
                              sentido = (1 + rand() % 40);
                      }			

                      break;
      }
      */
      /*Comandar robot*/
      //Aqui enviamos los datos al robot
      this->setMotorV(v);
      this->setMotorW(w);
      this->setMotorL(l);
      //pan=0;
      //tilt=0;
      //this->setPTEncoders(pan,tilt,1); //Con el segundo parametro elegimos la camara a comandar (1 o 2)
      

      
    }

   void Api::RunGraphicsAlgorithm(){
      /* TODO: ADD YOUR GRAPHIC CODE HERE */
      CvPoint3D32f aa,bb;
      CvPoint3D32f a,b;
      CvPoint3D32f c,d;
      CvPoint3D32f color;
      
      bb.x=this->destino.x;
      bb.y=this->destino.y;
      bb.z=0.;
      
      aa.x=encodersData->robotx;
      aa.y=encodersData->roboty;
      aa.z=0;
      
      color.x = 1.; // Red
      color.y = 0.; // Green
      color.z = 0.; // Blue
      this->pintaSegmento (aa, bb, color); // ROJO
      
      
      this->drawProjectionLines();
      
   }

}
