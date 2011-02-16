/*
*  Copyright (C) 1997-2010 JDERobot Developers Team
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
 *   Authors : Eduardo Perdices <eperdices@gsyc.es>,
 *             Jose María Cañas Plaza <jmplaza@gsyc.es>
 *
 */

#ifndef MONOSLAM_DRAWAREA_H
#define MONOSLAM_DRAWAREA_H

#include <string>
#include <iostream>
#include <pthread.h>
#include <gtkmm.h>
#include <gtkglmm.h>
#include <gdkglmm.h>
#include <libglademm.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <progeo/progeo.h>
#include <jderobot/motors.h>
#include <jderobot/encoders.h>
#include <jderobot/laser.h>
#include <jderobot/sonars.h>
#include "pioneer.h"
#include "pioneeropengl.h"
#define v3f glVertex3f
#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240

typedef struct SoRtype {
  struct SoRtype *father;
  float posx;
  float posy;
  float posz;
  float foax;
  float foay;
  float foaz;
  float roll;
} SofReference;

namespace teleoperator {
	class DrawArea : public Gtk::DrawingArea, public Gtk::GL::Widget<DrawArea>
	{
	public:
		DrawArea(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& builder);
		virtual ~DrawArea();

		float robotx;
		float roboty;
		float robottheta;

		int numSonars;
		std::vector <float> us;

		int numLasers;
		std::vector <float> distanceData;

		static const float MAXWORLD;
		static const float PI;

	protected:
		/*Override default signal handler*/
		virtual bool on_expose_event(GdkEventExpose* event);
		virtual bool on_motion_notify(GdkEventMotion* event);
		virtual bool on_drawarea_scroll(GdkEventScroll * event);

		bool on_timeout();

		void drawScene();
		void draw_world();
		void InitOGL (int w, int h);

		SofReference virtualcam, mypioneer;
		int refresh_time;
		HPoint3D glcam_pos;
		HPoint3D glcam_foa;
		TPinHoleCamera camera;
		HPoint3D cam_pos;

    float scale;
    float radius;
    float lati;
    float longi;
    float old_x;
    float old_y; 
	};
} /*namespace*/

#endif /*MONOSLAM_DRAWAREA_H*/
