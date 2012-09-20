// AcquireSingleFrame.cpp
//
// This application grabs images from a Basler 1394 or Basler GigE camera
// using the SingleFrame acquisition mode, i.e., the acquisition of each
// single image must be initiated by the application.
// The PYLON API framework is used to access the camera and to grab the
// acquired images into user memory.

// Include files to use the PYLON API
#include <pylon/PylonIncludes.h>
using namespace Pylon;


#if defined( USE_1394 )
// Settings to use  Basler 1394 cameras
#include <pylon/1394/Basler1394Camera.h>
typedef Pylon::CBasler1394Camera Camera_t;
using namespace Basler_IIDC1394CameraParams;
using namespace Basler_IIDC1394StreamParams;
#elif defined ( USE_GIGE )
// Settings to use Basler GigE cameras
#include <pylon/gige/BaslerGigECamera.h>
typedef Pylon::CBaslerGigECamera Camera_t;
using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;
#else
#error Camera type is not specified. For example, define USE_GIGE for using GigE cameras
#endif

#include <fstream>

// Signal Handling
#include <stdio.h>     /* standard I/O functions                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Global because must be acessed by click (signal)
Camera_t * pCamera;
Camera_t::StreamGrabber_t * pStreamGrabber;

// Namespace for using cout
using namespace std;

void ignoring(int sig_num) {
  cout << "click: I am taking a picture already." << " "
       << "Please wait...";
}

void awaking(int sig_num) {
  cout << "click: I am awaking already." << " "
       << "Please wait...";
}

void click(int sig_num) {
  /* If another SIGINT comes, ignore it */
  signal(SIGINT, ignoring);

  cout << "Taking picture..."; fflush(stdout);

  // Let the camera acquire one single image ( Acquisiton mode equals
  // SingleFrame! )
  pCamera->AcquisitionStart.Execute();

  // Wait for the grabbed image with a timeout of 3 seconds
  if (pStreamGrabber->GetWaitObject().Wait(3000))
    {
      // Get the grab result from the grabber's result queue
      GrabResult Result;
      pStreamGrabber->RetrieveResult(Result);

      if (Result.Succeeded())
	{
	  // Grabbing was successful, process image
	  cout << "Image acquired!" << endl;
	  cout << "Size: " << Result.GetSizeX() << " x "
	       << Result.GetSizeY() << endl;

	  // Get the pointer to the image buffer
	  const uint8_t *pImageBuffer = (uint8_t *) Result.Buffer();
                    
	  // cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0]
	  // << endl << endl;
                    
	  // Save to disk
	  ofstream outpgm("saida.pgm");
	  outpgm << "P2" << std::endl
		 << Result.GetSizeX() << " " << Result.GetSizeY() << std::endl
		 << "255" << std::endl;
	  for (int oa = 0; oa < Result.GetSizeY(); oa++) {
	    for (int oo = 0; oo < Result.GetSizeX(); oo++) {
	      outpgm << (int)pImageBuffer[oo + Result.GetSizeX()*oa] << " ";
	    }
	    outpgm << std::endl;
	  }
	  outpgm.close();

	  // Reuse the buffer for grabbing the next image
          pStreamGrabber->QueueBuffer(Result.Handle(), NULL);
	}
      else
	{
	  // Error handling
	  cerr << "No image acquired!" << endl;
	  cerr << "Error code : 0x" << hex
	       << Result.GetErrorCode() << endl;
	  cerr << "Error description : "
	       << Result.GetErrorDescription() << endl;
	}
    }
  else
    {
      // Timeout
      cerr << "Timeout occurred!" << endl;

      // Get the pending buffer back (You are not allowed to deregister
      // buffers when they are still queued)
      pStreamGrabber->CancelGrab();

      // Get all buffers back
      for (GrabResult r; pStreamGrabber->RetrieveResult(r););
    }

  cout << "Picture taken." << endl; fflush(stdout);
  
  cout << "Calling script <walle.sh>" << endl; fflush(stdout);

  // This script can be used to do stuff to the picture taken,
  // like conversion, renaming, etc.
  system("./walle.sh &");

  /* Now I can take another picture */
  signal(SIGINT, click);
}

// This function can be used to wait for user input at the end of the sample program.
void pressEnterToExit()
{
  //comment the following two lines to disable wait on exit here
  // cerr << endl << "Press enter to exit." << endl;
  // while( cin.get() != '\n');
}

int main(int argc, char* argv[])
{
  /* set the INT (Ctrl-C) signal handler to 'click' */
  signal(SIGINT, awaking);

  // Automagically call PylonInitialize and PylonTerminate to ensure that the pylon runtime
  // system is initialized during the lifetime of this object
  Pylon::PylonAutoInitTerm autoInitTerm;

  try
    {
      cout << "Get the transport layer factory " << endl;
      CTlFactory& TlFactory = CTlFactory::GetInstance();

      cout << "Create the transport layer object needed to enumerate or create a camera object of type Camera_t::DeviceClass()" << endl;
      ITransportLayer *pTl = TlFactory.CreateTl(Camera_t::DeviceClass());

      // Exit the application if the specific transport layer is not available
      if (! pTl)
        {
	  cerr << "Failed to create transport layer!" << endl;
	  pressEnterToExit();
	  return 1;
        }

      cout << "Get all attached cameras and exit the application if no camera is found" << endl;
      DeviceInfoList_t devices;
      if (0 == pTl->EnumerateDevices(devices))
        {
	  cerr << "No camera present!" << endl;
	  pressEnterToExit();
	  return 1;
        }

      cout "click: " << " Create the camera object of the first available camera.  The camera object is used to set and get all available camera features." << endl;
      pCamera = new Camera_t(pTl->CreateDevice(devices[0]));

      cout "click: " << " Open the camera" << endl;
      pCamera->Open();

      cout << "click: " << "Get the first stream grabber object of the selected camera" << endl;
      pStreamGrabber = new Camera_t::StreamGrabber_t(pCamera->GetStreamGrabber(0));

      cout << "click: " << "Open the stream grabber" << endl;
      pStreamGrabber->Open();

      // Set the image format and AOI
      //Camera.PixelFormat.SetValue(PixelFormat_Mono8);
      //Camera.PixelFormat.SetValue(PixelFormat_YUV422Packed);
      pCamera->PixelFormat.SetValue(PixelFormat_BayerBG8);
      pCamera->OffsetX.SetValue(0);
      pCamera->OffsetY.SetValue(0);
      pCamera->Width.SetValue(pCamera->Width.GetMax());
      pCamera->Height.SetValue(pCamera->Height.GetMax());

      cout << "click: " << "Disable acquisition start trigger if available" << endl;
      {
	GenApi::IEnumEntry* acquisitionStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_AcquisitionStart);
	if ( acquisitionStart && GenApi::IsAvailable( acquisitionStart))
	  {
	    pCamera->TriggerSelector.SetValue( TriggerSelector_AcquisitionStart);
	    pCamera->TriggerMode.SetValue( TriggerMode_Off);
	  }
      }

      cout << "click: " << "Disable frame start trigger if available" << endl;
      {
	GenApi::IEnumEntry* frameStart = pCamera->TriggerSelector.GetEntry( TriggerSelector_FrameStart);
	if ( frameStart && GenApi::IsAvailable( frameStart))
	  {
	    pCamera->TriggerSelector.SetValue( TriggerSelector_FrameStart);
	    pCamera->TriggerMode.SetValue( TriggerMode_Off);
	  }
      }

      cout << "click: " << "Set acquisition mode" << endl;
      pCamera->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);

      cout << "click: " << "Set exposure settings" << endl;
      pCamera->ExposureMode.SetValue(ExposureMode_Timed);
      pCamera->ExposureTimeRaw.SetValue(30000);

      cout << "click: " << " Create an image buffer" << endl;
      const size_t ImageSize = (size_t)(pCamera->PayloadSize.GetValue());
      uint8_t * const pBuffer = new uint8_t[ ImageSize ];

      cout << "click: " << " We won't use image buffers greater than ImageSize" << endl;
      pStreamGrabber->MaxBufferSize.SetValue(ImageSize);

      cout << "click: " << " We won't queue more than one image buffer at a time" << endl;
      pStreamGrabber->MaxNumBuffer.SetValue(1);

      cout << "click: " << "Setting Socket Buffer Size" << endl;
      pStreamGrabber->SocketBufferSize.SetValue(127);

      cout << "click: " << " Allocate all resources for grabbing. Critical parameters like image size now must not be changed until FinishGrab() is called." << endl;
      pStreamGrabber->PrepareGrab();

      cout << "click: " << " Buffers used for grabbing must be registered at the stream grabber.  The registration returns a handle to be used for queuing the buffer." << endl;
      const StreamBufferHandle hBuffer =
	pStreamGrabber->RegisterBuffer(pBuffer, ImageSize);

      cout << "click: " << " Put the buffer into the grab queue for grabbing" << endl;
      pStreamGrabber->QueueBuffer(hBuffer, NULL);

      /* Now I can take a picture */
      signal(SIGINT, click);
	
      // Wait for signal to take a picture
      /* now, lets get into an infinite loop of doing nothing. */
      cout << "click: Ready to shoot..."; fflush(stdout);
      for ( ;; )
	pause();

      // Clean up

      // You must deregister the buffers before freeing the memory
      pStreamGrabber->DeregisterBuffer(hBuffer);

      // Free all resources used for grabbing
      pStreamGrabber->FinishGrab();

      // Close stream grabber
      pStreamGrabber->Close();

      // Close camera
      pCamera->Close();


      // Free memory of image buffer
      delete[] pBuffer;
    }
  catch (GenICam::GenericException &e)
    {
      // Error handling
      cerr << "An exception occurred!" << endl
	   << e.GetDescription() << endl;
      pressEnterToExit();
      return 1;
    }

  // Quit the application
  //    pressEnterToExit();
  return 0;
}
