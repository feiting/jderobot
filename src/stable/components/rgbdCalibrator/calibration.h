/*
 *
 *  Copyright (C) 1997-2013 JDERobot Developers Team
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
 *  Authors : Roberto Calvo <rocapal [at] gsyc [dot] urjc [dot] es>
 *
 */

#ifndef RGBDCALIBRATOR_CALIBRATION_H
#define RGBDCALIBRATOR_CALIBRATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <string>
#include <visionlib/colorspaces/colorspacesmm.h>
#include <jderobot/camera.h>
#include "../../libs/geometry/math/Point3D.h"
#include <boost/tuple/tuple.hpp>

#include "../../libs/geometry/progeo/Progeo.h"
using namespace cv;

namespace rgbdCalibrator
{

  class Calibration
  {

  public:

    Calibration();
    ~Calibration();

    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

    bool runCalibrationAndSave(Size &boardSize, 
			       float squareSize, 
			       int flag, 
			       Size imageSize, 
			       Mat&  cameraMatrix, 
			       Mat& distCoeffs,
			       vector<vector<Point2f> > imagePoints );

    void BackProjectWithDepth (const Eigen::Vector3d pixel,
			       const colorspaces::Image depthData,
			       Eigen::Vector4d& res3D);

    bool addPatternPixel (Eigen::Vector3d pixel, 
			  const colorspaces::Image depthData);

    Eigen::Matrix4d getRTSolution() { return mRTsolution; };

    void test(Eigen::Vector4d pCamera);

  private:

    Mat mKMatrix;

    Eigen::Matrix4d mRTsolution;

    Progeo::Progeo *mProgeo;

    std::vector<Eigen::Vector3d> mPixelPoints;
    std::vector<Eigen::Vector4d> mPatternPoints;
    std::vector<Eigen::Vector4d> mCameraPoints;
    std::vector<std::pair<Eigen::Vector4d,Eigen::Vector4d> > mPairPoints;

    void initProgeo();

    void getRealPoint(const Eigen::Vector3d pixel,
		       const colorspaces::Image depthData,
		       Eigen::Vector4d& res3D);
    
    void initPatternPoints();

    void LSO();
   

    void calcBoardCornerPositions(Size boardSize, 
				  float squareSize, 
				  vector<Point3f>& corners,  
				  Pattern patternType /*= Settings::CHESSBOARD*/);

    double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints, 
				      const vector<vector<Point2f> >& imagePoints, 
				      const vector<Mat>& rvecs, 
				      const vector<Mat>& tvecs, 
				      const Mat& cameraMatrix , 
				      const Mat& distCoeffs, 
				      vector<float>& perViewErrors);

    bool runCalibration(Size &boardSize, 
			float squareSize, 
			int flag, 
			Size& imageSize, 
			Mat& cameraMatrix, 
			Mat& distCoeffs,
			vector<vector<Point2f> > imagePoints, 
			vector<Mat>& rvecs, 
			vector<Mat>& tvecs,
			vector<float>& reprojErrs,  
			double& totalAvgErr);


  
    
  };

  


}


#endif //RGBDCALIBRATOR_CALIBRATION_H 
