#include <stdio.h>
#include <stdlib.h>
#include <QCamApi.h>
#include <Assert.h>

//===== #DEFINES =========================================================

#define ASSERT( x )    assert( x )

//===== Function prototypes ===============================================
static QCam_Err F_SetupCamera( QCam_Handle handle, unsigned long ccdType, unsigned long exposure, QCam_ImageFormat* format );
static QCam_Err F_SnapAndSaveMono( QCam_Handle handle, char* filename );

//===== Main ==============================================================
int main(int argc, char* argv[])
{


	QCam_Err            errcode = qerrSuccess ;
	FILE*				log;
	int                  exposure = 16.4*1000; 
	QCam_ImageFormat    format = qfmtMono16;
	char* filename = "nameOfFile.dat"; 

	log = fopen("logfile.txt", "w");
	fprintf(log, "starting this prog\n");
	errcode = QCam_LoadDriver();
	fprintf (log,"finished loading\n");
		
	if(errcode == qerrSuccess){

		QCam_CamListItem    listItem;
        unsigned long       listLength = 1;

        errcode = QCam_ListCameras( &listItem, &listLength );
        if ( errcode == qerrSuccess )
        { 

			if ( listLength == 0 ){fprintf( log, "No cameras found.\n" );}

           else
            {
			if ( listLength > 1 ){fprintf(log,  "Choosing first of %u cameras.\n", listLength );}
               
			QCam_Handle        handle;
			errcode = QCam_OpenCamera( listItem.cameraId, &handle );

			if ( errcode == qerrSuccess )
                {
                    //
                    // 4.  Switch the camera to 16 bit mode, set the exposure.
                    //

                    unsigned long        ccdType;

                    errcode = QCam_GetInfo( handle, qinfCcdType, &ccdType );
                    ASSERT( errcode == qerrSuccess );
					fprintf(log, "assert successful\n");

                    errcode = F_SetupCamera( handle, ccdType, exposure, &format );
					

                    if ( errcode == qerrSuccess )
                    {
						fprintf(log, "ok, successfully assigned format to 16 bits\n");
                        //
                        // 5.  Capture and save the image.
                        //
						errcode = F_SnapAndSaveMono( handle, filename );
						if ( errcode == qerrSuccess )fprintf(log, "snapand save ok\n");
						else fprintf(log, "Snap and Save failed. too bad\n");
                    }
					else{fprintf(log, "failed to assign format to 16 bits\n");}

				}
			QCam_CloseCamera( handle );
			}

			
		}
			QCam_ReleaseDriver();
			fprintf(log, "success!\n");

		}
	else{fprintf(log, "failed:(\n");}
	fclose(log);

return 0;
}

	
QCam_Err F_SetupCamera
(
    QCam_Handle          handle,
    unsigned long        ccdType,
    unsigned long        exposure,            // Exposure in us.
    QCam_ImageFormat*    pFormat
)
{
    QCam_Err             errcode;
    QCam_SettingsEx      settings;
    QCam_ImageFormat     format;
    unsigned long        expMax;
    unsigned long        expMin;
    unsigned long        uTable[32];
    int                  uSize = 32;
    int                  i;

    // Get default settings from the camera.

    errcode = QCam_CreateCameraSettingsStruct( &settings );
    if ( errcode != qerrSuccess )
        return errcode;

    errcode = QCam_InitializeCameraSettings( handle, &settings );
    if ( errcode != qerrSuccess )
        return errcode;

    errcode = QCam_ReadDefaultSettings( handle, (QCam_Settings*)&settings );
    if ( errcode != qerrSuccess )
        return errcode;

    // Get min/max exposure times.

    errcode = QCam_GetParamMin( (QCam_Settings*)&settings, qprmExposure, &expMin );
    ASSERT( errcode == qerrSuccess );

    errcode = QCam_GetParamMax( (QCam_Settings*)&settings, qprmExposure, &expMax );
    ASSERT( errcode == qerrSuccess );

    // Set the format... either 16 bit mono or 16 bit bayer.

    errcode = QCam_GetParamSparseTable ((QCam_Settings*)&settings, qprmImageFormat, uTable, &uSize);

    ASSERT (errcode == qerrSuccess);

    format = ( ccdType == qcCcdColorBayer ) ? qfmtBayer16 : qfmtMono16;

    for (i=0;i<uSize;i++)
    {
        if (format == (QCam_ImageFormat)uTable[i])
            break;
    }

    // If we support the 16-bit format use it.  Else, drop down to 8-bit
    if (format != (QCam_ImageFormat)uTable[i])
        format = (qfmtBayer16 == format ? qfmtBayer8 : qfmtMono8);


    errcode = QCam_SetParam( (QCam_Settings*)&settings, qprmImageFormat, format );
        
    ASSERT( errcode == qerrSuccess );

    *pFormat = format;

    // Set the exposure time.  Validate against max and min exposure times.

    if ( exposure < expMin )
    {
        printf( "Exposure is too small, adjusting to %u us.\n", expMin );
        exposure = expMin;
    }
    else if ( exposure > expMax )
    {
        printf( "Exposure is too large, adjusting to %u us.\n", expMax );
        exposure = expMax;
    }

    errcode = QCam_SetParam( (QCam_Settings*)&settings, qprmExposure, exposure );
    ASSERT( errcode == qerrSuccess );

    // Here's where the camera is changed.

    errcode = QCam_SendSettingsToCam( handle, (QCam_Settings*)&settings );

    return errcode;
}

QCam_Err F_SnapAndSaveMono
(
    QCam_Handle         handle,
    char*               filename
)
{
    QCam_Err            errcode;
    unsigned long       imageSize;            // Size of image in bytes
    QCam_Frame          frame;
	FILE*				img;
	int i, j;

	img = fopen("new grabbed image.dat", "w");
    // Get size of image.
    errcode = QCam_GetInfo( handle, qinfImageSize, &imageSize );
    ASSERT( errcode == qerrSuccess );

    // Create our frame buffer.
    frame.pBuffer = new unsigned short[ imageSize ];
    frame.bufferSize = imageSize;

    // Grab the frame.
    errcode = QCam_GrabFrame( handle, &frame );
    if ( errcode == qerrSuccess )
    {
		for(i=0;i<frame.height;i++){
			for(j=0;j<frame.width;j++){
				fprintf(img, "%d,", (static_cast<unsigned short*>(frame.pBuffer))[i*frame.width+j]);
			}
			fprintf(img, "\n");
		}
		 
    }
	fclose(img);
    // Delete the frame buffer.    
    delete frame.pBuffer;

    return errcode;
}
