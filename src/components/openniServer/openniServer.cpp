/*
 *  Copyright (C) 1997-2012 JDE Developers Teameldercare.camRGB
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 *  Authors : Jose María Cañas <jmplaza@gsyc.es>
			Francisco Miguel Rivas Montero <franciscomiguel.rivas@urjc.es>
			
 */

/** \file openniServer.cpp
 * \brief openniServer component main file
 */

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <gbxsickacfr/gbxiceutilacfr/safethread.h>
#include <jderobot/kinectleds.h>
#include <jderobot/camera.h>
#include <jderobot/pose3dmotors.h>
#include <jderobot/remoteCloud.h>
#include <jderobot/remoteConfig.h>
#include <colorspaces/colorspacesmm.h>
#include <jderobotice/component.h>
#include <jderobotice/application.h>
#include <tr1/memory>
#include <list>
#include <sstream>
#include <jderobotice/exceptions.h>
#include <math.h>
//#include <controller.h>
#include <cv.h>
#include <highgui.h>
#include <XnOS.h>
#include <XnCppWrapper.h>
#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnPropNames.h>
#include <XnUSB.h>
#include <XnLog.h>
#include <XnVCircleDetector.h>
#include <XnFPSCalculator.h>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <libusb-1.0/libusb.h>
#include "myprogeo.h"
#include <iostream>
#include <fstream>


#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0
#define NUM_THREADS 5

#define CHECK_RC(rc, what)                                      \
if (rc != XN_STATUS_OK)                                         \
{                                                               \
    printf("%s failed: %s\n", what, xnGetStatusString(rc));     \
                                                    \
}

#define MAX_DEVICES 5
#define MAX_CAMERAS_SERVER 2




using namespace xn;

namespace openniServer{

/*user tracker*/
xn::Player g_Player;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
XnBool g_bDrawBackground = TRUE;
XnBool g_bDrawPixels = TRUE;
XnBool g_bDrawSkeleton = TRUE;
XnBool g_bPrintID = TRUE;
XnBool g_bPrintState = TRUE;

XnStatus rc=XN_STATUS_OK;;
xn::Context g_context;	
pthread_mutex_t mutex;
int n_devices=0;
int SELCAM;
std::vector<int> distances;
IplImage* srcRGB=NULL;
int colors[10][3];
int userGeneratorActive=0;
int width;
int height;


/*OJO solo funciona con imágenes de 640x480, no con imágenes redimensionadas, si valdría con tamaños fijados con configuración openni, pero no hemos conseguido que funcione variar la resolución por configuración*/
std::vector<int> pixelsID;
//int pixelsID[640*480];


struct KinectDevice
{
        char name[80];
        xn::ProductionNode device;
        xn::DepthGenerator depth;
        xn::DepthMetaData depthMD;
        xn::ImageGenerator image;
        xn::ImageMetaData imageMD;
	xn::SceneMetaData sceneMD;
	xn::UserGenerator g_UserGenerator;
	std::string creation_info;
        std::string camera_type;
        std::string serial;
        std::string vendor;
        unsigned short vendor_id;
        unsigned short product_id;
        unsigned char bus;
        unsigned char address;
};

KinectDevice sensors[MAX_DEVICES];
//std::vector<KinectDevice> sensors;


void UpdateCommon(KinectDevice &sensor)
{
        XnStatus rc = XN_STATUS_OK;
        rc = g_context.WaitAnyUpdateAll();
        CHECK_RC(rc, "WaitAnyUpdateAll() failed");

        if (sensor.depth.IsValid())
        {
                sensor.depth.GetMetaData(sensor.depthMD);
        }
	   if(sensor.g_UserGenerator.IsValid()){
            	sensor.g_UserGenerator.GetUserPixels(0,sensor.sceneMD);
		}
        if (sensor.image.IsValid())
        {
                sensor.image.GetMetaData(sensor.imageMD);
        }
}


void* kinectThread(void*)
{
	while(1){
		pthread_mutex_lock(&mutex);
			UpdateCommon(sensors[SELCAM]);
		pthread_mutex_unlock(&mutex);
      usleep(10);

   }
   
}


// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d New User %d\n", epochTime, nId);
	// New user found
	if (g_bNeedPose)
	{
		sensors[SELCAM].g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
	}
	else
	{
		sensors[SELCAM].g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Lost user %d\n", epochTime, nId);	
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
	sensors[SELCAM].g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	sensors[SELCAM].g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Calibration started for user %d\n", epochTime, nId);
}
// Callback: Finished calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
		// Calibration succeeded
		printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);		
		sensors[SELCAM].g_UserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		// Calibration failed
		printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==9)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
		if (g_bNeedPose)
		{
			sensors[SELCAM].g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
		}
		else
		{
			sensors[SELCAM].g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

std::map<XnUInt32, std::pair<XnCalibrationStatus, XnPoseDetectionStatus> > m_Errors;
void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& /*capability*/, XnUserID id, XnCalibrationStatus calibrationError, void* /*pCookie*/)
{
	m_Errors[id].first = calibrationError;
}
void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& /*capability*/, const XnChar* /*strPose*/, XnUserID id, XnPoseDetectionStatus poseError, void* /*pCookie*/)
{
	m_Errors[id].second = poseError;
}



//test function to draw joint on image
void DrawJoint(XnUserID player, XnSkeletonJoint eJoint)
{
	if (!sensors[SELCAM].g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		printf("not tracked!\n");
		return;
	}

	if (!sensors[SELCAM].g_UserGenerator.GetSkeletonCap().IsJointActive(eJoint))
	{
		return;
	}

	XnSkeletonJointPosition joint;
	sensors[SELCAM].g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);

	if (joint.fConfidence < 0.5)
	{
		return;
	}

	XnPoint3D pt;
	pt = joint.position;

	sensors[SELCAM].depth.ConvertRealWorldToProjective(1, &pt, &pt);

	//drawCircle(pt.X, pt.Y, 2);
	int x= pt.X;
	int y= pt.Y;
	srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 0] = 255;
	srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 1] = 0;
	srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 2] = 0;
}


void CalculateJoints(){
	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;
	sensors[SELCAM].g_UserGenerator.GetUsers(aUsers, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		// Draw Joints
		//if (g_bMarkJoints)
		if (sensors[SELCAM].g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
		{
			// Try to draw all joints
			DrawJoint(aUsers[i], XN_SKEL_HEAD);
			DrawJoint(aUsers[i], XN_SKEL_NECK);
			DrawJoint(aUsers[i], XN_SKEL_TORSO);
			DrawJoint(aUsers[i], XN_SKEL_WAIST);

			DrawJoint(aUsers[i], XN_SKEL_LEFT_COLLAR);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_SHOULDER);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_ELBOW);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_WRIST);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_HAND);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_FINGERTIP);

			DrawJoint(aUsers[i], XN_SKEL_RIGHT_COLLAR);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_SHOULDER);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_ELBOW);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_WRIST);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_HAND);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_FINGERTIP);

			DrawJoint(aUsers[i], XN_SKEL_LEFT_HIP);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_KNEE);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_ANKLE);
			DrawJoint(aUsers[i], XN_SKEL_LEFT_FOOT);

			DrawJoint(aUsers[i], XN_SKEL_RIGHT_HIP);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_KNEE);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_ANKLE);
			DrawJoint(aUsers[i], XN_SKEL_RIGHT_FOOT);
		}
	}
}




/**
* \brief Class wich contains all the functions and variables to make run the Robot Cameras
*/
	class CameraRGB: virtual public jderobot::Camera {
public:
	CameraRGB(std::string& propertyPrefix, const jderobotice::Context& context)
      : prefix(propertyPrefix),context(context),
	imageFmt(),
	imageDescription(new jderobot::ImageDescription()),
	cameraDescription(new jderobot::CameraDescription()),
	replyTask()
	{
	Ice::PropertiesPtr prop = context.properties();

	//fill cameraDescription
	cameraDescription->name = prop->getProperty(prefix+"Name");
	if (cameraDescription->name.size() == 0)
	throw 
		jderobotice::ConfigFileException(ERROR_INFO,"Camera name not configured");

	cameraDescription->shortDescription = prop->getProperty(prefix+"ShortDescription");

	//fill imageDescription
	imageDescription->width = width;
	imageDescription->height = height;
	int playerdetection = prop->getPropertyAsIntWithDefault(prefix+"PlayerDetection",0);
	if (!(userGeneratorActive))
		playerdetection=0;
	int fps = prop->getPropertyAsIntWithDefault(prefix+"fps",5);
	//we use formats acording to colorspaces
	std::string fmtStr = prop->getPropertyWithDefault(prefix+"Format","YUY2");//default format YUY2
	imageFmt = colorspaces::Image::Format::searchFormat(fmtStr);
	if (!imageFmt)
	throw 
		jderobotice::ConfigFileException(ERROR_INFO, "Format " + fmtStr + " unknown");
	imageDescription->size = imageDescription->width * imageDescription->height * CV_ELEM_SIZE(imageFmt->cvType);
	imageDescription->format = imageFmt->name;

	context.tracer().info("Starting thread for camera: " + cameraDescription->name);
	replyTask = new ReplyTask(this, imageDescription->width, imageDescription->height,fps, playerdetection);

	replyTask->start();//my own thread
	}

	virtual ~CameraRGB(){
		context.tracer().info("Stopping and joining thread for camera: " + cameraDescription->name);
		gbxiceutilacfr::stopAndJoin(replyTask);
	}
    
	virtual jderobot::ImageDescriptionPtr getImageDescription(const Ice::Current& c){
		return imageDescription;
	}

	virtual jderobot::CameraDescriptionPtr getCameraDescription(const Ice::Current& c){
		return cameraDescription;
	}

	virtual void getImageData_async(const jderobot::AMD_ImageProvider_getImageDataPtr& cb,const Ice::Current& c){
		replyTask->pushJob(cb);
	}

	virtual std::string startCameraStreaming(const Ice::Current&){
		context.tracer().info("Should be made anything to start camera streaming: " + cameraDescription->name);
	}

	virtual void stopCameraStreaming(const Ice::Current&) {
		context.tracer().info("Should be made anything to stop camera streaming: " + cameraDescription->name);
	}

	virtual Ice::Int setCameraDescription(const jderobot::CameraDescriptionPtr&, const Ice::Current&){
		
	}

private:
	class ReplyTask: public gbxiceutilacfr::SafeThread{
	public:
		ReplyTask(CameraRGB* camera, int width, int height, int fps, int playerdetection)
	: gbxiceutilacfr::SafeThread(camera->context.tracer()), mycameravga(camera) {
		segmentation=playerdetection;
		this->fps=fps;
      }
		

	void pushJob(const jderobot::AMD_ImageProvider_getImageDataPtr& cb){
		IceUtil::Mutex::Lock sync(requestsMutex);
		requests.push_back(cb);
	}

    virtual void walk(){
		int h=sensors[SELCAM].imageMD.YRes();
		int w=sensors[SELCAM].imageMD.XRes();


		jderobot::ImageDataPtr reply(new jderobot::ImageData);
		reply->description = mycameravga->imageDescription;
		reply->pixelData.resize(mycameravga->imageDescription->width*mycameravga->imageDescription->height*3);
		rgb.resize(width*height*3);
		srcRGB = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		IplImage* dst_resize = cvCreateImage(cvSize(mycameravga->imageDescription->width,mycameravga->imageDescription->height), IPL_DEPTH_8U, 3);

		g_nTexMapX = (((unsigned short)(sensors[SELCAM].imageMD.FullXRes()-1) / 512) + 1) * 512;
		g_nTexMapY = (((unsigned short)(sensors[SELCAM].imageMD.FullYRes()-1) / 512) + 1) * 512;
		g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
		SceneMetaData smd;

		struct timeval a, b;
		int cycle; // duración del ciclo
		long totala;
		long totalpre=0;
		long diff;

		std::cout << "FPS: " << fps << std::endl;
		cycle=(float)(1/(float)fps)*1000000;
		
	
		while(!isStopping()){
			gettimeofday(&a,NULL);
			totala=a.tv_sec*1000000+a.tv_usec;
			pthread_mutex_lock(&mutex);
		    IceUtil::Time t = IceUtil::Time::now();
		    reply->timeStamp.seconds = (long)t.toSeconds();
		    reply->timeStamp.useconds = (long)t.toMicroSeconds() - reply->timeStamp.seconds*1000000;
			const XnRGB24Pixel* pImageRow = sensors[SELCAM].imageMD.RGB24Data();
			const XnLabel* pLabels = sensors[SELCAM].sceneMD.Data();
			XnRGB24Pixel* pTexRow = g_pTexMap;

			
	
			for (XnUInt y = 0; y < sensors[SELCAM].imageMD.YRes(); ++y)
			{
				const XnRGB24Pixel* pImage = pImageRow;
				XnRGB24Pixel* pTex = pTexRow;
	
				for (XnUInt x = 0; x < sensors[SELCAM].imageMD.XRes(); ++x, ++pImage, ++pTex)
				{
					if (segmentation){
						pixelsID[(y*sensors[SELCAM].imageMD.XRes() + x)]= *pLabels;
						if (*pLabels!=0)
		                {
		                    srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 0] = colors[*pLabels][0];
							srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 1] = colors[*pLabels][1];
							srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 2] = colors[*pLabels][2];
						}
						else{
							srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 0] = 0;
							srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 1] = 0;
							srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 2] = 0;
						}
						++pLabels;
					}
					else{
						srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 0] = pImage->nRed;
						srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 1] = pImage->nGreen;
						srcRGB->imageData[(y*sensors[SELCAM].imageMD.XRes() + x)*3 + 2] = pImage->nBlue;
					}
					if ((userGeneratorActive)&&(!(segmentation))){
						pixelsID[(y*sensors[SELCAM].imageMD.XRes() + x)]= *pLabels;
						++pLabels;
					}
				}
				pImageRow += sensors[SELCAM].imageMD.XRes();
				pTexRow += g_nTexMapX;
			}	

			//test
			CalculateJoints();

			
			//cvFlip(srcRGB, NULL, 1);

			if ((mycameravga->imageDescription->width != w) || (mycameravga->imageDescription->height != h)){
				cvResize(srcRGB,dst_resize);
				memcpy(&(reply->pixelData[0]),(unsigned char *) dst_resize->imageData,dst_resize->width*dst_resize->height * 3);
			}
			else{
				memcpy(&(reply->pixelData[0]),(unsigned char *) srcRGB->imageData,srcRGB->width*srcRGB->height * 3);
			}
		    {//critical region start
			IceUtil::Mutex::Lock sync(requestsMutex);
		    while(!requests.empty()){
				jderobot::AMD_ImageProvider_getImageDataPtr cb = requests.front();
				requests.pop_front();
				cb->ice_response(reply);
			}

			}//critical region end
			pthread_mutex_unlock(&mutex);
			if (totalpre !=0){
				if ((totala - totalpre) > cycle ){
					std::cout<<"-------- openniServer: WARNING- RGB timeout-" << std::endl; 
				}
				else{
					usleep(cycle - (totala - totalpre));
				}
			}
			/*if (totalpre !=0){
				std::cout << "rgb: " <<  1000000/(totala-totalpre) << std::endl;
			}*/
			totalpre=totala;
		}
	}
	
	CameraRGB* mycameravga;
	IceUtil::Mutex requestsMutex;
	std::list<jderobot::AMD_ImageProvider_getImageDataPtr> requests;
	
	std::vector<uint8_t> rgb;
	char *img; 
	XnRGB24Pixel* g_pTexMap;
	unsigned int g_nTexMapX,g_nTexMapY;
	int segmentation;
	int fps;
	
    };
    typedef IceUtil::Handle<ReplyTask> ReplyTaskPtr;


    std::string prefix;
	jderobotice::Context context;
    colorspaces::Image::FormatPtr imageFmt;
    jderobot::ImageDescriptionPtr imageDescription;
    jderobot::CameraDescriptionPtr cameraDescription;
    ReplyTaskPtr replyTask;


  };


//*********************************************************************
	class CameraDEPTH: virtual public jderobot::Camera {
public:
	CameraDEPTH(std::string& propertyPrefix, const jderobotice::Context& context)
      : prefix(propertyPrefix),context(context),
	imageFmt(),
	imageDescription(new jderobot::ImageDescription()),
	cameraDescription(new jderobot::CameraDescription()),
	replyTask()
	{
      
      
	Ice::PropertiesPtr prop = context.properties();

	//fill cameraDescription
	cameraDescription->name = prop->getProperty(prefix+"Name");
	if (cameraDescription->name.size() == 0)
	throw 
		jderobotice::ConfigFileException(ERROR_INFO,"Camera name not configured");

	cameraDescription->shortDescription = prop->getProperty(prefix+"ShortDescription");

	//fill imageDescription
	imageDescription->width = width;
	int playerdetection = prop->getPropertyAsIntWithDefault(prefix+"PlayerDetection",0);
	if (!(userGeneratorActive))
		playerdetection=0;
	imageDescription->height = height;
	int fps = prop->getPropertyAsIntWithDefault(prefix+"fps",5);
	//we use formats acording to colorspaces
	std::string fmtStr = prop->getPropertyWithDefault(prefix+"Format","YUY2");//default format YUY2
	imageFmt = colorspaces::Image::Format::searchFormat(fmtStr);
	if (!imageFmt)
	throw 
		jderobotice::ConfigFileException(ERROR_INFO, "Format " + fmtStr + " unknown");
	imageDescription->size = imageDescription->width * imageDescription->height * CV_ELEM_SIZE(imageFmt->cvType);
	imageDescription->format = imageFmt->name;

	context.tracer().info("Starting thread for camera: " + cameraDescription->name);
	replyTask = new ReplyTask(this, imageDescription->width, imageDescription->height,fps, playerdetection);

	replyTask->start();//my own thread
	}

	virtual ~CameraDEPTH(){
		context.tracer().info("Stopping and joining thread for camera: " + cameraDescription->name);
		gbxiceutilacfr::stopAndJoin(replyTask);
	}
    
	virtual jderobot::ImageDescriptionPtr getImageDescription(const Ice::Current& c){
		return imageDescription;
	}

	virtual jderobot::CameraDescriptionPtr getCameraDescription(const Ice::Current& c){
		return cameraDescription;
	}

	virtual void getImageData_async(const jderobot::AMD_ImageProvider_getImageDataPtr& cb,const Ice::Current& c){
		replyTask->pushJob(cb);
	}

	virtual std::string startCameraStreaming(const Ice::Current&){
		context.tracer().info("Should be made anything to start camera streaming: " + cameraDescription->name);
	}

	virtual void stopCameraStreaming(const Ice::Current&) {
		context.tracer().info("Should be made anything to stop camera streaming: " + cameraDescription->name);
	}
	
	virtual Ice::Int setCameraDescription(const jderobot::CameraDescriptionPtr&, const Ice::Current&){
		
	}

private:
	class ReplyTask: public gbxiceutilacfr::SafeThread{
	public:
		ReplyTask(CameraDEPTH* camera, int width, int height, int fps, int playerDetection)
	: gbxiceutilacfr::SafeThread(camera->context.tracer()), mycameradepth(camera) {
		segmentation=playerDetection;
		this->fps=fps;
      }
		

	void pushJob(const jderobot::AMD_ImageProvider_getImageDataPtr& cb){
		IceUtil::Mutex::Lock sync(requestsMutex);
		requests.push_back(cb);
	}

    virtual void walk(){
		int test;
		

		jderobot::ImageDataPtr reply(new jderobot::ImageData);
		reply->description = mycameradepth->imageDescription;
		reply->pixelData.resize(mycameradepth->imageDescription->width*mycameradepth->imageDescription->height*3);
		IplImage* src = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		IplImage* dst_resize = cvCreateImage(cvSize(mycameradepth->imageDescription->width,mycameradepth->imageDescription->height), IPL_DEPTH_8U, 3);
		g_nTexMapX = (((unsigned short)(sensors[SELCAM].depthMD.FullXRes()-1) / 512) + 1) * 512;
		g_nTexMapY = (((unsigned short)(sensors[SELCAM].depthMD.FullYRes()-1) / 512) + 1) * 512;
		g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
		distances.resize(width*height);

		struct timeval a, b;
		int cycle; // duración del ciclo
		long totala;
		long totalpre=0;
		long diff;

		//std::cout << "FPS depth: " << fps << std::endl;
		cycle=(float)(1/(float)fps)*1000000;
		
		while(!isStopping()){
			gettimeofday(&a,NULL);
			totala=a.tv_sec*1000000+a.tv_usec;
			pthread_mutex_lock(&mutex);


		    IceUtil::Time t = IceUtil::Time::now();
		    reply->timeStamp.seconds = (long)t.toSeconds();
		    reply->timeStamp.useconds = (long)t.toMicroSeconds() - reply->timeStamp.seconds*1000000;
			xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));
			const XnDepthPixel* pDepthRow =(XnDepthPixel*) sensors[SELCAM].depthMD.Data();
			const XnLabel* pLabels = sensors[SELCAM].sceneMD.Data();
			cvZero(src);
			for (XnUInt y = 0; y < sensors[SELCAM].depthMD.YRes(); ++y)
			{
				XnDepthPixel* pDepth = (XnDepthPixel* )pDepthRow;
				for (XnUInt x = 0; x < sensors[SELCAM].depthMD.XRes(); ++x, ++pDepth)
				{
					test= *pDepth;
					
					if ((*pLabels!=0)||(!segmentation)){
						distances[(y*sensors[SELCAM].depthMD.XRes() + x)] = *pDepth;
						if (*pDepth != 0)
						{
	        					int pval = (int)*pDepth/10; 
							
							int lb = pval & 0xff;
							
							switch (pval>>8) {
							case 0:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 255;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 255-lb;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 255-lb;
								break;
							case 1:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 255;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = lb;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 0;
								break;
							case 2:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 255-lb;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 255;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 0;
								break;
							case 3:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 255;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = lb;
								break;
							case 4:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 255-lb;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 255;
								break;
							case 5:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 255-lb;
								break;
							default:
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 0;
								src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 0;
								break;
							}
						}
						else{
							src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+0] = 0;
							src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+1] = 0;
							src->imageData[(y*sensors[SELCAM].depthMD.XRes() + x)*3+2] = 0;
		
						}
					}
					else{
						distances[(y*sensors[SELCAM].depthMD.XRes() + x)] = 0;
					}
					++pLabels;
				
				}
				pDepthRow += sensors[SELCAM].depthMD.XRes();
			}
			//cvFlip(src, NULL, 1);

		   if ((mycameradepth->imageDescription->width != sensors[SELCAM].imageMD.XRes()) || (mycameradepth->imageDescription->height != sensors[SELCAM].imageMD.YRes())){
				cvResize(src,dst_resize);
				memcpy(&(reply->pixelData[0]),(unsigned char *) dst_resize->imageData,dst_resize->width*dst_resize->height * 3);
			}
			else{
				memcpy(&(reply->pixelData[0]),(unsigned char *) src->imageData,src->width*src->height * 3);
			}
		    
		    {//critical region start
			IceUtil::Mutex::Lock sync(requestsMutex);
		    while(!requests.empty()){
				jderobot::AMD_ImageProvider_getImageDataPtr cb = requests.front();
				requests.pop_front();
				cb->ice_response(reply);
			}
			}//critical region end
			pthread_mutex_unlock(&mutex);
			if (totalpre !=0){
				if ((totala - totalpre) > cycle ){
					std::cout<<"-------- openniServer: WARNING- DEPTH timeout-" << std::endl; 
				}
				else{
					usleep(cycle - (totala - totalpre));
				}
			}
			/*if (totalpre !=0){
				std::cout << "depth: " <<  1000000/(totala-totalpre) << std::endl;
			}*/
			totalpre=totala;
		}
	}
	
	CameraDEPTH* mycameradepth;
	IceUtil::Mutex requestsMutex;
	std::list<jderobot::AMD_ImageProvider_getImageDataPtr> requests;
	
	xn::Context *context;
	int segmentation;
	char *img; 
	XnRGB24Pixel* g_pTexMap;
	unsigned int g_nTexMapX,g_nTexMapY;	
	float g_pDepthHist[100000];
	int fps;
	
	
    };
    typedef IceUtil::Handle<ReplyTask> ReplyTaskPtr;


    std::string prefix;
    jderobotice::Context context;
    colorspaces::Image::FormatPtr imageFmt;
    jderobot::ImageDescriptionPtr imageDescription;
    jderobot::CameraDescriptionPtr cameraDescription;
    ReplyTaskPtr replyTask;

  };

/**
* \brief Class wich contains all the functions and variables to serve point cloud interface
*/

	class pointCloudI: virtual public jderobot::remoteCloud{
		public:
			pointCloudI (std::string& propertyPrefix, const jderobotice::Context& context):
				prefix(propertyPrefix),context(context),data(new jderobot::pointCloudData()) {
					Ice::PropertiesPtr prop = context.properties();

					int playerdetection = prop->getPropertyAsIntWithDefault("openniServer.PlayerDetection",0);
					int fps =prop->getPropertyAsIntWithDefault("openniServer.pointCloud.Fps",10);
					if (!(userGeneratorActive))
						playerdetection=0;
					   replyCloud = new ReplyCloud(this,prop->getProperty("openniServer.calibration"), playerdetection, width, height,fps);
					   replyCloud->start();
				}
		

		virtual jderobot::pointCloudDataPtr getCloudData(const Ice::Current&){
				data=replyCloud->getCloud();
				return data;
			};

		virtual Ice::Int initConfiguration(const Ice::Current&){
			
			if (idLocal==0){
	  			/* initialize random seed: */
				srand ( time(NULL) );
	
				/* generate secret number: */
				idLocal = rand() + 1;
	
				std::stringstream ss;//create a stringstream
				ss << idLocal << ".xml";//add number to the stream
				path=ss.str();
	
				f2.open(ss.str().c_str(), std::ofstream::out);
				return idLocal;
			}
			else
				return 0;
			
		};

		virtual std::string read(Ice::Int id, const Ice::Current&){
		};


		virtual Ice::Int write(const std::string& data, Ice::Int id, const Ice::Current&){
			if (id == idLocal){
				f2 << data << std::endl;
			}
		};

		virtual Ice::Int setConfiguration(Ice::Int id, const Ice::Current&){
			if (id == idLocal){
				replyCloud->setCalibrationFile(path);
				id=0;
				f2.close();
			}

			//guardar el xml nuevo encima del cargado por defecto (la siguiente vez empezará directamente con la nueva configuración
		};
		   
		   private:
			 class ReplyCloud :public gbxiceutilacfr::SafeThread{ 
		       public: 
		       	ReplyCloud (pointCloudI* pcloud, std::string filepath,  int playerDetection, int widthIn, int heightIn, int fpsIn) : gbxiceutilacfr::SafeThread(pcloud->context.tracer()), data(new jderobot::pointCloudData()), data2(new jderobot::pointCloudData())
		        	{
					path=filepath;
					segmentation=playerDetection;
					cWidth = widthIn;
					cHeight = heightIn;
					fps=fpsIn;
				}

				void setCalibrationFile(std::string path){
					mypro->load_cam((char*)path.c_str(),0, cWidth, cHeight);
				}

		       
		        void walk()
		        {
				mypro= new openniServer::myprogeo();
				mypro->load_cam((char*)path.c_str(),0, cWidth, cHeight);
				
				struct timeval a, b;
				int cycle; // duración del ciclo
				long totala;
				long totalpre=0;

				cycle=(float)(1/(float)fps)*1000000;

				while(!isStopping()){
					float distance;
					gettimeofday(&a,NULL);
					totala=a.tv_sec*1000000+a.tv_usec;
					pthread_mutex_lock(&mutex);
					data2->p.clear();
					for( unsigned int i = 0 ; (i < cWidth*cHeight)&&(distances.size()>0); i=i+9) {
							distance=(float)distances[i];
							if (distance!=0){
								//if (((unsigned char)srcRGB->imageData[3*i]!=0) && ((unsigned char)srcRGB->imageData[3*i+1]!=0) && ((unsigned char)srcRGB->imageData[3*i+2]!=0)){
									float xp,yp,zp,camx,camy,camz;
									float ux,uy,uz; 
									float x,y;
									float k;
									float c1x, c1y, c1z;
									float fx,fy,fz;
									float fmod;
									float t;
									float Fx,Fy,Fz;
				
									mypro->mybackproject(i % cWidth, i / cWidth, &xp, &yp, &zp, &camx, &camy, &camz,0);
				
									//vector unitario
									float modulo;
				
									modulo = sqrt(1/(((camx-xp)*(camx-xp))+((camy-yp)*(camy-yp))+((camz-zp)*(camz-zp))));
									mypro->mygetcamerafoa(&c1x, &c1y, &c1z, 0);
	
									fmod = sqrt(1/(((camx-c1x)*(camx-c1x))+((camy-c1y)*(camy-c1y))+((camz-c1z)*(camz-c1z))));
									fx = (c1x - camx)*fmod;
									fy = (c1y - camy)*fmod;
									fz = (c1z - camz) * fmod;
									ux = (xp-camx)*modulo;
									uy = (yp-camy)*modulo;
									uz = (zp-camz)*modulo;
									Fx= distance*fx + camx;
									Fy= distance*fy + camy;
									Fz= distance*fz + camz;
									// calculamos el punto real 
									t = (-(fx*camx) + (fx*Fx) - (fy*camy) + (fy*Fy) - (fz*camz) + (fz*Fz))/((fx*ux) + (fy*uy) + (fz*uz));
									//world->add_kinect_point(t*ux + camx,t*uy+ camy,t*uz + camz,i);
									auxP.x=t*ux + camx;
									auxP.y=t*uy+ camy;
									auxP.z=t*uz + camz;
									if ( segmentation){
										auxP.id=pixelsID[i];
									}
									auxP.r=(float)(int) (unsigned char)srcRGB->imageData[3*i];
									auxP.g=(float)(int) (unsigned char)srcRGB->imageData[3*i+1];
									auxP.b=(float)(int) (unsigned char)srcRGB->imageData[3*i+2];
									data2->p.push_back(auxP);
								}
							//}
						}
					pthread_mutex_unlock(&mutex);
					if (totalpre !=0){
						if ((totala - totalpre) > cycle ){
							std::cout<<"-------- openniServer: WARNING- POINTCLOUD timeout-" << std::endl; 
						}
						else{
							usleep(cycle - (totala - totalpre));
						}
					}
					/*if (totalpre !=0){
						std::cout << "cloud: " <<  1000000/(totala-totalpre) << std::endl;
					}*/
					totalpre=totala;
				}
		        }
		        myprogeo *mypro;
				int cWidth;
				int cHeight;
				int fps;
				jderobot::pointCloudDataPtr data, data2;
				jderobot::RGBPoint auxP;
				std::string path;
				int segmentation;
		        
		       jderobot::pointCloudDataPtr getCloud()
		       {
		          pthread_mutex_lock(&mutex);
				data->p=data2->p;
				pthread_mutex_unlock(&mutex);
		          return data;
		       }
				
		      
		    	};	

			typedef IceUtil::Handle<ReplyCloud> ReplyCloudPtr;
			ReplyCloudPtr replyCloud;
			std::string prefix;
			jderobotice::Context context;
			jderobot::pointCloudDataPtr data;
			std::ofstream f2;
			int idLocal;
			std::string path;
			
			
		};




/**
* \brief Class wich contains all the functions and variables to controle the Pose3DMotors module
*/
class Pose3DMotorsI: virtual public jderobot::Pose3DMotors {
	public:
		Pose3DMotorsI(XN_USB_DEV_HANDLE* d, std::string& propertyPrefix, const jderobotice::Context& context): prefix(propertyPrefix),context(context),Pose3DMotorsData(new jderobot::Pose3DMotorsData()), Pose3DMotorsParams(new jderobot::Pose3DMotorsParams())
		{
			Ice::PropertiesPtr prop = context.properties();
			Pose3DMotorsData->tilt=0;
			Pose3DMotorsData->tiltSpeed=0;
			rc= XN_STATUS_OK;
			dev=d;
			rc=xnUSBSendControl( *dev, XN_USB_CONTROL_TYPE_VENDOR, 0x06, 1, 0x00, NULL, 0, 0 );
			CHECK_RC(rc,"led");
     	}

		virtual ~Pose3DMotorsI(){};

		virtual  Ice::Int setPose3DMotorsData(const jderobot::Pose3DMotorsDataPtr& p, const Ice::Current&){
			Pose3DMotorsData=p;
			uint8_t empty[0x1];
			//int angle = 25 * 2;
			rc = xnUSBSendControl(*dev, XN_USB_CONTROL_TYPE_VENDOR, 0x31, (XnUInt16)p->tilt, 0x0, empty, 0x0, 0);
			CHECK_RC(rc,"Changing angle");

		};
	
		virtual jderobot::Pose3DMotorsParamsPtr getPose3DMotorsParams(const Ice::Current&){
			return Pose3DMotorsParams;
		};

		virtual jderobot::Pose3DMotorsDataPtr getPose3DMotorsData (const Ice::Current&){
			return Pose3DMotorsData;
		};

	private:
		std::string prefix;
		jderobotice::Context context;
		jderobot::Pose3DMotorsDataPtr Pose3DMotorsData;
		jderobot::Pose3DMotorsParamsPtr Pose3DMotorsParams;
		XnStatus rc;
		XN_USB_DEV_HANDLE* dev;
    };

/**
* \brief Class wich contains all the functions and variables to controle the KinectLeds module
*/
class KinectLedsI: virtual public jderobot::KinectLeds {
	public:
		KinectLedsI(XN_USB_DEV_HANDLE* d, std::string& propertyPrefix, const jderobotice::Context& context): prefix(propertyPrefix),context(context)
		{
			Ice::PropertiesPtr prop = context.properties();
			dev=d;
     	}

		virtual ~KinectLedsI(){};

		virtual  void setLedActive(jderobot::KinectLedsAvailable led, const Ice::Current&){
			int iled;
			if (led==jderobot::OFF)
				iled=0;
			if (led==jderobot::GREEN)
				iled=1;
			if (led==jderobot::RED)
				iled=2;
			if (led==jderobot::YELLOW)
				iled=3;
			if (led==jderobot::BLINKYELLOW)
				iled=4;
			if (led==jderobot::BLINKGREEN)
				iled=5;
			if (led==jderobot::BLINKRED)
				iled=6;
			uint8_t empty[0x1];
			rc = xnUSBSendControl(*dev, XN_USB_CONTROL_TYPE_VENDOR, 0x6, iled, 0x0, empty, 0x0, 0);
			CHECK_RC(rc,"Changing led");
		}

	private:
		std::string prefix;
		jderobotice::Context context;
		XN_USB_DEV_HANDLE* dev;
    };



/**
* \brief Class wich contains all the functions to remote configuration
*/
class RemoteConfigI: virtual public jderobot::remoteConfig {
	public:
		RemoteConfigI(Ice::ObjectPtr pointcloud1, std::string& propertyPrefix, const jderobotice::Context& context): prefix(propertyPrefix),context(context)
		{
			Ice::PropertiesPtr prop = context.properties();
			
     	}

		virtual ~RemoteConfigI(){};

		virtual Ice::Int initConfiguration(Ice::Int idConfig, const Ice::Current&){
			
			if (idLocal==0){
	  			/* initialize random seed: */
				srand ( time(NULL) );
	
				/* generate secret number: */
				idLocal = rand() + 1;
	
				std::stringstream ss;//create a stringstream
				ss << idLocal << ".txt";//add number to the stream
	
				f2.open(ss.str().c_str(), std::ofstream::out);
				return idLocal;
			}
			else
				return 0;
			
		};

		virtual std::string read(Ice::Int id, const Ice::Current&){
		};


		virtual Ice::Int write(const std::string& data, Ice::Int id, const Ice::Current&){
			if (id == idLocal){
				f2 << data << std::endl;
			}
		};

		virtual Ice::Int setConfiguration(Ice::Int id, const Ice::Current&){
			if (id == idLocal){
				id=0;
				f2.close();
			}

			//guardar el xml nuevo encima del cargado por defecto (la siguiente vez empezará directamente con la nueva configuración
		};


	private:
		
		std::string prefix;
		jderobotice::Context context;
		std::ofstream f2;
		int idLocal;
    };



/**
* \brief Main Class of the component wich create the diferents devices activated using the Ice configuration file.
*/
class Component: public jderobotice::Component{
public:
	Component()
	:jderobotice::Component("openniServer"), cameras(0) {}

	virtual void start(){
		Ice::PropertiesPtr prop = context().properties();
		int cameraR = prop->getPropertyAsIntWithDefault(context().tag() + ".CameraRGB",0);
		int cameraD = prop->getPropertyAsIntWithDefault(context().tag() + ".CameraDEPTH",0);
		int motors = prop->getPropertyAsIntWithDefault(context().tag() + ".Pose3DMotorsActive",0);
		int leds = prop->getPropertyAsIntWithDefault(context().tag() + ".KinectLedsActive",0);
		int pointCloud = prop->getPropertyAsIntWithDefault(context().tag() + ".pointCloudActive",0);
		int playerdetection = prop->getPropertyAsIntWithDefault(context().tag() + ".PlayerDetection",0);
		width=prop->getPropertyAsIntWithDefault("openniServer.Width", 640);
		height=prop->getPropertyAsIntWithDefault("openniServer.Height",480);
		int fps=prop->getPropertyAsIntWithDefault("openniServer.Fps",30);
		

		SELCAM = prop->getPropertyAsIntWithDefault(context().tag() + ".deviceId",0);
		std::cout << "Selected device: " << SELCAM << std::endl;
		int nCameras=0;
		XnLicense m_license;

//		std::vector<DeviceInfo> sensors;

		/*COLORS*/
		colors[0][0]=0;
		colors[0][1]=0;
		colors[0][2]=255;
		colors[1][0]=0;
		colors[1][1]=255;
		colors[1][2]=255;
		colors[2][0]=255;
		colors[2][1]=255;
		colors[2][2]=0;
		colors[3][0]=255;
		colors[3][1]=0;
		colors[3][2]=0;
		colors[4][0]=0;
		colors[4][1]=255;
		colors[4][2]=0;
		colors[5][0]=255;
		colors[5][1]=255;
		colors[5][2]=0;
		colors[6][0]=0;
		colors[6][1]=0;
		colors[6][2]=0;
		colors[7][0]=150;
		colors[7][1]=150;
		colors[7][2]=0;
		colors[8][0]=150;
		colors[8][1]=150;
		colors[8][2]=150;
		colors[9][0]=0;
		colors[9][1]=150;
		colors[9][2]=150;

		nCameras=cameraR + cameraD;
		//g_context =  new xn::Context;
		std::cout << "NCAMERAS = " << nCameras << std::endl;
		cameras.resize(MAX_CAMERAS_SERVER);
		pthread_mutex_init(&mutex, NULL);
		if ((nCameras>0)||(pointCloud)){
			// Getting Sensors information and configure all sensors
			rc = g_context.Init();
			xn::NodeInfoList device_node_info_list;
    			rc = g_context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, device_node_info_list);
    			if (rc != XN_STATUS_OK && device_node_info_list.Begin () != device_node_info_list.End ())
				std::cout << "enumerating devices failed. Reason: " << xnGetStatusString(rc) << std::endl;
	    		for (xn::NodeInfoList::Iterator nodeIt = device_node_info_list.Begin();
         			nodeIt != device_node_info_list.End (); ++nodeIt)
    				{
				
        			const xn::NodeInfo& deviceInfo = *nodeIt;
        			const XnProductionNodeDescription& description = deviceInfo.GetDescription();
        			std::cout << cv::format("Found device: vendor %s name %s", description.strVendor, description.strName) << std::endl;
			

		   		sensors[n_devices].camera_type = description.strName;
		   		sensors[n_devices].vendor = description.strVendor;
		   		sensors[n_devices].creation_info = deviceInfo.GetCreationInfo();

		   		unsigned short vendor_id;
		   		unsigned short product_id;
		   		unsigned char bus;
		   		unsigned char address;        
		   		sscanf(deviceInfo.GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
		   		sensors[n_devices].vendor_id = vendor_id;
		   		sensors[n_devices].product_id = product_id;
		   		sensors[n_devices].bus = bus;
		   		sensors[n_devices].address = address;
		   		//sensors.push_back(info);
				n_devices++;
    			}

			strcpy(m_license.strVendor, "PrimeSense");
    			strcpy(m_license.strKey, "0KOIk2JeIBYClPWVnMoRKn5cdY4=");
    			g_context.AddLicense(m_license);

			/*licencias */
			libusb_context *context = 0;
			int result = libusb_init(&context); //initialize a library session
			if (result < 0) 
				return;

			libusb_device **devices;
			int count = libusb_get_device_list (context, &devices);
			if (count < 0) 
				return;   //Count is the number of USB devices

			for (int devIdx = 0; devIdx < count; ++devIdx)
			{
				libusb_device* device = devices[devIdx];
				uint8_t busId = libusb_get_bus_number (device);
				uint8_t address = libusb_get_device_address (device);

				int device_id = -1;
				for (size_t i = 0; device_id < 0 && i < n_devices; ++i)
				{
					if (busId == sensors[i].bus && address == sensors[i].address)
					device_id = i;
				}

				if (device_id < 0)
					continue;

        			libusb_device_descriptor descriptor;
        			result = libusb_get_device_descriptor (devices[devIdx], &descriptor);
        			if (result == 0)
        			{
            			libusb_device_handle* dev_handle;
            			result = libusb_open (device, &dev_handle);
            			if (result == 0)
            			{
                			unsigned char buffer[1024];
                			int len = libusb_get_string_descriptor_ascii (dev_handle, descriptor.iSerialNumber, buffer, 1024);

                			if (len > 4)
                			{
                    			buffer[len] = 0;
                    			sensors[device_id].serial = std::string((const char*) buffer);
               			}
                			else
                			{
                    			// If there is no serial (e.g. Asus XTION), use the bus address.
                    			sensors[device_id].serial = cv::format("%d", busId);
                			}
                			libusb_close (dev_handle);
            			}
        			}
    			}
    			libusb_free_device_list (devices, 1);
    			libusb_exit (context);
			std::cout << "Number of detected devices: " << n_devices << std::endl;
			NodeInfoList devicesList;
			int devicesListCount = 0;
		   	rc = g_context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, devicesList);
			for (NodeInfoList::Iterator it = devicesList.Begin(); it != devicesList.End(); ++it)
			{
				devicesListCount++;
			}
			CHECK_RC(rc, "Enumerate");
			int i=0;
			for (NodeInfoList::Iterator it = devicesList.Begin(); it != devicesList.End(); ++it, ++i)
			{
				if (i==SELCAM){
					KinectDevice& info = sensors[i];
	        			if (info.serial.empty())
	            			info.serial = i;
					std::cout << cv::format("[Device %d] %s, %s, serial=%s", i, info.vendor.c_str(), info.camera_type.c_str(), info.serial.c_str()) << std::endl;
					// Create the device node
					NodeInfo deviceInfo = *it;
				 	rc = g_context.CreateProductionTree(deviceInfo, sensors[i].device);
					CHECK_RC(rc, "Create Device");
					Query query;
			       	query.AddNeededNode(deviceInfo.GetInstanceName());
					xnOSMemCopy(sensors[i].name,deviceInfo.GetInstanceName(),
					xnOSStrLen(deviceInfo.GetInstanceName()));
					// Now create a depth generator over this device
				 	rc = g_context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &query, sensors[i].depth);
					// depth configuration:
	
					/*XnUInt32 xNuma = sensors[i].depth.GetSupportedMapOutputModesCount();
					cout << "Support mode: " << xNuma << endl;
	
			
					XnMapOutputMode* aModeD = new XnMapOutputMode[xNuma];
					sensors[i].depth.GetSupportedMapOutputModes( aModeD, xNuma );
					for( unsigned int j = 0; j < xNuma; j++ )
					{
						std::cout << aModeD[j].nXRes << " * " << aModeD[j].nYRes << " @" << aModeD[j].nFPS << "FPS" << std::endl;
					}
					delete[] aModeD;*/
	
	
	
	
	    			    XnMapOutputMode depth_mode; 
					depth_mode.nXRes = width;
				    depth_mode.nYRes = height;
				    depth_mode.nFPS = 30;
				    sensors[i].depth.SetMapOutputMode(depth_mode);
	
					CHECK_RC(rc, "Create Depth");
					// now create a image generator over this device
				  	rc = g_context.CreateAnyProductionTree(XN_NODE_TYPE_IMAGE, &query, sensors[i].image);
					CHECK_RC(rc, "Create Image");
					/*XnUInt32 xNumb = sensors[i].image.GetSupportedMapOutputModesCount();
					cout << "Support mode: " << xNumb << endl;
					XnMapOutputMode* aModeR = new XnMapOutputMode[xNumb];
					sensors[i].image.GetSupportedMapOutputModes( aModeR, xNumb );
					for( unsigned int j = 0; j < xNumb; j++ )
					{
						std::cout << aModeR[j].nXRes << " * " << aModeR[j].nYRes << " @" << aModeR[j].nFPS << "FPS" << std::endl;
					}
					delete[] aModeD;*/
	
					rc = g_context.CreateAnyProductionTree(XN_NODE_TYPE_USER, &query, sensors[i].g_UserGenerator);
					CHECK_RC(rc, "Find user generator");
	
					XnMapOutputMode rgb_mode;
	            		rgb_mode.nXRes = width;
	            		rgb_mode.nYRes = height;
            			rgb_mode.nFPS = fps;
	        			sensors[i].image.SetMapOutputMode(rgb_mode);
		       		
					sensors[i].depth.GetAlternativeViewPointCap().SetViewPoint(sensors[i].image);
					
					XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected, hCalibrationInProgress, hPoseInProgress;
					if (playerdetection){
						/*init player id array*/
						pixelsID.resize(width*height);
						if (!sensors[i].g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
						{
							printf("Supplied user generator doesn't support skeleton\n");
							//return 1;
							userGeneratorActive=0;
						}
				
						rc = sensors[i].g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
						CHECK_RC(rc, "Register to user callbacks");
						rc = sensors[i].g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
						CHECK_RC(rc, "Register to calibration start");
						rc = sensors[i].g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
						CHECK_RC(rc, "Register to calibration complete");
						userGeneratorActive=1;
						if (sensors[i].g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
						{
							g_bNeedPose = TRUE;
							if (!sensors[i].g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
							{
								std::cout << "Pose required, but not supported" << std::endl;
							}
							rc = sensors[i].g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
							CHECK_RC(rc, "Register to Pose Detected");
							sensors[i].g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);

							rc = sensors[i].g_UserGenerator.GetPoseDetectionCap().RegisterToPoseInProgress(MyPoseInProgress, NULL, hPoseInProgress);
							CHECK_RC(rc, "Register to pose in progress");
						}

						sensors[i].g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

						rc = sensors[i].g_UserGenerator.GetSkeletonCap().RegisterToCalibrationInProgress(MyCalibrationInProgress, NULL, hCalibrationInProgress);
						CHECK_RC(rc, "Register to calibration in progress");
					}
				}
			}
			g_context.SetGlobalMirror(false);
			g_context.StartGeneratingAll();
			/*XnFPSData xnFPS;
		   	rc = xnFPSInit(&xnFPS, 180);
		   	CHECK_RC(rc, "FPS Init");*/
			rc =  pthread_create(&threads[0], NULL, &openniServer::kinectThread, NULL);
			if (rc){
		 		std::cout<< "ERROR; return code from pthread_create() is " << rc << std::endl;
	            	exit(-1);
     	    	}
		}

		if ((motors) || (leds)){
			const XnUSBConnectionString *paths; 
			XnUInt32 count; 
			std::cout << "inicializo el dispositivo" << std::endl;
			rc = xnUSBInit();
			CHECK_RC(rc, "USB Initialization") ;
			//rc = xnUSBOpenDevice(VID_MICROSOFT, PID_NUI_MOTOR, NULL, NULL, &dev);
			CHECK_RC(rc,"Openning Device");
			rc = xnUSBEnumerateDevices(VID_MICROSOFT, PID_NUI_MOTOR, &paths, &count);
             	CHECK_RC(rc,"xnUSBEnumerateDevices failed");


	        	// Open first found device
        		rc = xnUSBOpenDeviceByPath(paths[SELCAM], &dev);
	        	CHECK_RC(rc,"xnUSBOpenDeviceByPath failed");
		}

		if (cameraR){
			std::string objPrefix(context().tag() + ".CameraRGB.");
			std::string cameraName = prop->getProperty(objPrefix + "Name");
			if (cameraName.size() == 0){//no name specified, we create one using the index
				cameraName = "cameraR";
				prop->setProperty(objPrefix + "Name",cameraName);//set the value
				}
			context().tracer().info("Creating camera " + cameraName);
			cameras[0] = new CameraRGB(objPrefix,context());
			context().createInterfaceWithString(cameras[0],cameraName);
			//test camera ok
			std::cout<<"              -------- openniServer: Component: CameraRGB created successfully   --------" << std::endl;
		}
		if (cameraD){
			std::string objPrefix(context().tag() + ".CameraDEPTH.");
			std::string cameraName = prop->getProperty(objPrefix + "Name");
			if (cameraName.size() == 0){//no name specified, we create one using the index
				cameraName = "cameraD";
				prop->setProperty(objPrefix + "Name",cameraName);//set the value
				}
			context().tracer().info("Creating camera " + cameraName);
			cameras[1] = new CameraDEPTH(objPrefix,context());
			context().createInterfaceWithString(cameras[1],cameraName);
			//test camera ok
			std::cout<<"              -------- openniServer: Component: CameraDEPTH created successfully   --------" << std::endl;
		}
		if (motors){
			std::string objPrefix4="Pose3DMotors1";
			std::string Pose3DMotorsName = "Pose3DMotors1";
			context().tracer().info("Creating Pose3DMotors1 " + Pose3DMotorsName);
			Pose3DMotors1 = new Pose3DMotorsI(&dev,objPrefix4,context());
			context().createInterfaceWithString(Pose3DMotors1,Pose3DMotorsName);
			std::cout<<"              -------- openniServer: Component: Pose3DMotors created successfully   --------" << std::endl;
		}
			
		if (leds){
			std::string objPrefix4="kinectleds1";
			std::string Name = "kinectleds1";
			context().tracer().info("Creating kinectleds1 " + Name);
			kinectleds1 = new KinectLedsI(&dev,objPrefix4,context());
			context().createInterfaceWithString(kinectleds1,Name);
			std::cout<<"              -------- openniServer: Component: KinectLeds created successfully   --------" << std::endl;
		}
		if (pointCloud){
			std::string objPrefix5="pointcloud1";
			std::string Name = "pointcloud1";
			context().tracer().info("Creating pointcloud1 " + Name);
			pointcloud1 = new pointCloudI(objPrefix5,context());
			context().createInterfaceWithString(pointcloud1,Name);
			std::cout<<"              -------- openniServer: Component: PointCloud created successfully   --------" << std::endl;
		}


    }

    virtual ~Component(){
    }

  private:
    std::vector<Ice::ObjectPtr> cameras;
	Ice::ObjectPtr Pose3DMotors1;
	Ice::ObjectPtr kinectleds1;
	Ice::ObjectPtr pointcloud1;
	pthread_t threads[NUM_THREADS];
	XN_USB_DEV_HANDLE dev;

  };

} //namespace

int main(int argc, char** argv){

  openniServer::Component component;

	//usleep(1000);
     jderobotice::Application app(component);

     return app.jderobotMain(argc,argv);

}
