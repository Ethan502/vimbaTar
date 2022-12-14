/*=============================================================================
  Copyright (C) 2013 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ApiController.cpp

  Description: Implementation file for the ApiController helper class that
               demonstrates how to implement an asynchronous, continuous image
               acquisition with VimbaCPP.

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
#include <sstream>
#include <iostream>

#include "ApiController.h"
#include "Common/StreamSystemInfo.h"
#include "Common/ErrorCodeToMessage.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {

#define NUM_FRAMES 10

ApiController::ApiController()
    // Get a reference to the Vimba singleton
    : m_system ( VimbaSystem::GetInstance() )
{}

ApiController::~ApiController()
{
}

// Translates Vimba error codes to readable error messages
std::string ApiController::ErrorCodeToMessage( VmbErrorType eErr ) const
{
    return AVT::VmbAPI::Examples::ErrorCodeToMessage( eErr );
}

VmbErrorType ApiController::StartUp()
{
    return m_system.Startup();
}

void ApiController::ShutDown()
{
    // Release Vimba
    m_system.Shutdown();
}

VmbErrorType ApiController::StartContinuousImageAcquisition( const ProgramConfig &Config )
{
    // Open the desired camera by its ID
    VmbErrorType res = m_system.OpenCameraByID( Config.getCameraID().c_str(), VmbAccessModeFull, m_pCamera );
    if ( VmbErrorSuccess == res )
    {
        // Set the GeV packet size to the highest possible value
        // (In this example we do not test whether this cam actually is a GigE cam)
        FeaturePtr pCommandFeature;
        if ( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
        {
            if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
            {
                bool bIsCommandDone = false;
                do
                {
                    if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
                    {
                        break;
                    }
                } while ( false == bIsCommandDone );
            }
        }

        if ( VmbErrorSuccess == res )
        {
            // set camera so that transform algorithmens will never fail
            PrepareCamera();
            // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
            m_pFrameObserver = new FrameObserver( m_pCamera, Config.getFrameInfos(), Config.getColorProcessing() );
            // Start streaming
            res = m_pCamera->StartContinuousImageAcquisition( NUM_FRAMES, IFrameObserverPtr( m_pFrameObserver ));
        }
    }

    return res;
}

//sunny modify it, so that it can support multi-camera streaming

/**setting a feature to maximum value that is a multiple of 2*/
VmbErrorType SetIntFeatureValueModulo2( const CameraPtr &pCamera, const char* const& Name )
{
    VmbErrorType    result;
    FeaturePtr      feature;
    VmbInt64_t      value_min,value_max;
    result = SP_ACCESS( pCamera )->GetFeatureByName( Name, feature );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    result = SP_ACCESS( feature )->GetRange( value_min, value_max );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    value_max =( value_max>>1 )<<1;
    result = SP_ACCESS( feature )->SetValue ( value_max );
    return result;
}
/**prepare camera so that the delivered image will not fail in image transform*/
VmbErrorType ApiController::PrepareCamera()
{
    VmbErrorType result;
    result = SetIntFeatureValueModulo2( m_pCamera, "Width" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    result = SetIntFeatureValueModulo2( m_pCamera, "Height" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    return result;
}
VmbErrorType ApiController::StopContinuousImageAcquisition()
{
    // Stop streaming
    m_pCamera->StopContinuousImageAcquisition();

    // Close camera
    return  m_pCamera->Close();
}

// sunny add for multi-camera support
#if MULTI_CAMERA_SUPPORT
/**prepare camera so that the delivered image will not fail in image transform*/
VmbErrorType ApiController::PrepareCamera(CameraPtr pCamera)
{
    VmbErrorType result;
    result = SetIntFeatureValueModulo2( pCamera, "Width" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    result = SetIntFeatureValueModulo2( pCamera, "Height" );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    return result;
}

VmbErrorType ApiController::StartMulticamContinuousImageAcquisition( AVT::VmbAPI::CameraPtrVector cameras)
{
	// wait to do -- try to get cameras number, then allocate the resource for them
	// Wait until all the cameras have been closed.
	VmbErrorType res;
	std::vector<CameraPtr>::iterator itr;
	for(itr = cameras.begin(); itr != cameras.end(); itr++)
	{
		std::string strCameraID;
        res = (*itr)->GetID( strCameraID );
		if ( VmbErrorSuccess != res )
		{
			continue;
		}

		// Open the desired camera by its ID
		res = m_system.OpenCameraByID( strCameraID.c_str(), VmbAccessModeFull, m_pCamera );
		if ( VmbErrorSuccess == res )
		{
			std::cout << strCameraID.c_str() << "\n";
			// Set the GeV packet size to the highest possible value
			// (In this example we do not test whether this cam actually is a GigE cam)
			FeaturePtr pCommandFeature;
			if ( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
			{
				if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
				{
					bool bIsCommandDone = false;
					do
					{
						if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
						{
							break;
						}
					} while ( false == bIsCommandDone );
				}
			}

			if ( VmbErrorSuccess == res )
			{
				// set camera so that transform algorithmens will never fail
				PrepareCamera(m_pCamera);
				// Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
				m_pFrameObserver = new FrameObserver( m_pCamera, FrameInfos_Off, ColorProcessing_Off );
				// Start streaming
				res = m_pCamera->StartContinuousImageAcquisition( NUM_FRAMES, IFrameObserverPtr( m_pFrameObserver ));

				// wait to do
				m_camObservers.push_back(m_pFrameObserver);
				m_OpenCameras.push_back(m_pCamera);
			}
		}
	}

    return res;
}

VmbErrorType ApiController::StopMulticamContinuousImageAcquisition(AVT::VmbAPI::CameraPtrVector cameras)
{
	// Wait until all the cameras have been closed.
	VmbErrorType res;

	std::vector<CameraPtr>::iterator itr;
	for(itr = cameras.begin(); itr != cameras.end(); itr++)
	{
		// Stop streaming
		(*itr)->StopContinuousImageAcquisition();

		// Close camera
		res = (*itr)->Close();
	}
	return res;
}
#endif

CameraPtrVector ApiController::GetCameraList() const
{
    CameraPtrVector cameras;
    // Get all known cameras
    if ( VmbErrorSuccess == m_system.GetCameras( cameras ))
    {
        // And return them
        return cameras;
    }
    return CameraPtrVector();
}
std::string ApiController::GetVersion() const
{
    std::ostringstream  os;
    os<<m_system;
    return os.str();
}
}}} // namespace AVT::VmbAPI::Examples
