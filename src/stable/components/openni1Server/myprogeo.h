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
 *   Authors : Francisco Miguel Rivas Montero <franciscomiguel.rivas@urjc.es>
 *             Jose María Cañas Plaza <jmplaza@gsyc.es>
 *
 */

#ifndef OPENNISERVER_MYPROGEO_H
#define OPENNISERVER_MYPROGEO_H

#include <progeo/progeo.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_CAMERAS 8
#define MAX_BUFFER 1024

namespace openni1Server {
class myprogeo {
public:
    myprogeo();
    ~myprogeo();
    int load_cam_line(FILE *myfile,int cam);
    void load_cam(char *fich_in,int cam, int w, int h);
    void mybackproject(float x, float y, float* xp, float* yp, float* zp, float* camx, float* camy, float* camz, int cam);
    void myproject(float x, float y, float z, float* xp, float* yp, int cam);
    void mygetcameraposition(float *x, float *y, float *z, int cam);
    void mygetcamerafoa(float *x, float *y, float *z, int cam);
    void mygetcamerasize(float *w, float *h, int cam);
    TPinHoleCamera getCamera(int camera);

private:
    /* cameras */
	void pixel2optical(float*x,float*y);
	void optical2pixel(float* x, float* y); 
    TPinHoleCamera cameras[MAX_CAMERAS];
	int w,h;
};
} // namespace

#endif /*KINECTVIEWER_MYPROGEO_H*/
