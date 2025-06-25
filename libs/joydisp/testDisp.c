/*
testDisp enciende todo el display de a un LED por vez
Luego apaga todos los leds de la misma forma
*/

#include <stdio.h>
#include <unistd.h>
#include "disdrv.h"

int decision(void);

int main(void)
{
    printf("Starting Test...\n");
	dcoord_t myPoint;
	disp_init();				//inicializa el display
	disp_clear();				//limpia todo el display

	for (myPoint.y = DISP_MIN; myPoint.y <= DISP_MAX_Y; myPoint.y++)	//para cada coordenada en y...
	{
		for ( myPoint.x = DISP_MIN; myPoint.x <= DISP_MAX_X ; myPoint.x++)	//para cada coordenada en x...
		{
			disp_write(myPoint, D_ON);				//prende el LED en el buffer. Otra opción es: disp_write({i, j}, D_ON);
			printf("(%2d,%2d) " , myPoint.x, myPoint.y);//imprime cuál LED fue encendido
			disp_update();							//ahora lo vemos en el display
            usleep(50000);                 //Delay porque sino es muy rapido.
		}
		printf("\n");
	}

    sleep(2);   //wait 2 seconds
    printf("Clearing Display!\n");

    disp_clear();
    disp_update();
	return 0;
}

