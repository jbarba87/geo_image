#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <sys/select.h>

#include "getData.h"


int inicializa_gps(char *device_serial){

	fd_set set;
	struct timeval timeout;
	int rv;
	
	// Activate alternate mode (UART) for pins 0 and 1
	system("echo 3 > /sys/devices/virtual/misc/gpio/mode/gpio0");	// Rx
	system("echo 3 > /sys/devices/virtual/misc/gpio/mode/gpio1");	// Tx

	// Openning device
	int serialHandle = open(device_serial, O_RDONLY);
  if (serialHandle == -1) {
    printf("Error al abrir archivo \n");
    return 0;
  }

/*
	FD_ZERO(&set);
	FD_SET(serialHandle, &set);

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	rv = select(serialHandle + 1, &set, NULL, NULL, &timeout);
printf("rv %i \n", rv);
	if (rv == -1){
		printf("Dispositivo serial no detectado \n");
		return -1;
	}	else {
		return serialHandle;
	}
*/


	return serialHandle;

}



int lee_data_gps(int serialHandle, char *data_gps, ImData *image_data){
// Return number of satelites or 0 if it is not fixed
	int gps_fix = 0;
	char num_sat[3];
	int i;
  char a;
	char id[5];

	for (;;){
	 
		read(serialHandle, &a, 1);

		if (a == '$') {
		  read(serialHandle, id, 5);

		  if ( id[3] == 'G' && id[4] == 'A' ){ // looking for identificatos $GPGGA

		  read(serialHandle, data_gps, 67);

				if (data_gps[37] == '1'){

					// Setting up visual indicator (LED)
					// Not implemented


					// GPS fix
					// Setting position parameters to the variables
					num_sat[0] = data_gps[39]; num_sat[1] = data_gps[40];
					gps_fix = atoi(num_sat);

					// Setting parameters for the struct
					for (i = 0; i<=8; i++){
						image_data->latitude[i] = data_gps[12 + i];
						image_data->longitude[i] = data_gps[25 + i];
					}

					image_data->latitude[9] = ' ';
					image_data->longitude[9] = ' ';

					image_data->latitude[10] = data_gps[22];
					image_data->longitude[10] = data_gps[35];

					image_data->latitude[11] = '\0';
					image_data->longitude[11] = '\0';
				} else {
						gps_fix = 0;		// GPS non fix
						for (i = 0; i<10; i++){
							image_data->latitude[i] = '\0';
							image_data->longitude[i] = '\0';
						}
					}

				break; // get out for
		  }
		}
	}
	return gps_fix;
}




int escribe_imu(int handle, int length, char *data){

	int opResult = write(handle, (void *) data, length);
	if (opResult != length) {
		printf("Error al escribir \n");
		return -1;
	}
}

int lee_imu(int handle, char address, int length, char *data){

	// Putting the address in the i2c bus
	escribe_imu(handle, 1, &address);

	int opResult = read(handle, (void *) data, length);
	if (opResult != length) {
		printf("Error al leer \n");
		return -1;
	}
}


void get_angles(char *data, ImData *image_data){

	int ACC_X = 0,  ACC_Y = 0, ACC_Z = 0;
//	int TEMP = 0; don't neet monitore temperature 
	int GYR_X = 0,  GYR_Y = 0, GYR_Z = 0;

	float angle_X = image_data->roll, angle_Y = image_data->pitch;	// Getting the last medition

	// Auxiliary accelerometer and gyroscope variables
	float X, Y, Z;
	float GX, GY;

	float delta_time = 0.01; // 0.01 seconds
	float dummy;

//	ACC_X = 0;  ACC_Y = 0; ACC_Z = 0; TEMP = 0;
	ACC_X = (data[0]<<8) + data[1];
	ACC_Y = (data[2]<<8) + data[3];
	ACC_Z = (data[4]<<8) + data[5];
//	TEMP = (data[6]<<8) + data[7];

	GYR_X = (data[8]<<8) + data[9];
	GYR_Y = (data[10]<<8) + data[11];
	GYR_Z = (data[12]<<8) + data[13];

	// values bigger than 32768 must be negative (two's complement)
	if (ACC_X >=32768) ACC_X = -(65536 - ACC_X);
	if (ACC_Y >=32768) ACC_Y = -(65536 - ACC_Y);
	if (ACC_Z >=32768) ACC_Z = -(65536 - ACC_Z);

	if (GYR_X >=32768) GYR_X = -(65536 - GYR_X);
	if (GYR_Y >=32768) GYR_Y = -(65536 - GYR_Y);
	if (GYR_Z >=32768) GYR_Z = -(65536 - GYR_Z);

	//	Converting the value to radians (sexagecimals)
	X = ((float)ACC_X/SEN_ACC), Y = ((float) ACC_Y/SEN_ACC),  Z =  ((float) ACC_Z/SEN_ACC);

	GX = ((float) (GYR_X - GYR_OFFSET_X))/SEN_GYR;
	GY = ((float) (GYR_Y - GYR_OFFSET_Y))/SEN_GYR;
//	GZ = ((float) (GYR_Z - GYR_OFFSET_X))/SEN_GYR; // not used

	// Getting angles without using gyro's (most basic algorithms)
	angle_X = atan2f(Y, sqrtf(X*X + Z*Z))*180.0/PI;
	angle_Y = atan2f(X, sqrtf(Y*Y + Z*Z))*180.0/PI;

/*
	// Complementary filter
	dummy = atan2f(Y, sqrtf(X*X + Z*Z))*180.0/PI;
	angle_X = 0.98*( angle_X + GX*delta_time ) + 0.02*dummy;
	dummy = atan2f(X, sqrtf(Y*Y + Z*Z))*180.0/PI;
	angle_Y = 0.98*( angle_Y + GY*delta_time ) + 0.02*dummy;
*/

	image_data->roll = angle_X;
	image_data->pitch = angle_Y;

//	usleep(10000);
}



// initializing IMU
int inicializa_imu(int address, char *device){

	int resultado;

	int i2cHandle = open("/dev/i2c-2", O_RDWR);
	if(i2cHandle == -1) {
		printf("Error al abrir device \n");
		return -1;
	}
	int opResult = ioctl(i2cHandle, I2C_SLAVE, address);
	if(opResult == -1) {
		printf("Error de ioctl \n");
		return -1;
	}

	usleep(2000);

	// Starting the IMU, set to 0x00 address 0x6B
	char sec_inicializa[2];

	sec_inicializa[0] = 0x6B;
	sec_inicializa[1] = 0x00;

	resultado = escribe_imu(i2cHandle, 2, sec_inicializa);

	if (resultado == -1) {
		printf("IMU not detected \n");
		return resultado; // IMU not detected
	}	else {

	// Writting in the address 0x1C can change the sensibility (0x18)
	// Using the default sensibility
		return i2cHandle;
	}
}


