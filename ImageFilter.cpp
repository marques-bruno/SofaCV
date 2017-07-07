/******************************************************************************
*       SOFAOR, SOFA plugin for the Operating Room, development version       *
*                        (c) 2017 INRIA, MIMESIS Team                         *
*                                                                             *
* This program is a free software; you can redistribute it and/or modify it   *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 1.0 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: Bruno Marques and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact-mimesis@inria.fr                               *
******************************************************************************/

#include "ImageFilter.h"
#include <AcquisitOR/BaseFrameGrabber.h>
#include <opencv2/highgui.hpp>
#include "DataSliderMgr.inl"

#include <sofa/core/visual/DrawToolGL.h>

#include <sofa/helper/gl/RAII.h>
#include <sofa/helper/system/gl.h>
#include <sofa/helper/system/glut.h>

namespace sofaor
{
namespace processor
{
unsigned ImageFilter::m_window_uid = 0;

ImageFilter::ImageFilter(bool outputImage)
    : d_img(initData(
          &d_img, common::cvMat(), "img",
          "Input image, that will undergo changes through the filter.", false)),
      d_img_out(initData(&d_img_out, "img_out",
                         "Output image, holding the filter's result", false)),
      d_displayDebugWindow(initData(&d_displayDebugWindow, false, "Debug",
                                    "Display a debug window to see in live "
                                    "the changes applied to the filter")),
      d_isActive(initData(&d_isActive, true, "isActive",
                          "if false, does not call applyFilter")),
      m_outputImage(outputImage),
      m_win_name(std::to_string(m_window_uid) + "_" + getClassName())
{
  addAlias(&d_img_out, "img1_out");
  f_listening.setValue(true);
  m_window_uid++;
}

ImageFilter::~ImageFilter() {}
void ImageFilter::init()
{
  m_displayDebugDataTracker.trackData(d_displayDebugWindow);
  addInput(&d_img);
  addOutput(&d_img_out);
}

void ImageFilter::update()
{
  if (!d_isActive.getValue())
  {
    // filter inactive, out = in
    d_img_out.setValue(d_img.getValue());
    d_img_out.endEdit();
    if (d_displayDebugWindow.getValue())
    {
      cv::imshow(m_win_name, d_img_out.getValue());
      cv::waitKey(1);
    }
    return;
  }
  m_debugImage = d_img.getValue().zeros(
      d_img.getValue().rows, d_img.getValue().cols, d_img.getValue().type());
  applyFilter(d_img.getValue(), m_debugImage);
  if (!m_outputImage)
  {
    d_img_out.setValue(d_img.getValue());
    d_img_out.endEdit();
  }
  else
  {
    d_img_out.setValue(m_debugImage);
    d_img_out.endEdit();
  }
  if (d_displayDebugWindow.getValue() && !m_debugImage.empty())
  {
    cv::imshow(m_win_name, m_debugImage);
    cv::waitKey(1);
  }
}

void ImageFilter::reinit()
{
  if (m_displayDebugDataTracker.isDirty() && d_isActive.getValue())
  {
    reinitDebugWindow();
    refreshDebugWindow();
  }

  // to set needsRefresh to true
  ImplicitDataEngine::reinit();
}

void ImageFilter::reinitDebugWindow()
{
  if (!d_displayDebugWindow.getValue())
  {
    if (m_isMouseCallbackActive) cv::setMouseCallback(m_win_name, NULL, NULL);
    cv::destroyWindow(m_win_name);
    return;
  }

	// WITH QT:
	//  cv::namedWindow(m_win_name,
	//                  CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	cv::namedWindow(m_win_name, CV_WINDOW_AUTOSIZE);
	for (DSM* dmgr : m_params) dmgr->createSlider(m_win_name);
  if (m_isMouseCallbackActive)
    cv::setMouseCallback(m_win_name, &ImageFilter::_mouseCallback, this);
}

void ImageFilter::refreshDebugWindow()
{
  if (!d_displayDebugWindow.getValue()) return;
  applyFilter(d_img.getValue(), m_debugImage, true);
  if (m_debugImage.empty()) return;

  cv::imshow(m_win_name, m_debugImage);
  cv::waitKey(1);
}

void ImageFilter::activateMouseCallback()
{
  m_isMouseCallbackActive = !m_isMouseCallbackActive;
}

void ImageFilter::unregisterAllData() { m_params.clear(); }

void ImageFilter::registerData(sofa::Data<bool>* data)
{
	m_params.push_back(new ScalarSliderManager<bool>(data));
}

void ImageFilter::registerData(sofa::Data<sofa::helper::OptionsGroup>* data)
{
	m_params.push_back(new CustomSliderManager<sofa::helper::OptionsGroup, int>(data));
}

void ImageFilter::registerData(sofa::Data<int>* data, int min, int max, int step)
{
	m_params.push_back(new ScalarSliderManager<int>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<unsigned>* data, unsigned min, unsigned max,
															 unsigned step)
{
	m_params.push_back(
			new ScalarSliderManager<unsigned>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<double>* data, double min, double max,
                               double step)
{
	m_params.push_back(
			new ScalarSliderManager<double>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<float>* data, float min, float max,
                               float step)
{
	m_params.push_back(
			new ScalarSliderManager<float>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec2u>* data, unsigned min,
															 unsigned max, unsigned step)
{
	m_params.push_back(new VecSliderManager<2, unsigned>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec3u>* data, unsigned min,
															 unsigned max, unsigned step)
{
	m_params.push_back(new VecSliderManager<3, unsigned>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec4u>* data, unsigned min,
															 unsigned max, unsigned step)
{
	m_params.push_back(new VecSliderManager<4, unsigned>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec2i>* data, int min, int max,
															 int step)
{
	m_params.push_back(new VecSliderManager<2, int>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec3i>* data, int min, int max,
															 int step)
{
	m_params.push_back(new VecSliderManager<3, int>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec4i>* data, int min, int max,
															 int step)
{
	m_params.push_back(new VecSliderManager<4, int>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec2f>* data, float min,
															 float max, float step)
{
	m_params.push_back(new VecSliderManager<2, float>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec3f>* data, float min,
															 float max, float step)
{
	m_params.push_back(new VecSliderManager<3, float>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec4f>* data, float min,
															 float max, float step)
{
	m_params.push_back(new VecSliderManager<4, float>(data, min, max, step));
}

void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec2d>* data, double min,
															 double max, double step)
{
	m_params.push_back(new VecSliderManager<2, double>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec3d>* data, double min,
															 double max, double step)
{
	m_params.push_back(new VecSliderManager<3, double>(data, min, max, step));
}
void ImageFilter::registerData(sofa::Data<sofa::defaulttype::Vec4d>* data, double min,
															 double max, double step)
{
	m_params.push_back(new VecSliderManager<4, double>(data, min, max, step));
}

void ImageFilter::drawImage()
{
	common::cvMat img;
	if (d_img_out.getValue().empty())
		img = d_img.getValue();
	else
		img = d_img_out.getValue();
	std::stringstream imageString;
	imageString.write((const char*)img.data, img.total() * img.elemSize());

	glEnable(GL_TEXTURE_2D);  // enable the texture
	glDisable(GL_LIGHTING);   // disable the light

	glBindTexture(GL_TEXTURE_2D, 0);  // texture bind
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR_EXT,
							 GL_UNSIGNED_BYTE, imageString.str().c_str());
	// glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE, m_imageWidth,
	// m_imageHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_imgData.c_str() );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
									GL_LINEAR);  // Linear Filtering

	//		float eps = 0.0;
	//		float z0 = 0.0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glMatrixMode(GL_PROJECTION);  // init the projection matrix
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);  // orthogonal view
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glTexCoord2f(1, 1);
	glVertex2f(1, 0);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(0, 1);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);     // enable light
	glDisable(GL_TEXTURE_2D);  // disable texture 2D
														 // glDepthMask (GL_TRUE);		// enable zBuffer
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

}  // namespace processor
}  // namespace sofaor
