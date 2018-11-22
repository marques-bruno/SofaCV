#ifndef SOFACV_CVDMATCH_H
#define SOFACV_CVDMATCH_H

#include "SofaCVPlugin.h"

#include <sofa/defaulttype/DataTypeInfo.h>
#include <iostream>
#include <opencv2/core/types.hpp>

namespace sofacv
{
/**
 * @brief The cvDMatch class, Needed to override the stream operators for SOFA
 */
class SOFA_SOFACV_API cvDMatch : public cv::DMatch
{
 public:
  cvDMatch()
  {
    distance = -1.0f;
    imgIdx = -1;
    queryIdx = -1;
    trainIdx = -1;
  }
  cvDMatch(const cv::DMatch& o)
  {
    distance = o.distance;
    imgIdx = o.imgIdx;
    queryIdx = o.queryIdx;
    trainIdx = o.trainIdx;
  }
  cvDMatch(int _queryIdx, int _trainIdx, float _distance)
      : cv::DMatch(_queryIdx, _trainIdx, _distance)
  {
  }
  cvDMatch(int _queryIdx, int _trainIdx, int _imgIdx, float _distance)
      : cv::DMatch(_queryIdx, _trainIdx, _imgIdx, _distance)
  {
  }

  inline friend std::istream& operator>>(std::istream& in, cvDMatch& s)
  {
    in >> s.distance >> s.imgIdx >> s.queryIdx >> s.trainIdx;
    return in;
  }

  inline friend std::ostream& operator<<(std::ostream& out, const cvDMatch& s)
  {
    out << s.distance << ' ' << s.imgIdx << ' ' << s.queryIdx << ' '
        << s.trainIdx << ' ';
    return out;
  }
};

}  // namespace sofacv

namespace sofa
{
namespace defaulttype
{
/**
 *  \brief Implementation of SOFA's DataType interface to pass cv::DMatch data
 * structures as sofa::Data
 */
template <>
struct DataTypeName<sofacv::cvDMatch>
{
  static const char* name() { return "cvDMatch"; }
};

}  // namespace defaulttype
}  // namespace sofa

#endif  // SOFACV_CVDMATCH_H
