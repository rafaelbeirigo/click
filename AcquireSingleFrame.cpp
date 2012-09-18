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

// Salvar a figura
#include <fstream>

// Signal Handling
#include <stdio.h>     /* standard I/O functions                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */


Camera_t * pCamera;
Camera_t::StreamGrabber_t * pStreamGrabber;

// Namespace for using cout
using namespace std;

void click(int sig_num) {
  // Grab 10 times
  const uint32_t numGrabs = 1;

  cout << "teste";
  fflush(stdout);

  /* re-set the signal handler again to catch_int, for next time */
  signal(SIGINT, click);

  for (int n = 0; n < numGrabs; n++)
    {
      // Let the camera acquire one single image ( Acquisiton mode equals
      // SingleFrame! )
      pCamera->AcquisitionStart.Execute();
      return;

      // Wait for the grabbed image with a timeout of 3 seconds
      if (pStreamGrabber->GetWaitObject().Wait(3000))
	{
	  // Get the grab result from the grabber's result queue
	  GrabResult Result;
	  pStreamGrabber->RetrieveResult(Result);

	  if (Result.Succeeded())
	    {
	      // Grabbing was successful, process image
	      cout << "Image #" << n << " acquired!" << endl;
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
	      if (n < numGrabs - 1)
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

	      // Cancel loop
	      break;
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

	  // Cancel loop
	  break;
	}
    }
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
    signal(SIGINT, click);

    // now, lets get into an infinite loop of doing nothing.
    // for ( ;; )
    //     pause();

    // Automagically call PylonInitialize and PylonTerminate to ensure that the pylon runtime
    // system is initialized during the lifetime of this object
    Pylon::PylonAutoInitTerm autoInitTerm;

    try
    {
        // Get the transport layer factory
        CTlFactory& TlFactory = CTlFactory::GetInstance();

        // Create the transport layer object needed to enumerate or
        // create a camera object of type Camera_t::DeviceClass()
        ITransportLayer *pTl = TlFactory.CreateTl(Camera_t::DeviceClass());

        // Exit the application if the specific transport layer is not available
        if (! pTl)
        {
            cerr << "Failed to create transport layer!" << endl;
            pressEnterToExit();
            return 1;
        }

        // Get all attached cameras and exit the application if no camera is found
        DeviceInfoList_t devices;
        if (0 == pTl->EnumerateDevices(devices))
        {
            cerr << "No camera present!" << endl;
            pressEnterToExit();
            return 1;
        }

        // Create the camera object of the first available camera.
        // The camera object is used to set and get all available
        // camera features.
        pCamera = new Camera_t(pTl->CreateDevice(devices[0]));

	// cout << "vai";
	// fflush(stdout);
	// pCamera->AcquisitionStart.Execute();
	// cout << "foi";
	// fflush(stdout);

        // Open the camera
        Camera.Open();

        // Get the first stream grabber object of the selected camera
        Camera_t::StreamGrabber_t StreamGrabber(Camera.GetStreamGrabber(0));
	pStreamGrabber = &StreamGrabber;

        // Open the stream grabber
        StreamGrabber.Open();

        // Set the image format and AOI
        //Camera.PixelFormat.SetValue(PixelFormat_Mono8);
	//Camera.PixelFormat.SetValue(PixelFormat_YUV422Packed);
	Camera.PixelFormat.SetValue(PixelFormat_BayerBG8);
        Camera.OffsetX.SetValue(0);
        Camera.OffsetY.SetValue(0);
        Camera.Width.SetValue(Camera.Width.GetMax());
        Camera.Height.SetValue(Camera.Height.GetMax());

        //Disable acquisition start trigger if available
        {
            GenApi::IEnumEntry* acquisitionStart = Camera.TriggerSelector.GetEntry( TriggerSelector_AcquisitionStart);
            if ( acquisitionStart && GenApi::IsAvailable( acquisitionStart))
            {
                Camera.TriggerSelector.SetValue( TriggerSelector_AcquisitionStart);
                Camera.TriggerMode.SetValue( TriggerMode_Off);
            }
        }

        //Disable frame start trigger if available
        {
            GenApi::IEnumEntry* frameStart = Camera.TriggerSelector.GetEntry( TriggerSelector_FrameStart);
            if ( frameStart && GenApi::IsAvailable( frameStart))
            {
                Camera.TriggerSelector.SetValue( TriggerSelector_FrameStart);
                Camera.TriggerMode.SetValue( TriggerMode_Off);
            }
        }

        //Set acquisition mode
        Camera.AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);

        //Set exposure settings
        Camera.ExposureMode.SetValue(ExposureMode_Timed);
        Camera.ExposureTimeRaw.SetValue(30000);

        // Create an image buffer
        const size_t ImageSize = (size_t)(Camera.PayloadSize.GetValue());
        uint8_t * const pBuffer = new uint8_t[ ImageSize ];

        // We won't use image buffers greater than ImageSize
        StreamGrabber.MaxBufferSize.SetValue(ImageSize);

        // We won't queue more than one image buffer at a time
        StreamGrabber.MaxNumBuffer.SetValue(1);

	// Rafael: tive que setar essa opcao para a camera funcionar
        StreamGrabber.SocketBufferSize.SetValue(127);

        // Allocate all resources for grabbing. Critical parameters like image
        // size now must not be changed until FinishGrab() is called.
        StreamGrabber.PrepareGrab();

        // Buffers used for grabbing must be registered at the stream grabber.
        // The registration returns a handle to be used for queuing the buffer.
        const StreamBufferHandle hBuffer =
            StreamGrabber.RegisterBuffer(pBuffer, ImageSize);

        // Put the buffer into the grab queue for grabbing
        StreamGrabber.QueueBuffer(hBuffer, NULL);

	// Wait for signal to take a picture
	/* now, lets get into an infinite loop of doing nothing. */
	for ( ;; )
	  pause();

        // Clean up

        // You must deregister the buffers before freeing the memory
        StreamGrabber.DeregisterBuffer(hBuffer);

        // Free all resources used for grabbing
        StreamGrabber.FinishGrab();

        // Close stream grabber
        StreamGrabber.Close();

        // Close camera
        Camera.Close();


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
