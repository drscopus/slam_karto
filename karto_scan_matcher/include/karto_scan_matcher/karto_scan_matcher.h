/************************************************************
 * \file
 * 
 * Ros wrapper for the Karto scan matcher
 *
 * \author Bhaskara Marthi
 ************************************************************/

#ifndef KARTO_SCAN_MATCHER_KARTO_SCAN_MATCHER_H
#define KARTO_SCAN_MATCHER_KARTO_SCAN_MATCHER_H

#include <karto/Mapper.h>
// Undo the various defines from the karto headers to be safe
#undef forEach
#undef forEachAs
#undef const_forEach
#undef const_forEachAs
#undef forEachR
#undef const_forEachR

#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Pose2D.h>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>




namespace karto_scan_matcher
{

using std::vector;

/// Some parameters of the scan matching
const double DEFAULT_SMEAR_DEVIATION = 0.03;
const double DEFAULT_RANGE_THRESHOLD = 12.0;


/// \brief Represents a 2d scan at some pose
/// 
/// The frame_id of \a scan is ignored.  It's treated instead as being in the sensor frame
/// corresponding to the base frame being at \a pose.
struct ScanWithPose
{
  ScanWithPose (const sensor_msgs::LaserScan& scan, const geometry_msgs::Pose2D& pose)
    : scan(scan), pose(pose) {}
  ScanWithPose () {}

  sensor_msgs::LaserScan scan;
  geometry_msgs::Pose2D pose;
};

typedef std::pair<geometry_msgs::Pose2D, double> ScanMatchResult;
typedef std::vector<double> DoubleVector;

/// \brief Wraps the Karto Scan matcher
///
/// To use it, just construct an instance and call scanMatch as many times as you want.
/// The main constraint is that laser offset and frame should be the same for all scans passed into
/// the constructor and to scanMatch.
///
/// All scans are assumed to be in the base_laser_link frame, and poses are of the base_link frame.
class KartoScanMatcher 
{
public:
  
  /// \brief Constructor
  /// \param init_scan A scan that's used to set the parameters of the laser
  /// \param tf Used to figure out the offset of the laser (we're assuming this will be fixed henceforth)
  /// \param name Prefixed to the sensor name (since all karto instances in this process have a global namespace)
  /// \param search_space_size Size in meters of space to search over centered at initial estimate
  /// \param search_grid_resolution Resolution (m/cell) of discretization of search space
  KartoScanMatcher(const sensor_msgs::LaserScan& init_scan, const tf::TransformListener& tf,
                   double search_space_size, double search_grid_resolution);

  /// \brief Constructor when laser relative pose is known
  /// \param init_scan A scan that's used to set the parameters of the laser
  /// \param name Prefixed to the sensor name (since all karto instances in this process have a global namespace)
  /// \param laser_pose Relative pose of the laser w.r.t. the base
  /// \param search_space_size Size in meters of space to search over centered at initial estimate
  /// \param search_grid_resolution Resolution (m/cell) of discretization of search space
  KartoScanMatcher(const sensor_msgs::LaserScan& init_scan, const geometry_msgs::Pose2D& laser_pose,
                   double search_space_size, double search_grid_resolution);

  /// \brief Constructor when we have multiple matchers we want to use in sequence (increasingly fine)
  /// \param init_scan A scan that's used to set the parameters of the laser
  /// \param laser_pose Relative pose of the laser w.r.t. the base
  /// \param name Prefixed to the sensor name (since all karto instances in this process have a global namespace)
  /// \param search_space_sizes The sizes in meters of space to search over at each level (coarse to fine)
  /// \param search_grid_resolutions Resolutions (m/cells) of discretizations of search space
  /// (match up with the ones in search_space_sizes)
  KartoScanMatcher(const sensor_msgs::LaserScan& init_scan, const geometry_msgs::Pose2D& laser_pose,
                   const DoubleVector& search_sizes, const DoubleVector& search_resolutions);


  /// \brief Match scans
  /// \param scan The current laser scan.
  /// \param pose The initial estimate of the pose at which this scan was taken.
  /// \param reference_scans Each of these is a scan, together with the pose at which it was taken
  /// \retval a pair consisting of the optimal pose, and response value 
  ScanMatchResult scanMatch (const sensor_msgs::LaserScan& scan, const geometry_msgs::Pose2D& pose,
                             const vector<ScanWithPose>& reference_scans) const;
  
private:

  void initialize (const sensor_msgs::LaserScan& init_scan, const geometry_msgs::Pose2D& laser_pose,
                   const DoubleVector& search_space_sizes, const DoubleVector& search_space_resolutions);

  typedef boost::shared_ptr<karto::ScanMatcher> MatcherPtr;
  typedef boost::shared_ptr<karto::Mapper> MapperPtr;

  boost::shared_ptr<karto::Dataset> dataset_;
  std::vector<MatcherPtr> matchers_;
  std::vector<MapperPtr> mappers_;
  karto::LaserRangeFinder* laser_; // memory managed by dataset_
  
};


} // namespace karto_scan_matcher



#endif // include guard
