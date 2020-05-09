#ifndef MYIMAGEHANDLER_H
#define MYIMAGEHANDLER_H
#include "camerahandler.h"
#include <string>
#include "calibrationparams.h"
#include "rotationmatrix.h"

class MyCameraHandler: public CameraHandler
{
public:
    MyCameraHandler(std::string name);
    void handleRawFrame(const RawImageFrame *rawFrame);
    void handleUpdateFinished(Result result);
    void setStereoCalibParams(StereoCalibrationParameters &params);
    void setRotationMatrix(RotationMatrix &rotationMatrix);

protected:
    void processFrame(int frameId, char *image, uint32_t dataSize, int width, int height, int frameFormat);
    void handleDisparityPointByPoint(unsigned char *image, int width, int height, int bitNum);
    void handleDisparityByLookupTable(unsigned char *image, int width, int height, int bitNum);

private:
    std::string mName;
    bool mIsLookupTableGenerated;
    Result mUpgradeResult;
    StereoCalibrationParameters mStereoCalibrationParameters;
    bool mIsCalibParamReady;
    RotationMatrix mRotationMatrix;
};

#endif // MYIMAGEHANDLER_H
