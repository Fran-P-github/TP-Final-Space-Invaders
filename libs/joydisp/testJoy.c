/*
testJoy imprime las coordenadas del joystick. Termina cuando se presiona el switch.
*/

#include <stdio.h>
#include <unistd.h>
#include "joydrv.h"

int main(void)
{
	printf("Test Start:\n");
	joyinfo_t info;
	joy_init();				//inicializo el joystick
	do
	{
		info = joy_read();				//las guardo en info
		printf("(%4d, %4d)\n" , info.x, info.y);	
		usleep(200000);
	} while (info.sw == J_NOPRESS);	//mientras no se presione el switch
	
	return 0;
}