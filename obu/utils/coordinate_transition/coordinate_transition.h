/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file coordinate_transition.h
 * @brief The coordinate transition program for the v2x module
 */

#ifndef V2X_APP_POLICIES_COORDINATE_TRANSITION_COORDINATE_TRANSITION_H
#define V2X_APP_POLICIES_COORDINATE_TRANSITION_COORDINATE_TRANSITION_H

#include <math.h>
#include <iostream>
#include "v2x_gflags.h"

// namespace apollo {
namespace v2x {

class CoordinateTransition {
 public:
  explicit CoordinateTransition();
  ~CoordinateTransition() {}
  // Function that init
  void Init();
  // Function that convert LatitudeLongitude pair to x and y coordinates
  // in the Universal Transverse Mercator projection
  void LatLonToUTMXY(const double lat, const double lon, double& utm_x,
                     double& utm_y);
  // Function that convert XY coordinates in the Universal Transverse Mercator
  // projection to LatitudeLongitude pair
  void UTMXYToLatLon(double utm_x, double utm_y, double& lat, double& lon);
  // Function that convert Theta to Heading, velocity_x and velocity_y(m/s)
  void ThetaToHeading(const double theta_radian, double velocity_x,
                      double velocity_y, double& heading);
  // Function that convert Deg to Rad
  double DegToRad(const double deg) {
    return (deg / 180.0 * kPI);
  }
  // Function that Rad Theta to Deg
  double RadToDeg(const double rad) {
    return (rad / kPI * 180.0);
  }
  const double kPI = 3.14159265358979323846;
  const double kEmA = 6378137.0;
  const double kEmB = 6356752.3142451;
  const double kUTMScaleFactor = 0.9996;
  const double kUTMXCompensation = 500000.0;
  const double kUTMYCompensation = 10000000.0;

 private:
  // Function that mathematical formula to achieve LatLon to XY
  void MathLanLonToXY(const double lat_radian, const double lon_radian,
                      const double central_meridian_radian, double& x,
                      double& y);
  // Function that mathematical formula to achieve XY to LatLon
  void MathXYToLanLon(const double x, const double y,
                      const double central_meridian_radian, double& lat_radian,
                      double& lon_radian);
  // Function that computes the footpoint latitude for use
  // in converting transverse Mercator coordinates to ellipsoidal coordinates
  double FootpointLatitude(const double y);
  // Function that computes the ellipsoidal distance from the equator to
  // a point at a given latitude
  double ArcLengthOfMeridian(const double lat_radian);
  // Function that computes CentralMeridian
  double UTMCentralMeridian(const double lon);
  int zone_;
  bool southhemi_;
};

}  // namespace v2x
//}  // namespace apollo

#endif  // V2X_APP_POLICIES_COORDINATE_TRANSITION_COORDINATE_TRANSITION_H
