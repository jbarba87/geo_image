/* 06 de julio, programa funciona ok, falta probar toma de imagenes por largos periodos de tiempo*/

#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<time.h>

//#include <FlyCapture2.h>
#include "C/FlyCapture2_C.h"
#include<getopt.h>

#include "getData.h"

#include "geoCam.h"
#include "txVideo.h"

// int *numPhotos, int *waitTime, int *delay, int *shuttle, int *gps

typedef struct params{

	int numFotos;
	int	delay;
	int	waitTime;
	int shuttle;
	int video;
	int gps;
	int imu;

} params;


int saveData(FILE *file, ImData *ImageData, int numFoto, int sat, int numCameras, time_t hora){

	int i;

	fprintf(file, " %i ", numFoto);

	// Saving picture name
	for (i = 0; i < numCameras; i++){
		fprintf(file, " %c%05i.bmp  ", 'A'+i, numFoto);
	}

	// Saving time since epoch
	fprintf(file, " %u  ", hora);

	// Saving attitude and position
	fprintf(file, "\t %f  %f \t ", ImageData->roll, ImageData->pitch);
	fprintf(file, "%i %s %s \n ", sat, ImageData->latitude, ImageData->longitude);//

	return 0;
}


enum {
	FOTOS = 1,
	TIME,
	DELAY,
	VIDEO,
	SHUTTLE,
	IMU,
	GPS,
	VERBOSE,
	HELP
};


struct option opts_list[] = {

	{"fotos",1,0,FOTOS},
	{"tiempo",1,0,TIME},
	{"retraso",1,0,DELAY},
	{"video",0,0,VIDEO},
	{"shuttle",0,0,SHUTTLE},
	{"GPS",0,0,GPS},
	{"IMU",0,0,IMU},
	{"verbose",0,0,VERBOSE},
	{"help",0,0,HELP},
	{0,0,0,0}
};



//int get_input(int argc, char **argv, int *numPhotos, int *waitTime, int *delay, int *shuttle, int *gps){
int get_input(int argc, char **argv, params *parametros){

	int c;

	// Setting default parameters
	parametros->numFotos = 10;
	parametros->delay = 1;
	parametros->waitTime = 1;


	while ((c = getopt_long(argc, argv, "", opts_list, NULL)) != EOF){
		switch (c) {
			case FOTOS:
				parametros->numFotos = atoi(optarg);
				if (parametros->numFotos < 0){
					printf(" Numero de fotos debe ser mayor que 0, considerando 10 \n");
					parametros->numFotos = 10;
				}
				break;

			case TIME:
				parametros->waitTime = atoi(optarg);
				if (parametros->waitTime < 0){
					printf(" Tiempo de espera debe ser mayor que 0, considerando 1 \n");
					parametros->waitTime = 1;
				}
				break;

			case DELAY:

				parametros->delay = atoi(optarg);
				if (parametros->delay < 0){
					printf(" Tiempo de retraso debe ser mayor que 0, considerando 1 \n");
					parametros->delay = 1;
				}
				break;

			case VIDEO:
				parametros->video = 1;
				break;

			case GPS:

				parametros->gps = 1;
				break;

			case IMU:

				parametros->imu = 1;
				break;

			case SHUTTLE:

				parametros->shuttle = 1;
				break;

			case VERBOSE:
				//  Not yet implemented
				break;
			case HELP:
				printf(" Modo de uso: ./capfotos [opt]\n");
				printf(" opt: \n");
				printf("\t --fotos \t Numero de fotos a tomar \n");
				printf("\t --tiempo \t Intervalo de tiempo entre fotos \n");
				printf("\t --retraso \t Tiempo de espera para iniciar captura \n");
				printf("\t --shuttle \t Shuttle Manual\n");
				printf("\t --GPS   \t Imagenes con datos de ubicacion\n");
				printf("\t --IMU   \t Imagenes con datos de posicion\n");
				printf("\t --video \t Modo transmision de video \n");
				printf("\n");
				exit (0);
				break;

			default:
				printf(" Comando invalido \n");
				exit (0);
		}
}
	return 0;
}


void	imprime_parametros(params *parametros){

	printf("\n Parametros:	\n");
	printf("\t Numero de fotos: %i \n", parametros->numFotos);
	printf("\t Tiempo de espera: %i \n", parametros->delay);
	printf("\t Tiempo entre fotos: %i \n", parametros->waitTime);

	if (parametros->shuttle) printf("\t Shuttle Manual \n");
	else printf("\t Shuttle Auto\n");

	if (parametros->gps) printf("\t Imagenes con datos de ubicacion (GPS) \n");
	else printf("\t Imagenes sin datos de ubicacion (GPS) \n");

	if (parametros->imu) printf("\t Imagenes con datos de posicion (IMU) \n");
	else printf("\t Imagenes sin datos de posicion (IMU) \n");

	printf("\n");
}


int cuentaCams(void){

  fc2Error error;
	unsigned int numCameras;

  fc2Context ct;
	error = fc2CreateContext( &ct ); imprime_error(error, "Error en funcion fc2CreateContext\n");
  error = fc2GetNumOfCameras( ct, &numCameras ); imprime_error(error, "Error en funcion fc2GetNumOfCameras\n");
	printf("\n Camaras detectadas: %i \n", numCameras);
	if (numCameras == 0) {
		exit (0);
	}

	return numCameras;
}


// Programa Principal
int main(int argc, char **argv){

	time_t hora;

	params *parametros = (params *)calloc(1, sizeof(params));

	// Getting parameters
	get_input(argc, argv, parametros);

	// Transmition video mode?
	if (parametros->video == 1){
		printf("\n Modo transmision de video \n");
		txVideo();
		return 0;
	}


	FILE *file;

	char data_imu[14];
	char data_gps[67];

	// Devices
	char device_i2c[] = "/dev/i2c-2"; // Just for pcduino
	char device_serial[] = "/dev/ttyS1"; // Serial comunication

	// Openning file
	file =  fopen("Resultados/file.txt", "w");

// Initializing devices
	int i2cHandle, serialHandle;
	if (parametros->imu == 1)		i2cHandle	= inicializa_imu(ADDRESS_IMU, device_i2c);
	if (parametros->gps == 1)		serialHandle = inicializa_gps(device_serial);

	int i = 0, sat = 0;

	ImData *ImageData = (ImData *)calloc(1, sizeof(ImData));

	int numCameras = cuentaCams();

	imprime_parametros(parametros);

	FlyStruct var; 
	iniFly(&var, parametros->shuttle);

	sleep( parametros->delay );

	// Main loop
	for (i = 0; i<parametros->numFotos; i++){

		// The IMU must be read every 10ms
		// The data start at direction 0x3B, getting 14 position of memory
		if (parametros->imu == 1){
			lee_imu(i2cHandle, 0x3B, 14, data_imu);
			get_angles(data_imu, ImageData);
		}


		if (parametros->gps == 1){
			sat = lee_data_gps(serialHandle, data_gps, ImageData);
		}

		getImage(&var, i);
		
		hora = time(NULL);	// get seconds since epoch

		saveData(file, ImageData, i, sat, numCameras, hora);
		sleep(parametros->waitTime);
	}

	printf("\n Finalizando\n");
	printf("\n");

	fclose(file);
	CloseCameras(&var);
	
	return 0;
}

