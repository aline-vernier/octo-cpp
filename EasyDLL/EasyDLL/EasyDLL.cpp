// EasyDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "math.h" //library that defines the pow function
#include <QCamApi.h>
#include <stdio.h>

#define M_PI 3.14159 //declare our M_PI constant



//Function declarations
extern "C" __declspec(dllexport) int GetSphereSAandVol(double radius, double* sa, double* vol);
extern "C" __declspec(dllexport) int OpenCamLib(int i);
extern "C" __declspec(dllexport) unsigned long ListCam(unsigned long*  listLength, unsigned long * _cameraId);
extern "C" __declspec(dllexport) int OpenCam(unsigned long cameraId, unsigned long * handle);
extern "C" __declspec(dllexport) int CloseCam(unsigned long  handle);
extern "C" __declspec(dllexport) int OpenCloseCam(int i, unsigned long cameraId, unsigned long * handle);
extern "C" __declspec(dllexport) int CloseCamLib(int i);

double GetSA(double radius);
double GetVol(double radius);

__declspec(dllexport) int GetSphereSAandVol(double radius, double* sa, double* vol)
//Calculate the surface area and volume of a sphere with given radius
{
if(radius < 0)
return false; //return false (0) if radius is negative
*sa = GetSA(radius);
*vol = GetVol(radius);
return true;
}

__declspec(dllexport) int OpenCamLib(int i)

{
		QCam_Err            errcode = qerrSuccess ;

		errcode = QCam_LoadDriver();
		if(errcode==qerrSuccess)  return 0;
		else return -1;

}
__declspec(dllexport) unsigned long ListCam(unsigned long* listLength, unsigned long * _cameraId)

{
		QCam_Err            errcode = qerrSuccess ;
		QCam_CamListItem	listItem;
	
		errcode = QCam_ListCameras(&listItem, listLength );
		
		if(errcode == qerrSuccess){
			*_cameraId =  listItem.cameraId;
			return listItem.cameraId; 
		}
		else return -1;

}


__declspec(dllexport) int OpenCam(unsigned long cameraId, unsigned long * handle)

{
		QCam_Err			errcode = qerrSuccess ;
		QCam_Handle			_handle;


		errcode = QCam_OpenCamera(cameraId, &_handle );
		*handle = *(unsigned long*)((void*)(_handle));
		
		if(errcode == qerrSuccess){ 
		//	errcode = QCam_CloseCamera(&_handle);
			return sizeof(QCam_Handle); 
		}
		else return (int)(errcode);


}

__declspec(dllexport) int CloseCam(unsigned long  handle)

{
		QCam_Err			errcode = qerrSuccess ;
		QCam_Handle			_handle;


		_handle = (QCam_Handle)((void *)(handle));
		errcode = QCam_CloseCamera(&_handle);
		
		if(errcode == qerrSuccess)return 0; 
		else return (int)(qerrSuccess);


}

__declspec(dllexport) int OpenCloseCam(int i, unsigned long cameraId, unsigned long * handle)

{


			QCam_Err			errcode = qerrSuccess ;
			QCam_Handle			_handle = NULL;
			
			errcode = QCam_OpenCamera(cameraId, &_handle);
			*handle = *(unsigned long*)((void*)(_handle));

			if(errcode == qerrSuccess){
				QCam_CloseCamera(&_handle);

				return (int)(errcode);}


			 return 0;
		


}

__declspec(dllexport) int CloseCamLib(int i)

{



		QCam_ReleaseDriver();
		return 0;
}


        

        

double GetSA(double radius)
{
return 4 * M_PI * radius * radius;
}
double GetVol(double radius)
{
return 4.0/3.0 * M_PI * pow(radius, 3.0);
}