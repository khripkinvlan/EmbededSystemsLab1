#include "mainLogic.h"
#include <math.h>

extern volatile int16_t x_acc;
extern volatile int16_t y_acc;
extern volatile int16_t z_acc;

void AccConstrain()
{
	if (x_acc<=200 && x_acc>-200) {
		x_acc = 1;
	}
	if (y_acc<=200 && y_acc>-200) {
		y_acc = 1;
	}
	if (z_acc<=200 && z_acc>-200) {
		z_acc = 1;
	}
}

float weightCoordinatesCalc()
{
	float angle = 5.0;
	angle = atan((float)y_acc/(float)z_acc);
	return angle;
}
