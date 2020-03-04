

#define PI 3.1416

// Accelerometer and gyroscope factors (according to datasheet)
#define SEN_ACC 16384.0
#define SEN_GYR 131.0

// Gyroscope offset, experimental calculation
#define GYR_OFFSET_X -88
#define GYR_OFFSET_Y 184
#define GYR_OFFSET_Z -176

// Slave IMU address, connected to i2c-2 (for pcduino)
#define ADDRESS_IMU 0x68  


typedef struct ImData {

	float roll;
	float pitch;
	float yaw; // not implemented (no magnetometer)

	char	latitude[12];
	char	longitude[12];
	float altitude;

	char time[19]; // not implemented (no RTC)

}	ImData;

int inicializa_gps(char *device_serial);
int lee_data_gps(int serialHandle, char *data_gps,  ImData *image_data);

int escribe_imu(int handle, int length, char *data);
int lee_imu(int handle, char address, int length, char *data);
void get_angles(char *data, ImData *image_data);

int inicializa_imu(int address, char *device);





