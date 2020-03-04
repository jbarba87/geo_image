/*
void PrintError( Error error );
void imprime_error(Error error, char *mensaje_error);
void PrintCameraInfo( CameraInfo* pCamInfo );

int InitCameras( void *cA, void *cB);
int GetPicture( void *cA, void *cB, int i);
int CloseCameras( void *cA, void *cB);
*/


typedef struct FlyStruct{
	unsigned int numCameras;
  fc2Context *context;
  fc2PGRGuid *guid;
	fc2Image *rawImage;
} FlyStruct;


void imprime_error(fc2Error error, char *mensaje_error);
int iniFly(FlyStruct *var, int shuttle);
int getImage(FlyStruct *var, int numFoto);
int CloseCameras(FlyStruct *var);

