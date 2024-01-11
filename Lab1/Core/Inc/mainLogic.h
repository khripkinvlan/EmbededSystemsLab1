/*
 * mainLogic.h
 *
 *  Created on: 9 янв. 2024 г.
 *      Author: khrip
 */

#ifndef INC_MAINLOGIC_H_
#define INC_MAINLOGIC_H_
#include "stm32f4xx_hal.h"
#define ropeLength 50

typedef struct {
    float x;
    float y;
} point;

void AccConstrain();
float weightCoordinatesCalc();

#endif /* INC_MAINLOGIC_H_ */
