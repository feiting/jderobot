/*
 *
 *  Copyright (C) 1997-2009 JDERobot Developers Team
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
 *  Authors : David Lobato Bravo <dav.lobato@gmail.com>
 *	      Sara Marugán Alonso <smarugan@gsyc.es>
 *
 */


#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include <jderobot/camera.h>
#include <jderobot/image.h>
#include <colorspaces/colorspacesmm.h>


//Opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <stdlib.h>
#include <list>


namespace cameraserver{

class CameraI: virtual public jderobot::Camera {


    public:
    std::string name;
    std::string uri;
    int framerateN;
    int framerateD;

        CameraI(std::string propertyPrefix, Ice::CommunicatorPtr ic)
               : prefix(propertyPrefix) {

            std::cout << "Constructor CameraI -> " << propertyPrefix << std::endl;

            imageDescription = (new jderobot::ImageDescription());
            cameraDescription = (new jderobot::CameraDescription());

            Ice::PropertiesPtr prop = ic->getProperties();

            //fill cameraDescription
            name = prop->getProperty(prefix+"Name");
            if (name.size() == 0)
                  throw "Camera name not configured";

            cameraDescription->shortDescription = prop->getProperty(prefix+"ShortDescription");
            cameraDescription->streamingUri = prop->getProperty(prefix+"StreamingUri");

            //fill imageDescription
            imageDescription->width = prop->getPropertyAsIntWithDefault(prefix+"ImageWidth",340);
            imageDescription->height = prop->getPropertyAsIntWithDefault(prefix+"ImageHeight",280);

            //we use formats acording to colorspaces
            std::string fmtStr = prop->getPropertyWithDefault(prefix+"Format","YUY2");//default format YUY2
            imageFmt = colorspaces::Image::Format::searchFormat(fmtStr);
            if (!imageFmt)
                throw  "Format " + fmtStr + " unknown";

            imageDescription->size = imageDescription->width * imageDescription->height * CV_ELEM_SIZE(imageFmt->cvType);
            imageDescription->format = imageFmt->name;

            //fill pipeline cfg
            uri = prop->getProperty(prefix+"Uri");
            framerateN = prop->getPropertyAsIntWithDefault(prefix+"FramerateN",25);
            framerateD = prop->getPropertyAsIntWithDefault(prefix+"FramerateD",1);

            std::cout << "URI: " << uri << std::endl;

            if(uri.size()>3)
                cap.open(uri);
            else
                cap.open(atoi(uri.c_str()));

            if(cap.isOpened()){
                replyTask = new ReplyTask(this);
                replyTask->start(); // my own thread
            }else{
                exit(-1);
            }
        }

        std::string getName () {
            return (cameraDescription->name);
        }

        std::string getRobotName () {
//            return ((context.properties())->getProperty(context.tag()+".RobotName"));
        }

        virtual ~CameraI() {
//            context.tracer().info("Stopping and joining thread for camera: " + cameraDescription->name);
//            gbxiceutilacfr::stopAndJoin(replyTask);
        }

        virtual jderobot::ImageDescriptionPtr getImageDescription(const Ice::Current& c){
            return imageDescription;
        }

        virtual jderobot::CameraDescriptionPtr getCameraDescription(const Ice::Current& c){
            return cameraDescription;
        }

        virtual Ice::Int setCameraDescription(const jderobot::CameraDescriptionPtr &description, const Ice::Current& c) {
            return 0;
        }

        virtual void getImageData_async(const jderobot::AMD_ImageProvider_getImageDataPtr& cb,const Ice::Current& c){
            replyTask->pushJob(cb);
        }

        virtual std::string startCameraStreaming(const Ice::Current&){
        }

        virtual void stopCameraStreaming(const Ice::Current&) {
        }

    private:

        class ReplyTask: public IceUtil::Thread {
            public:
                ReplyTask(CameraI* camera)
                {
                    std::cout << "safeThread" << std::endl;
                    mycamera = camera;
                }

                void pushJob(const jderobot::AMD_ImageProvider_getImageDataPtr& cb){
                    IceUtil::Mutex::Lock sync(requestsMutex);
                    requests.push_back(cb);
                }

                virtual void run(){
                    jderobot::ImageDataPtr reply(new jderobot::ImageData);
                    struct timeval a, b;
                    int cycle = 48;
                    long totalb,totala;
                    long diff;

                    int count = 0 ;
                    cv::Mat frame;

                    int cycle_control = 1000/mycamera->framerateN;

                    while(1){

                        gettimeofday(&a,NULL);
                        totala=a.tv_sec*1000000+a.tv_usec;

                        if(!mycamera->cap.isOpened()){
                            exit(-1);
                        }

                        mycamera->cap >> frame;

                        if(!frame.data){
                            mycamera->cap.set(CV_CAP_PROP_POS_AVI_RATIO, 0.0);
                            mycamera->cap >> frame;
                        }
                        cv::cvtColor(frame, frame, CV_RGB2BGR);

                        if(mycamera->imageDescription->width!=frame.rows &&
                           mycamera->imageDescription->height!=frame.cols)
                            cv::resize(frame, frame,
                                       cv::Size(mycamera->imageDescription->width,
                                                mycamera->imageDescription->height));

                        if(count==0){
                            reply->description = mycamera->imageDescription;
                            count++;
                        }

                        IceUtil::Time t = IceUtil::Time::now();
                        reply->timeStamp.seconds = (long)t.toSeconds();
                        reply->timeStamp.useconds = (long)t.toMicroSeconds() - reply->timeStamp.seconds*1000000;

//                        pthread_mutex_lock (&mycamera->cameraI->mutex);
                        reply->pixelData.resize(frame.rows*frame.cols*3);

                        memcpy( &(reply->pixelData[0]), (unsigned char *) frame.data, frame.rows*frame.cols*3);
//                        pthread_mutex_unlock (&mycamera->cameraI->mutex);

                       { //critical region start
                           IceUtil::Mutex::Lock sync(requestsMutex);
                           while(!requests.empty()) {
                               jderobot::AMD_ImageProvider_getImageDataPtr cb = requests.front();
                               requests.pop_front();
                               cb->ice_response(reply);
                           }
                       } //critical region end

                        gettimeofday(&b,NULL);
                        totalb=b.tv_sec*1000000+b.tv_usec;

                        diff = (totalb-totala)/1000;
                        diff = cycle-diff;

                        //std::cout << "Gazeboserver takes " << diff << " ms " << mycamera->fileName << std::endl;

                        if (diff < 0 || diff > cycle_control)
                            diff = cycle_control;
                        else
                            diff = cycle_control - diff;

                        /*Sleep Algorithm*/
                        usleep(diff * 1000);
//                        if (diff < 33)
//                            usleep(33 * 1000);
                    }
                }

                CameraI* mycamera;
                IceUtil::Mutex requestsMutex;
                std::list<jderobot::AMD_ImageProvider_getImageDataPtr> requests;
        };

        typedef IceUtil::Handle<ReplyTask> ReplyTaskPtr;
        std::string prefix;
        colorspaces::Image::FormatPtr imageFmt;
        jderobot::ImageDescriptionPtr imageDescription;
        jderobot::CameraDescriptionPtr cameraDescription;
        ReplyTaskPtr replyTask;
        cv::VideoCapture cap;

}; // end class CameraI

} //namespace

int main(int argc, char** argv)
{
    std::vector<Ice::ObjectPtr> cameras;

    Ice::CommunicatorPtr ic;
    try{
        ic = Ice::initialize(argc, argv);

        Ice::PropertiesPtr prop = ic->getProperties();

        std::string Endpoints = prop->getProperty("CameraSrv.Endpoints");

        int nCameras = prop->getPropertyAsInt("CameraSrv.NCameras");
        cameras.resize(nCameras);
		Ice::ObjectAdapterPtr adapter =ic->createObjectAdapterWithEndpoints("CameraServer", Endpoints);
        for (int i=0; i<nCameras; i++){//build camera objects
          std::stringstream objIdS;
          objIdS <<  i;
          std::string objId = objIdS.str();// should this be something unique??
          std::string objPrefix("CameraSrv.Camera." + objId + ".");
          std::string cameraName = prop->getProperty(objPrefix + "Name");
          Ice::ObjectPtr object = new cameraserver::CameraI(objPrefix, ic);

          adapter->add(object, ic->stringToIdentity(cameraName));

          
        }
		adapter->activate();
        ic->waitForShutdown();

    }catch (const Ice::Exception& ex) {
        std::cerr << ex << std::endl;
        exit(-1);
    } catch (const char* msg) {
        std::cerr << msg << std::endl;
        exit(-1);
    }

}
