
#include<stdio.h>
#include "C/FlyCapture2_C.h"
#include <stdlib.h>

#include"geoCam.h"


void imprime_error(fc2Error error, char *mensaje_error){

	if (error != FC2_ERROR_OK ){
		printf("%s", mensaje_error);
		//PrintError( error );
		//exit(0);
	} 
}

/*
void PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}
*/

int iniFly(FlyStruct *var, int shuttle){

  fc2Error error;
	unsigned int numCameras;
	int i, value;

  fc2Context ct;
	error = fc2CreateContext( &ct ); imprime_error(error, "Error en funcion fc2CreateContext\n");
  error = fc2GetNumOfCameras( ct, &numCameras ); imprime_error(error, "Error en funcion fc2GetNumOfCameras\n");

	// CReating structures
	fc2Context *context = (fc2Context *) malloc(numCameras * sizeof(fc2Context));
	fc2PGRGuid *guid = (fc2PGRGuid *) malloc(numCameras * sizeof(fc2PGRGuid));
	fc2Image *rawImage = (fc2Image *) malloc(numCameras * sizeof(fc2Image));


	// Initializing structures
	for (i = 0; i<numCameras; i++){
		error = fc2CreateContext( &context[i] ); imprime_error(error, "Error en funcion fc2CreateContext\n");
	}

	for (i = 0; i<numCameras; i++){
	  error = fc2GetCameraFromIndex( context[i], i, &guid[i] );  imprime_error(error, "Error en funcion fc2GetCameraFromIndex\n");
	}

	for (i = 0; i<numCameras; i++){
	  error = fc2Connect( context[i], &guid[i] ); imprime_error(error, "Error en funcion fc2Connect\n");
	}

	for (i = 0; i<numCameras; i++){
	  error = fc2StartCapture( context[i] );  imprime_error(error, "Error en funcion fc2StartCapture\n");
	}

	for (i = 0; i<numCameras; i++){
	  error = fc2CreateImage( &rawImage[i] ); imprime_error(error, "Error en funcion fc2CreateImage\n");
	}

	float abs_shuttle; // 0.25 - 130 ms
	int rel_shuttle; // 0-531


	// Shuttle
	if (shuttle){ // Shuttle Manual

		for (i = 0; i<numCameras; i++){

			// Valor de shuttle?
			printf("Ingrese shuttle camara %i (ms): ", i);
			scanf("%f", &abs_shuttle);
			rel_shuttle = (int) (abs_shuttle/0.25);
//			printf("shuttle seleccionado %i\n", rel_shuttle);


			// setting shuttle on the camera
			fc2ReadRegister( context[i], 0x81C, &value);
			value &= 0xBE000000;
			fc2WriteRegister( context[i], 0x81C, value);

			// Setting off auto gain and setting it to the medium value
			fc2ReadRegister( context[i], 0x820, &value);
			value &= 0xFEFFFF00;
			value |= 0x00000022; 
			fc2WriteRegister( context[i], 0x820, value);

			// Set relative shuttle to the camera
			fc2ReadRegister( context[i], 0x81C, &value);
			value |= rel_shuttle;
			fc2WriteRegister( context[i], 0x81C, value);
			printf("Camera %i initial shuttle  %i \n", i, rel_shuttle);

		}


	} else {  // Shuttle Auto

		for (i = 0; i<numCameras; i++){
			// Setting auto mode in cameras
			fc2ReadRegister( context[i], 0x81C, &value);
			value |= 0xC3000000;
			fc2WriteRegister( context[i], 0x81C, value);

			// Setting on auto gain
			fc2ReadRegister( context[i], 0x820, &value);
			value |= 0x83000000;
			fc2WriteRegister( context[i], 0x820, value);

		}

	}
	var->numCameras = numCameras;
	var->context = context;
	var->guid = guid;
	var->rawImage = rawImage;

	return 0;
}


int getImage(FlyStruct *var, int numFoto){

	int i;
	unsigned int numCameras = var->numCameras;

	fc2Error error;
	fc2Context *context = var->context;
	fc2PGRGuid *guid = var->guid;
	fc2Image *rawImage = var->rawImage;

	int value = 0;

	char nombre[30];

	for (i = 0; i<numCameras; i++){
		error = fc2RetrieveBuffer( context[i], &rawImage[i] ); imprime_error(error, "Error en funcion fc2RetrieveBuffer\n");
	}

	for (i = 0; i<numCameras; i++){

		sprintf(nombre, "Resultados/%c%05i.bmp", 'A'+i, numFoto);
		printf(" Guardando imagen %s \n", nombre);
		error = fc2SaveImage( &rawImage[i], nombre, FC2_BMP ); imprime_error(error, "Error en funcion fc2SaveImage\n");
	}

	return 0;
}

int CloseCameras(FlyStruct *var){

	int i;
	unsigned int numCameras = var->numCameras;
	
	fc2Error error;
	fc2Context *context = var->context;
  fc2Image *rawImage = var->rawImage;

	for (i = 0; i<numCameras; i++){
   	error = fc2DestroyContext( context[i] ); imprime_error(error, "Error en funcion fc2DestroyContext\n");
	}

	for (i = 0; i<numCameras; i++){
   	error = fc2DestroyImage( &rawImage[i] ); imprime_error(error, "Error en funcion fc2DestroyImage\n");
	}
	return 0;
}

