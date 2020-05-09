#include <iostream>
#include <string>
#include <stdio.h>

#include "stereocamera.h"
#include "frameid.h"
#include "taskiddef.h"
#include "mycamerahandler.h"
#include "calibrationparams.h"
#include "rotationmatrix.h"

int main(int argc, char *argv[])
{
    StereoCamera *cameraA = StereoCamera::connect("192.168.1.251");
    MyCameraHandler *cameraHandlerA = new MyCameraHandler("camera A");

    // select tasks you want to run, you can also " enableTasks(TaskId::NoTask); " to pause all tasks
    cameraA->enableTasks(TaskId::ObstacleTask | TaskId::LaneTask | TaskId::DisplayTask);

    // you can request more data, like: FrameId::CalibLeftCamera | FrameId::Disparity
    cameraA->requestFrame(cameraHandlerA, FrameId::Disparity );

    // if you want to connect more device, create another one:
//    StereoCamera *cameraB = StereoCamera::connect("192.168.20.100");
//    MyCameraHandler *cameraHandlerB = new MyCameraHandler("camera B");
//    cameraB->requestFrame(cameraHandlerB, FrameId::CalibRightCamera);

    // prevent app to exit.
    int c = 0;
    while (c!= 'x')
    {
        switch (c)
        {
        case 'f':
        {
//      get stereo camera parameters for converting disparity to distance
            StereoCalibrationParameters params;
            if (cameraA->requestStereoCameraParameters(params)) {
                cameraHandlerA->setStereoCalibParams(params);
            } else {
                std::cout << "Stereo camera parameters request failed." << std::endl;
            }
        }
            break;
        case 'r':
        {
//      get rotation matrix
            RotationMatrix rotationMatrix;
            if (cameraA->requestRotationMatrix(rotationMatrix)) {
                cameraHandlerA->setRotationMatrix(rotationMatrix);
            } else {
                std::cout << "Rotation matrix request failed." << std::endl;
            }
        }
            break;
        case 'u':
        {
            std::cout << "Input the path of your local firmware package: " << std::endl;
            std::string packagePath;
            std::cin >> packagePath;
            cameraA->updateFirmware(packagePath.data());
        }
            break;
        case 'l':
        {
            int light = -1;
            bool result = false;
            result = cameraA->getAmbientLight(light);
            if(result){
                switch(light){
                case 1:
                    std::cout<<"day"<<std::endl;
                    break;
                case 2:
                    std::cout<<"dusk"<<std::endl;
                    break;
                case 3:
                    std::cout<<"night"<<std::endl;
                    break;
                default:
                    std::cout<<"unknown"<<std::endl;
                }
            }
        }
            break;
        case 'z':
        {
            cameraA->disconnectFromServer();
        }
            break;
        default:
            break;
        }
        c = getchar();
    }
}
