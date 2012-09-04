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
 *                 Jose María Cañas Plaza <jmplaza@gsyc.es>
 */

#ifndef WIIMOTECLIENT_API_H
#define WIIMOTECLIENT_API_H

namespace wiimoteClient{
    
    class Api{
        
    public:
        
        virtual ~Api();
        
        //Shared Memory
        int acc[3];
        int button;
        int ir1[2];
        int ir2[2];
        int ir3[2];
        int nunchukAcc[3];
        int nunchukStick[2];
        int nunchukButton;
        bool guiVisible;
        
    };//class
    
    
}//end namespace
#endif //WIIMOTECLIENT_API_H
