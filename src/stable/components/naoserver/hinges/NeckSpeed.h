/*
 *  Copyright (C) 1997-2013 JDERobot Developers Team
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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors : Borja Menéndez <borjamonserrano@gmail.com>
 *
 */

#ifndef NECKSPEED_H
#define NECKSPEED_H

#include <iostream>
#include <stdio.h>
#include <cmath>

#include "alcore/alptr.h"
#include "alproxies/alledsproxy.h"
#include "alproxies/almemoryproxy.h"
#include "alproxies/alsensorsproxy.h"
#include "alproxies/alsonarproxy.h"
#include "alproxies/alrobotposeproxy.h"
#include "alproxies/almotionproxy.h"
#include "alcommon/alproxy.h"
#include "alcommon/albroker.h"
#include "alcommon/almodule.h"

#include <IceE/IceE.h>
#include <pose3dmotors.h>

#include "Singleton.h"
#include "Common.h"

class NeckSpeed : public Singleton<NeckSpeed>, public jderobot::Pose3DMotors {
public:
    // Constructor
    NeckSpeed ();
        
    // Destructor
    virtual ~NeckSpeed ();
    
    // Another functions
    void init ( const std::string newName, AL::ALPtr<AL::ALBroker> parentBroker, float stiffness, float speed );
        
    /*Pose3DMotors*/
    Ice::Int setPose3DMotorsData ( const jderobot::Pose3DMotorsDataPtr & data, const Ice::Current& );
    jderobot::Pose3DMotorsDataPtr getPose3DMotorsData ( const Ice::Current& );
    jderobot::Pose3DMotorsParamsPtr getPose3DMotorsParams ( const Ice::Current& );

private:
    std::string name;
    float stiffness, speed;
    AL::ALValue jointPitch, jointYaw;
    AL::ALPtr<AL::ALMotionProxy> motion;
    
};
#endif // NECKSPEED_H
