#include <stdio.h>

#include <stdlib.h>

#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include "C/FlyCapture2_C.h"

#include "geoCam.h"
#include "txVideo.h"


int txVideo(void){

	fc2Error error;
	unsigned int numCameras;

  fc2Context ct;
	error = fc2CreateContext( &ct ); imprime_error(error, "Error en funcion fc2CreateContext\n");
  error = fc2GetNumOfCameras( ct, &numCameras ); imprime_error(error, "Error en funcion fc2GetNumOfCameras\n");
	printf(" Numero de camaras %i \n", numCameras);


	int socket_desc , new_socket , c;
	struct sockaddr_in server , client;

	//Create socket
	socket_desc = socket(AF_INET ,SOCK_STREAM ,0);
	if (socket_desc == -1){
		 printf(" Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
     
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
		puts(" Fallo al hacer bind");
		return 1;
	}
	printf(" Bind hecho\n");

	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
	puts(" Esperando conexiones...");
	c = sizeof(struct sockaddr_in);

	FlyStruct var; 
	iniFly(&var, 0);

	int n,i = 0;
	char key, mensaje;

	fc2Context *context = var.context;
	fc2PGRGuid *guid = var.guid;
	fc2Image *rawImage = var.rawImage;

	int value, aux;


	// Setting up initial parameters (Shuttle and Gain)
	for (i = 0; i<numCameras; i++){

		// Set non auto and non absolute mode (value mode)
		fc2ReadRegister( context[i], 0x81C, &value);
		value &= 0xBE000000;
		fc2WriteRegister( context[i], 0x81C, value);

		// Setting off auto gain and setting it to the medium value
		fc2ReadRegister( context[i], 0x820, &value);
		value &= 0xFEFFFF00;
		value |= 0x00000022; 
		fc2WriteRegister( context[i], 0x820, value);

		// Set an initial value to Shuttle
		fc2ReadRegister( context[i], 0x81C, &value);
		value |= 0x0000000F;
		fc2WriteRegister( context[i], 0x81C, value);
		//printf("Camera %i initial shuttle  %x \n", i, value);

	}


	int total_imagenes = 0;
	mensaje = 'n';


	// Waiting for connections
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*) &c)) ){
		puts("Conexion aceptada");

		i = 0;
		total_imagenes = 0;
		printf("\n");

		while ( key != 'q' ){

			error = fc2RetrieveBuffer( context[i], &rawImage[i] ); imprime_error(error, "Error en funcion fc2RetrieveBuffer\n");

			n = send(new_socket , (void *) rawImage[i].pData, rawImage[i].dataSize, 0);

			read(new_socket, &mensaje, 1);

			// changing camera
			if (mensaje == 'x'){
				i++;
				printf("\n");
				if ( i == numCameras) i = 0;
			}

			// Increasing shuttle
			if (mensaje == '+'){
				fc2ReadRegister( context[i], 0x81C, &value);
				aux = value & 0x3FF; // shuttle son los 10 ultimos bits (de 0 a 531, no se por que)
				if(aux < 521) value += 10;
				fc2WriteRegister( context[i], 0x81C, value);
			}

			if (mensaje == 'm'){
				fc2ReadRegister( context[i], 0x81C, &value);
				aux = value & 0x3FF; // shuttle son los 10 ultimos bits (de 0 a 531, no se por que)
				if(aux < 431) value += 100;
				fc2WriteRegister( context[i], 0x81C, value);
			}

			// Decreasing shuttle
			if (mensaje == '-'){
				fc2ReadRegister( context[i], 0x81C, &value);
				aux = value & 0x3FF; // shuttle son los 10 ultimos bits (de 0 a 531, no se por que)
				if(aux > 10) value -= 10;
				fc2WriteRegister( context[i], 0x81C, value);
			}
			if (mensaje == 'n'){
				fc2ReadRegister( context[i], 0x81C, &value);
				aux = value & 0x3FF; // shuttle son los 10 ultimos bits (de 0 a 531, no se por que)
				if(aux > 100) value -= 100;
				fc2WriteRegister( context[i], 0x81C, value);
			}


			fc2ReadRegister( context[i], 0x81C, &value);
			aux = value & 0x3FF; // shuttle son los 10 ultimos bits (de 0 a 531, no se por que)

			total_imagenes++;
			printf("\r Total imagenes enviadas: %i, Camara %i, valor Shuttle %f (absoluto) ", total_imagenes, i, ((float)aux) * 0.25 );
			fflush(stdout);

			usleep(300000);

			if (mensaje == 'q'){
				printf("\n Finalizando transferencia \n");
				close(new_socket);
				CloseCameras(&var);
				exit(0);
				break;
			}
		}
	}

	if (new_socket<0){
		perror("accept failed");
		return 1;
	}

/*
	close(new_socket);
	CloseCameras(&var);
*/
	return 0;
}



