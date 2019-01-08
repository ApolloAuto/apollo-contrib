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
 * @file coordinate_transition.cc
 * @brief The coordinate transition program for the v2x module
 */

#include "coordinate_transition.h"

// namespace apollo {
namespace v2x {

CoordinateTransition::CoordinateTransition()
    : zone_(FLAGS_zone), southhemi_(FLAGS_southernhemi_flag) {}

void CoordinateTransition::Init() {}

void CoordinateTransition::LatLonToUTMXY(const double lat, const double lon,
                                         double& utm_x, double& utm_y) {
  MathLanLonToXY(DegToRad(lat), DegToRad(lon), UTMCentralMeridian(lon), utm_x,
                 utm_y);
  // Adjust easting and northing for UTM system
  utm_x = utm_x * kUTMScaleFactor + kUTMXCompensation;
  utm_y = utm_y * kUTMScaleFactor;
  if (utm_y < 0.0) {
    utm_y += kUTMYCompensation;
  }
}

void CoordinateTransition::UTMXYToLatLon(double utm_x, double utm_y,
                                         double& lat, double& lon) {
  utm_x -= kUTMXCompensation;
  utm_x /= kUTMScaleFactor;
  // If in southern hemisphere, adjust y accordingly
  if (southhemi_) {
    utm_y -= kUTMYCompensation;
  }
  utm_y /= kUTMScaleFactor;
  double cmeridian = DegToRad(zone_ * 6.0 - 183.0);
  double lat_radian = 0.0;
  double lon_radian = 0.0;
  MathXYToLanLon(utm_x, utm_y, cmeridian, lat_radian, lon_radian);
  lat = RadToDeg(lat_radian);
  lon = RadToDeg(lon_radian);
}

void CoordinateTransition::ThetaToHeading(const double theta_radian,
                                          double velocity_x, double velocity_y,
                                          double& heading) {
  // Stationary state
  if (fabs(velocity_x) < 0.01) {
    velocity_x = 0.0;
  }
  if (fabs(velocity_y) < 0.01) {
    velocity_y = 0.0;
  }
  double heading_radian = 0.0;
  if (fabs(velocity_x) < 0.01 && fabs(velocity_y) < 0.01) {
    heading_radian = 0.0;
  } else if (fabs(velocity_x) < 0.01 && velocity_y > 0.0) {
    heading_radian = 0.0;
  } else if (fabs(velocity_x) < 0.01 && velocity_y < 0.0) {
    heading_radian = kPI;
  } else if (fabs(velocity_y) < 0.01 && velocity_x > 0.0) {
    heading_radian = kPI / 2;
  } else if (fabs(velocity_y) < 0.01 && velocity_x < 0.0) {
    heading_radian = (3 * kPI) / 2;
  } else if (velocity_x > 0.0 && velocity_y > 0.0) {
    heading_radian = kPI / 2 - theta_radian;
  } else if (velocity_x > 0.0 && velocity_y < 0.0) {
    heading_radian = kPI / 2 - theta_radian;
  } else if (velocity_x < 0.0 && velocity_y < 0.0) {
    heading_radian = kPI / 2 - theta_radian;
  } else if (velocity_x < 0.0 && velocity_y > 0.0) {
    heading_radian = (5 * kPI) / 2 - theta_radian;
  }
  heading = RadToDeg(heading_radian);
}

void CoordinateTransition::MathLanLonToXY(const double lat_radian,
                                          const double lon_radian,
                                          const double central_meridian_radian,
                                          double& x, double& y) {
  // Precalculate ep2
  double ep2 = (pow(kEmA, 2.0) - pow(kEmB, 2.0)) / pow(kEmB, 2.0);
  // Precalculate nu2
  double nu2 = ep2 * pow(cos(lat_radian), 2.0);
  // Precalculate N
  double n = pow(kEmA, 2.0) / (kEmB * sqrt(1 + nu2));
  // Precalculate t
  double t = tan(lat_radian);
  double t2 = pow(t, 2.0);
  double t4 = pow(t, 4.0);
  double t6 = pow(t, 6.0);
  // Precalculate l
  double l = lon_radian - central_meridian_radian;
  // Precalculate coefficients for l**n in the equations below so a normal human
  // being can read the expressions for easting and northing -- l**1 and l**2
  // have coefficients of 1.0
  double l3coef = 1.0 - t2 + nu2;
  double l4coef = 5.0 - t2 + 9 * nu2 + 4.0 * (nu2 * nu2);
  double l5coef = 5.0 - 18.0 * t2 + t4 + 14.0 * nu2 - 58.0 * t2 * nu2;
  double l6coef = 61.0 - 58.0 * t2 + t4 + 270.0 * nu2 - 330.0 * t2 * nu2;
  double l7coef = 61.0 - 479.0 * t2 + 179.0 * t4 - t6;
  double l8coef = 1385.0 - 3111.0 * t2 + 543.0 * t4 - t6;
  // Calculate easting (x)
  x = n * cos(lat_radian) * l +
      (n / 6.0 * pow(cos(lat_radian), 3.0) * l3coef * pow(l, 3.0)) +
      (n / 120.0 * pow(cos(lat_radian), 5.0) * l5coef * pow(l, 5.0)) +
      (n / 5040.0 * pow(cos(lat_radian), 7.0) * l7coef * pow(l, 7.0));
  // Calculate northing (y)
  y = ArcLengthOfMeridian(lat_radian) +
      (t / 2.0 * n * pow(cos(lat_radian), 2.0) * pow(l, 2.0)) +
      (t / 24.0 * n * pow(cos(lat_radian), 4.0) * l4coef * pow(l, 4.0)) +
      (t / 720.0 * n * pow(cos(lat_radian), 6.0) * l6coef * pow(l, 6.0)) +
      (t / 40320.0 * n * pow(cos(lat_radian), 8.0) * l8coef * pow(l, 8.0));
}

void CoordinateTransition::MathXYToLanLon(const double x, const double y,
                                          const double central_meridian_radian,
                                          double& lat_radian,
                                          double& lon_radian) {
  // Get the value of phif, the footpoint latitude
  double phif = FootpointLatitude(y);
  // Precalculate ep2
  double ep2 = (pow(kEmA, 2.0) - pow(kEmB, 2.0)) / pow(kEmB, 2.0);
  // Precalculate cos (phif)
  double cf = cos(phif);
  // Precalculate nuf2
  double nuf2 = ep2 * pow(cf, 2.0);
  // Precalculate Nf and initialize nf_pow
  double nf = pow(kEmA, 2.0) / (kEmB * sqrt(1 + nuf2));
  double nf_pow = nf;
  // Precalculate tf
  double tf = tan(phif);
  double tf2 = tf * tf;
  double tf4 = tf2 * tf2;
  // Precalculate fractional coefficients for x**n in the equations below to
  // simplify the expressions for latitude and longitude
  double x1frac = 1.0 / (nf_pow * cf);
  nf_pow *= nf; /* now equals Nf**2) */
  double x2frac = tf / (2.0 * nf_pow);
  nf_pow *= nf; /* now equals Nf**3) */
  double x3frac = 1.0 / (6.0 * nf_pow * cf);
  nf_pow *= nf; /* now equals Nf**4) */
  double x4frac = tf / (24.0 * nf_pow);
  nf_pow *= nf; /* now equals Nf**5) */
  double x5frac = 1.0 / (120.0 * nf_pow * cf);
  nf_pow *= nf; /* now equals Nf**6) */
  double x6frac = tf / (720.0 * nf_pow);
  nf_pow *= nf; /* now equals Nf**7) */
  double x7frac = 1.0 / (5040.0 * nf_pow * cf);
  nf_pow *= nf; /* now equals Nf**8) */
  double x8frac = tf / (40320.0 * nf_pow);
  // Precalculate polynomial coefficients for x**n. -- x**1 does not have a
  // polynomial coefficient
  double x2poly = -1.0 - nuf2;
  double x3poly = -1.0 - 2 * tf2 - nuf2;
  double x4poly = 5.0 + 3.0 * tf2 + 6.0 * nuf2 - 6.0 * tf2 * nuf2 -
                  3.0 * (nuf2 * nuf2) - 9.0 * tf2 * (nuf2 * nuf2);
  double x5poly = 5.0 + 28.0 * tf2 + 24.0 * tf4 + 6.0 * nuf2 + 8.0 * tf2 * nuf2;
  double x6poly =
      -61.0 - 90.0 * tf2 - 45.0 * tf4 - 107.0 * nuf2 + 162.0 * tf2 * nuf2;
  double x7poly = -61.0 - 662.0 * tf2 - 1320.0 * tf4 - 720.0 * (tf4 * tf2);
  double x8poly = 1385.0 + 3633.0 * tf2 + 4095.0 * tf4 + 1575 * (tf4 * tf2);
  // Calculate latitude
  lat_radian = phif + x2frac * x2poly * (x * x) +
               x4frac * x4poly * pow(x, 4.0) + x6frac * x6poly * pow(x, 6.0) +
               x8frac * x8poly * pow(x, 8.0);
  // Calculate longitude
  lon_radian = central_meridian_radian + x1frac * x +
               x3frac * x3poly * pow(x, 3.0) + x5frac * x5poly * pow(x, 5.0) +
               x7frac * x7poly * pow(x, 7.0);
}

double CoordinateTransition::FootpointLatitude(const double y) {
  double result = 0.0;
  // Precalculate n (Eq. 10.18)
  double n = (kEmA - kEmB) / (kEmA + kEmB);
  // Precalculate alpha_ (Eq. 10.22)
  // (Same as alpha in Eq. 10.17)
  double alpha_ =
      ((kEmA + kEmB) / 2.0) * (1 + (pow(n, 2.0) / 4) + (pow(n, 4.0) / 64));
  // Precalculate y_ (Eq. 10.23)
  double y_ = y / alpha_;
  // Precalculate beta_ (Eq. 10.22)
  double beta_ = (3.0 * n / 2.0) + (-27.0 * pow(n, 3.0) / 32.0) +
                 (269.0 * pow(n, 5.0) / 512.0);
  // Precalculate gamma_ (Eq. 10.22)
  double gamma_ = (21.0 * pow(n, 2.0) / 16.0) + (-55.0 * pow(n, 4.0) / 32.0);
  // Precalculate delta_ (Eq. 10.22)
  double delta_ = (151.0 * pow(n, 3.0) / 96.0) + (-417.0 * pow(n, 5.0) / 128.0);
  // Precalculate epsilon_ (Eq. 10.22)
  double epsilon_ = (1097.0 * pow(n, 4.0) / 512.0);
  // Now calculate the sum of the series (Eq. 10.21)
  result = y_ + (beta_ * sin(2.0 * y_)) + (gamma_ * sin(4.0 * y_)) +
           (delta_ * sin(6.0 * y_)) + (epsilon_ * sin(8.0 * y_));
  return result;
}

double CoordinateTransition::ArcLengthOfMeridian(const double lat_radian) {
  double result = 0.0;
  // Precalculate n
  double n = (kEmA - kEmB) / (kEmA + kEmB);
  // Precalculate alpha
  double alpha = ((kEmA + kEmB) / 2.0) *
                 (1.0 + (pow(n, 2.0) / 4.0) + (pow(n, 4.0) / 64.0));
  // Precalculate beta
  double beta = (-3.0 * n / 2.0) + (9.0 * pow(n, 3.0) / 16.0) +
                (-3.0 * pow(n, 5.0) / 32.0);
  // Precalculate gamma
  double gamma = (15.0 * pow(n, 2.0) / 16.0) + (-15.0 * pow(n, 4.0) / 32.0);
  // Precalculate delta
  double delta = (-35.0 * pow(n, 3.0) / 48.0) + (105.0 * pow(n, 5.0) / 256.0);
  // Precalculate epsilon
  double epsilon = (315.0 * pow(n, 4.0) / 512.0);
  // Now calculate the sum of the series and return
  result = alpha *
           (lat_radian + (beta * sin(2.0 * lat_radian)) +
            (gamma * sin(4.0 * lat_radian)) + (delta * sin(6.0 * lat_radian)) +
            (epsilon * sin(8.0 * lat_radian)));
  return result;
}

double CoordinateTransition::UTMCentralMeridian(const double lon) {
  double zone = 0.0;
  if (lon < 0.0) {
    zone = static_cast<int>((180 + lon) / 6.0) + 1;
  } else {
    zone = static_cast<int>(lon / 6) + 31;
  }
  return DegToRad(-183.0 + (zone * 6.0));
}

}  // namespace v2x
//}  // namespace apollo
