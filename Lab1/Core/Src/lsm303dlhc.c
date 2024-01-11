#include "lsm303dlhc.h"

extern I2C_HandleTypeDef hi2c1;

volatile int16_t x_acc = 0x00;
volatile int16_t y_acc = 0x00;
volatile int16_t z_acc = 0x00;


extern UART_HandleTypeDef huart1;
uint8_t buf2[14]={0};
char str1[30]={0};
//—————————————————————————————————————————————————————————————————

static void Error(void) {
	LED_R1_ON;
}

void Accel_Ini(void) {
	uint16_t ctrl = 0x0000;
	HAL_Delay(1000);
	if (Accel_ReadID() == 0x33) {
		LED_G_ON;
		LED_R1_OFF;
	} else
		Error();
	ctrl |= (LSM303DLHC_NORMAL_MODE | LSM303DLHC_ODR_50_HZ
			| LSM303DLHC_AXES_ENABLE);
	ctrl |= ((LSM303DLHC_BlockUpdate_Continous | LSM303DLHC_BLE_LSB
			| LSM303DLHC_HR_ENABLE) << 8);
	AccInit(ctrl);

	ctrl = (uint8_t) (LSM303DLHC_HPM_NORMAL_MODE | LSM303DLHC_HPFCF_16
			| LSM303DLHC_HPF_AOI1_DISABLE | LSM303DLHC_HPF_AOI2_DISABLE);
	Accel_AccFilterConfig(ctrl);
	LED_B_ON;
}

void AccInit(uint16_t InitStruct) {
	uint8_t ctrl = 0x00;
	ctrl = (uint8_t) InitStruct;
	Accel_IO_Write(0x32, LSM303DLHC_CTRL_REG1_A, ctrl);
	ctrl = (uint8_t) (InitStruct << 8);
	Accel_IO_Write(0x32, LSM303DLHC_CTRL_REG4_A, ctrl);
}

static uint8_t I2C1_ReadData(uint16_t Addr, uint8_t Reg) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t value = 0;
	status = HAL_I2C_Mem_Read(&hi2c1, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value,
			1, 0x10000);
	if (status != HAL_OK) {
		/* Execute user timeout callback */
		Error();
	}
	return value;
}

uint8_t Accel_IO_Read(uint16_t DeviceAddr, uint8_t RegisterAddr) {
	return I2C1_ReadData(DeviceAddr, RegisterAddr);
}

uint8_t Accel_ReadID(void) {
	uint8_t ctrl = 0x00;
	ctrl = Accel_IO_Read(0x32, 0x0F);
	return ctrl;
}

static void I2C1_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value) {
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t) Reg,
	I2C_MEMADD_SIZE_8BIT, &Value, 1, 0x10000);
	/* Check the communication status */
	if (status != HAL_OK) {
		/* Execute user timeout callback */
		Error();
	}
}

void Accel_IO_Write(uint16_t DeviceAddr, uint8_t RegisterAddr, uint8_t Value) {
	I2C1_WriteData(DeviceAddr, RegisterAddr, Value);
}

void Accel_AccFilterConfig(uint8_t FilterStruct) {
	uint8_t tmpreg;
	/* Read CTRL_REG2 register */
	tmpreg = Accel_IO_Read(0x32, LSM303DLHC_CTRL_REG2_A);
	tmpreg &= 0x0C;
	tmpreg |= FilterStruct;
	/* Write value to ACC MEMS CTRL_REG2 register */
	Accel_IO_Write(0x32, LSM303DLHC_CTRL_REG2_A, tmpreg);
}

//——————————————————————————————————————————————————————————————————————————————
void Accel_GetXYZ(int16_t *pData) {
	int16_t pnRawData[3];
	uint8_t ctrlx[2] = { 0, 0 };
	int8_t buffer[6];
	uint8_t i = 0;
	uint8_t sensitivity = LSM303DLHC_ACC_SENSITIVITY_2G;

	/* Read the acceleration control register content */
	ctrlx[0] = Accel_IO_Read(0x32, LSM303DLHC_CTRL_REG4_A);
	ctrlx[1] = Accel_IO_Read(0x32, LSM303DLHC_CTRL_REG5_A);

	/* Read output register X, Y & Z acceleration */
	buffer[0] = Accel_IO_Read(0x32, LSM303DLHC_OUT_X_L_A);
	buffer[1] = Accel_IO_Read(0x32, LSM303DLHC_OUT_X_H_A);
	buffer[2] = Accel_IO_Read(0x32, LSM303DLHC_OUT_Y_L_A);
	buffer[3] = Accel_IO_Read(0x32, LSM303DLHC_OUT_Y_H_A);
	buffer[4] = Accel_IO_Read(0x32, LSM303DLHC_OUT_Z_L_A);
	buffer[5] = Accel_IO_Read(0x32, LSM303DLHC_OUT_Z_H_A);

	/* Check in the control register4 the data alignment*/
	if (!(ctrlx[0] & LSM303DLHC_BLE_MSB)) {
		for (i = 0; i < 3; i++) {
			pnRawData[i] = ((int16_t) ((uint16_t) buffer[2 * i + 1] << 8)
					+ buffer[2 * i]);
		}
	} else /* Big Endian Mode */
	{
		for (i = 0; i < 3; i++) {
			pnRawData[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
					+ buffer[2 * i + 1]);
		}
	}
	/* Normal mode */

	/* Switch the sensitivity value set in the CRTL4 */
	switch (ctrlx[0] & LSM303DLHC_FULLSCALE_16G) {
	case LSM303DLHC_FULLSCALE_2G:
		sensitivity = LSM303DLHC_ACC_SENSITIVITY_2G;
		break;

	case LSM303DLHC_FULLSCALE_4G:
		sensitivity = LSM303DLHC_ACC_SENSITIVITY_4G;
		break;

	case LSM303DLHC_FULLSCALE_8G:
		sensitivity = LSM303DLHC_ACC_SENSITIVITY_8G;
		break;

	case LSM303DLHC_FULLSCALE_16G:
		sensitivity = LSM303DLHC_ACC_SENSITIVITY_16G;
		break;
	}

	/* Obtain the mg value for the three axis */
	for (i = 0; i < 3; i++) {
		pData[i] = (pnRawData[i] * sensitivity);
	}
}
//——————————————————————————————————————————————————————————————————————————————

void Accel_ReadAcc(void) {
	int16_t buffer[3] = { 0 };
	int16_t xval, yval, zval = 0x00;
	uint16_t tmp16 = 0;

	Accel_GetXYZ(buffer);
	xval = buffer[0];
	x_acc = xval;
	yval = buffer[1];
	y_acc = yval;
	zval = buffer[2];
	z_acc = zval;
}
