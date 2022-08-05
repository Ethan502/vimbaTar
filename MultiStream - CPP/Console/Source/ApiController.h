/*=============================================================================
  Copyright (C) 2013 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ApiController.h

  Description: Header file for the ApiController helper class that demonstrates
               how to implement an asynchronous, continuous image acquisition
               with VimbaCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef AVT_VMBAPI_EXAMPLES_APICONTROLLER
#define AVT_VMBAPI_EXAMPLES_APICONTROLLER

#include <string>

#include "VimbaCPP/Include/VimbaCPP.h"

#include "FrameObserver.h"

//Sunny add for debug
#define MULTI_CAMERA_SUPPORT	1

namespace AVT {
namespace VmbAPI {
namespace Examples {

class ApiController
{
  public:
    ApiController();
    ~ApiController();

    VmbErrorType        StartUp();
    void                ShutDown();

    VmbErrorType        StartContinuousImageAcquisition( const ProgramConfig & );
    VmbErrorType        StopContinuousImageAcquisition();

    CameraPtrVector     GetCameraList() const;
    std::string         ErrorCodeToMessage( VmbErrorType eErr ) const;
    std::string         GetVersion() const;

#if MULTI_CAMERA_SUPPORT
VmbErrorType ApiController::PrepareCamera(CameraPtr pCamera);
VmbErrorType ApiController::StartMulticamContinuousImageAcquisition( AVT::VmbAPI::CameraPtrVector cameras);
VmbErrorType ApiController::StopMulticamContinuousImageAcquisition(AVT::VmbAPI::CameraPtrVector cameras);
#endif

  private:
    VmbErrorType        PrepareCamera();
    VimbaSystem &       m_system;                   // A reference to our Vimba singleton
    CameraPtr           m_pCamera;                  // The currently streaming camera
    FrameObserver*      m_pFrameObserver;           // Every camera has its own frame observer

	// sunny add for multi-camera support
#if MULTI_CAMERA_SUPPORT
	CameraPtrVector m_OpenCameras;
	std::vector<FrameObserver *> m_camObservers;
#endif

};

}}} // namespace AVT::VmbAPI::Examples

#endif
