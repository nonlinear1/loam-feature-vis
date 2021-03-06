// Copyright 2013, Ji Zhang, Carnegie Mellon University
// Further contributions copyright (c) 2016, Southwest Research Institute
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is an implementation of the algorithm described in the following paper:
//   J. Zhang and S. Singh. LOAM: Lidar Odometry and Mapping in Real-time.
//     Robotics: Science and Systems Conference (RSS). Berkeley, CA, July 2014.

#ifndef LOAM_LASERODOMETRY_H
#define LOAM_LASERODOMETRY_H

#include "common.h"
#include "Twist.h"
#include "nanoflann_pcl.h"
#include "Parameters.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/io.h>
//#include <tf/transform_datatypes.h>
//#include <tf/transform_broadcaster.h>


namespace loam {

/** \brief Implementation of the LOAM laser odometry component.
 *
 */
class LaserOdometry {
public:
  explicit LaserOdometry(const LaserOdometryParams& params = LaserOdometryParams());

  /** \brief Process incoming messages in a loop until shutdown (used in active mode). */
  void spin(const pcl::PointCloud<pcl::PointXYZI>::Ptr&,
            const pcl::PointCloud<pcl::PointXYZI>::Ptr&,
            const pcl::PointCloud<pcl::PointXYZI>::Ptr&,
            const pcl::PointCloud<pcl::PointXYZI>::Ptr&,
            const pcl::PointCloud<pcl::PointXYZI>::Ptr&,
            Twist _transform,
            Time timestamp);

  /** \brief Try to process buffered data. */
  bool process();

  bool generateRegisteredCloud(pcl::PointCloud<pcl::PointXYZI>::Ptr& registered_cloud);


  pcl::PointCloud<pcl::PointXYZI>::Ptr& cornerPointsSharp() {
    return _cornerPointsSharp;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& cornerPointsLessSharp() {
    return _cornerPointsLessSharp;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& surfPointsFlat() {
    return _surfPointsFlat;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& surfPointsLessFlat() {
    return _surfPointsLessFlat;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& laserCloudFullRes() {
    return _laserCloudFullRes;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& lastCornerCloud() {
    return _lastCornerCloud;
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr& lastSurfaceCloud() {
    return _lastSurfaceCloud;
  }

  LaserOdometryParams& params() {
    return _params;
  }

  Twist& transformSum() {
    return _transformSum;
  }

  Angle& imuRollStart() {
    return _imuRollStart;
  }

  Angle& imuPitchStart() {
    return _imuPitchStart;
  }

  Angle& imuYawStart() {
    return _imuYawStart;
  }

  Angle& imuRollEnd() {
    return _imuRollEnd;
  }

  Angle& imuPitchEnd() {
    return _imuPitchEnd;
  }

  Angle& imuYawEnd() {
    return _imuYawEnd;
  }

  Vector3& imuShiftFromStart() {
    return _imuShiftFromStart;
  }

  Vector3& imuVeloFromStart() {
    return _imuVeloFromStart;
  }

  Time& timeCornerPointsSharp() {
    return _timeCornerPointsSharp;
  }

  Time& timeCornerPointsLessSharp() {
    return _timeCornerPointsLessSharp;
  }

  Time& timeSurfPointsFlat() {
    return _timeSurfPointsFlat;
  }

  Time& timeSurfPointsLessFlat() {
    return _timeSurfPointsLessFlat;
  }

  Time& timeLaserCloudFullRes() {
    return _timeLaserCloudFullRes;
  }

  Time& timeImuTrans() {
    return _timeImuTrans;
  }

  bool& newCornerPointsSharp() {
    return _newCornerPointsSharp;
  }

  bool& newCornerPointsLessSharp() {
    return _newCornerPointsLessSharp;
  }

  bool& newSurfPointsFlat() {
    return _newSurfPointsFlat;
  }

  bool& newSurfPointsLessFlat() {
    return _newSurfPointsLessFlat;
  }

  bool& newLaserCloudFullRes() {
    return _newLaserCloudFullRes;
  }

  bool& newImuTrans() {
    return _newImuTrans;
  }

protected:
  /** \brief Reset flags, etc. */
  void reset();

  /** \brief Check if all required information for a new processing step is available. */
  bool hasNewData();

  /** \brief Transform the given point to the start of the sweep.
   *
   * @param pi the point to transform
   * @param po the point instance for storing the result
   */
  void transformToStart(const pcl::PointXYZI& pi,
                        pcl::PointXYZI& po);

  /** \brief Transform the given point cloud to the end of the sweep.
   *
   * @param cloud the point cloud to transform
   */
  size_t transformToEnd(pcl::PointCloud<pcl::PointXYZI>::Ptr& cloud);

  void pluginIMURotation(const Angle& bcx, const Angle& bcy, const Angle& bcz,
                         const Angle& blx, const Angle& bly, const Angle& blz,
                         const Angle& alx, const Angle& aly, const Angle& alz,
                         Angle &acx, Angle &acy, Angle &acz);

  void accumulateRotation(Angle cx, Angle cy, Angle cz,
                          Angle lx, Angle ly, Angle lz,
                          Angle &ox, Angle &oy, Angle &oz);
  

private:

  LaserOdometryParams _params;

  bool _systemInited;      ///< initialization flag
  long _frameCount;        ///< number of processed frames

  Time _timeCornerPointsSharp;      ///< time of current sharp corner cloud
  Time _timeCornerPointsLessSharp;  ///< time of current less sharp corner cloud
  Time _timeSurfPointsFlat;         ///< time of current flat surface cloud
  Time _timeSurfPointsLessFlat;     ///< time of current less flat surface cloud
  Time _timeLaserCloudFullRes;      ///< time of current full resolution cloud
  Time _timeImuTrans;               ///< time of current IMU transformation information

  bool _newCornerPointsSharp;       ///< flag if a new sharp corner cloud has been received
  bool _newCornerPointsLessSharp;   ///< flag if a new less sharp corner cloud has been received
  bool _newSurfPointsFlat;          ///< flag if a new flat surface cloud has been received
  bool _newSurfPointsLessFlat;      ///< flag if a new less flat surface cloud has been received
  bool _newLaserCloudFullRes;       ///< flag if a new full resolution cloud has been received
  bool _newImuTrans;                ///< flag if a new IMU transformation information cloud has been received

  pcl::PointCloud<pcl::PointXYZI>::Ptr _cornerPointsSharp;      ///< sharp corner points cloud
  pcl::PointCloud<pcl::PointXYZI>::Ptr _cornerPointsLessSharp;  ///< less sharp corner points cloud
  pcl::PointCloud<pcl::PointXYZI>::Ptr _surfPointsFlat;         ///< flat surface points cloud
  pcl::PointCloud<pcl::PointXYZI>::Ptr _surfPointsLessFlat;     ///< less flat surface points cloud
  pcl::PointCloud<pcl::PointXYZI>::Ptr _laserCloudFullRes;             ///< full resolution cloud

  pcl::PointCloud<pcl::PointXYZI>::Ptr _lastCornerCloud;    ///< last corner points cloud
  pcl::PointCloud<pcl::PointXYZI>::Ptr _lastSurfaceCloud;   ///< last surface points cloud

  pcl::PointCloud<pcl::PointXYZI>::Ptr _laserCloudOri;      ///< point selection
  pcl::PointCloud<pcl::PointXYZI>::Ptr _coeffSel;           ///< point selection coefficients

  nanoflann::KdTreeFLANN<pcl::PointXYZI>::Ptr _lastCornerKDTree;   ///< last corner cloud KD-tree
  nanoflann::KdTreeFLANN<pcl::PointXYZI>::Ptr _lastSurfaceKDTree;  ///< last surface cloud KD-tree


  std::vector<int> _pointSearchCornerInd1;    ///< first corner point search index buffer
  std::vector<int> _pointSearchCornerInd2;    ///< second corner point search index buffer

  std::vector<int> _pointSearchSurfInd1;    ///< first surface point search index buffer
  std::vector<int> _pointSearchSurfInd2;    ///< second surface point search index buffer
  std::vector<int> _pointSearchSurfInd3;    ///< third surface point search index buffer

  Twist _transform;     ///< optimized pose transformation
  Twist _transformSum;  ///< accumulated optimized pose transformation

  Angle _imuRollStart, _imuPitchStart, _imuYawStart;
  Angle _imuRollEnd, _imuPitchEnd, _imuYawEnd;

  Vector3 _imuShiftFromStart;
  Vector3 _imuVeloFromStart;

};

} // end namespace loam

#endif //LOAM_LASERODOMETRY_H
