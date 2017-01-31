#ifndef SOFA_OR_ACQUISITOR_CALIBLOADER_H
#define SOFA_OR_ACQUISITOR_CALIBLOADER_H

#include "SofaORCommon/ImplicitDataEngine.h"
#include "SofaORCommon/CameraCalib.h"
#include "SofaORCommon/StereoCalib.h"
#include "SofaORCommon/cvMat.h"
#include "initplugin.h"

#include <sofa/core/objectmodel/DataFileName.h>
#include <sofa/helper/OptionsGroup.h>

#include <opencv2/core/core.hpp>

#include <map>

namespace sofa
{
namespace OR
{
namespace common
{
class CalibLoader : public ImplicitDataEngine
{
  struct CalibData
  {
    CalibData() {}
    CalibData(const defaulttype::Matrix3& _projMat1,
              const helper::vector<double>& _distCoefs1, double _error1,
              const defaulttype::Matrix3& _projMat2,
              const helper::vector<double>& _distCoefs2, double _error2,
              const defaulttype::Matrix3& _R, const defaulttype::Vector3& _T,
              const defaulttype::Matrix3& _F, double _totalError)
        : projMat1(_projMat1),
          distCoefs1(_distCoefs1),
          error1(_error1),
          projMat2(_projMat2),
          distCoefs2(_distCoefs2),
          error2(_error2),
          R(_R),
          T(_T),
          F(_F),
          totalError(_totalError)
    {
    }

    defaulttype::Matrix3 projMat1;
    helper::vector<double> distCoefs1;
    double error1;

    defaulttype::Matrix3 projMat2;
    helper::vector<double> distCoefs2;
    double error2;

    defaulttype::Matrix3 R;
    defaulttype::Vector3 T;
    defaulttype::Matrix3 F;
    double totalError;
  };

 public:
  SOFA_CLASS(CalibLoader, core::DataEngine);

  CalibLoader();
  virtual ~CalibLoader();

  void init();
  void update();
  void reinit();

  sofa::core::objectmodel::DataFileName d_calibFolder;
  Data<helper::OptionsGroup> d_calibNames;
  Data<bool> d_isStereo;

  Data<defaulttype::Matrix3> d_projMat1;
  Data<helper::vector<double> > d_distCoefs1;
  Data<double> d_error1;

  Data<defaulttype::Matrix3> d_projMat2;
  Data<helper::vector<double> > d_distCoefs2;
  Data<double> d_error2;

  Data<defaulttype::Matrix3> d_R;
  Data<defaulttype::Vector3> d_T;
  Data<defaulttype::Matrix3> d_F;
  Data<double> d_totalError;

 protected:
  std::map<std::string, CalibData> m_calibs;

  void handleEvent(sofa::core::objectmodel::Event* e);
 private:
  void load(const std::string& calibfile);
  bool canLoad(const std::string& calibfile) const;
  void setCurrentCalib(CalibData& d);
  void setCurrentCalib(const std::string& calibName);
  std::string getPathToCalibs();
  void getAllCalibFiles(std::vector<std::string>& calibFiles);
};

}  // namespace common
}  // namespace OR
}  // namespace sofa

#endif  // SOFA_OR_ACQUISITOR_CALIBLOADER_H
