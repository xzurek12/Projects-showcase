#include<ADuC7020.h>
#include<stdint.h>
#include <stdio.h>

void My_IRQ_Function(void);
extern void SendStrUart(char *str);

#define ACC_ADDR 0x3A						// Acclerometr address(1D) + WriteBit(0)
#define WHO_AM_I 0x0D						// Device ID register (should be 0x2A = 42 = '*')
#define CTRL_REG1 0x2A						// Device ACTIVE/STANDBY register, 01 = ACTIVE, 00 = STDNBY 
#define XYZ_DATA_CFG 0x0E				   	// Device Scale register (2g, 4g, 8g) 0x02 = 8g scale
#define PL_CFG 0x11							// Device Portrait/Landscape register

// Device registres for XYZ axis data
#define OUT_X_MSB 0x01
#define OUT_Y_MSB 0x03
#define OUT_Z_MSB 0x05

// Constants
#define ACTIVE_MODE 0x03  
#define STANDBY_MODE 0x00
#define SCALE_G8 0x02
#define PL_ENABLE 0xC0

#define BYTE_LENGTH 8
#define DELAY_TIME 1000
#define G8 8
#define GRAVITY_CONST 10.0f

// Global variables
int8_t XYZ_DATA[3];
int AXIS_INDEX = 0;


//-----------------------------------------------------------------------------------------------
void print_byte(int8_t byte){

	int i;
	int8_t mask = 0x01;
	char byteStr[BYTE_LENGTH + 1] = "\0";
	
	for (i = BYTE_LENGTH - 1; i >= 0; i--){
		
		if (byte & mask){
			byteStr[i] = '1';
		}
		else{
			byteStr[i] = '0';
		}

		mask = mask << 1;
	}
	
	SendStrUart(byteStr);	
}
//-----------------------------------------------------------------------------------------------
/**
*  Function for generating some delay
**/
void delay(int length){
	int i;
	for (i = 0; i < length; i++){}
}
//-----------------------------------------------------------------------------------------------
/**
*  Function writes to Serial port(UART)
**/
int uart_write(int ch){                   

	if (ch == '\n'){
    	while(!(0x020 == (COMSTA0 & 0x020))){}
		COMTX = 0x0D;
	}
    while (!(0x020 == (COMSTA0 & 0x020))){}

 	return (COMTX = ch);
}
//-----------------------------------------------------------------------------------------------
/**
*  Function reads from Serial port(UART)
**/
int uart_read(){                     

   	while (!(0x01 == (COMSTA0 & 0x01))){}
  	return (COMRX);
}
//-----------------------------------------------------------------------------------------------
/**
*  Function for UART and I2C configuration
**/
void init_i2c_uart(){

	GP1CON = 0x2211;						// Set UART tx & rx pins on P1.0 and P1.1  = 0x0011
											// and I2C on P1.2(SCL) and P1.3(SDA) 	   = 0x2200  
	
	// Start setting up I2C at 100kHz
	I2C1CFG = 0x82;		  					// Master Enable & Enable Generation of Master Clock
	I2C1DIV = 0xCFCF;		    			// 0xCFCF = 100kHz, 0x3232 = 400kHz

	// Start setting up UART at 19200 bauds
	COMCON0 = 0x080;						// Setting DLAB
   	COMDIV0 = 0x44;							// Setting DIV0 and DIV1 to DL calculated 19200	
   	COMDIV1 = 0x000;
   	COMCON0 = 0x007;						// Clearing DLAB
	COMIEN0 = 0x01;							// Enable UART RX interrupt

	// Interrupt config
	IRQ = My_IRQ_Function; 					// Interrupt rutine
	IRQEN = SM_MASTER1_BIT + UART_BIT;		// Interrupt enabled, SM_MASTER1_BIT = I2C1, UART_BIT = UART

	GP4DAT = 0x04000000;                    // P4.2 configured as an output. LED is turned on
}
//-----------------------------------------------------------------------------------------------
/**
*	devAddr = Device address + WriteBit(0), regAdrr = Register address
*	Function reads 1 byte from Device, Data read in Interrupt routine
**/
void i2c_byte_read(unsigned char devAddr, unsigned char regAddr){

	I2C0CCNT = 0x0; 						// Sets Start/Stop condition counter value to 0 - minimum value. 
	I2C1ADR = devAddr; 						// Accelerometer(slave) adress(Master Write)
	I2C1MTX = regAddr;						// Accelerometr register address
	while ((I2C1FSTA & 0x30) != 0x00) {;} 	// Wait for the Tx FIFO to empty
	I2C1CNT = 0x0; 							// Number of bytes to be read from slave - 1, 0 = 1 byte will be read
	I2C1ADR = devAddr + 1; 					// Accelerometer(slave) adress(Master Read) = devAddr + 1
	I2C0CCNT = 0x80;						// Set the Start/Stop counter to a nonzero value to re-enable the Stop Condition

	delay(DELAY_TIME);
}
//-----------------------------------------------------------------------------------------------
/**
*	devAddr = Device address + WriteBit(0), regAdrr = Register address
*	Function writes 1 byte to Device
**/
void i2c_byte_write(unsigned char devAddr, unsigned char regAddr, unsigned char data){

	I2C1MTX = regAddr;						// Accelerometr register address
	I2C1MTX = data;							// Data to Slave
	I2C1ADR = devAddr; 						// Accelerometer(slave) adress(Master Write)

	delay(DELAY_TIME);
}
//-----------------------------------------------------------------------------------------------
/**
*  Function for Accelerometr configuration
*  Accelerometr is set to ACTIVE mode
**/
void init_acc(){

	// Acc to STANDBY mode, in order to change registres
	i2c_byte_write(ACC_ADDR, CTRL_REG1, STANDBY_MODE);
	// Scale to 8g  
	i2c_byte_write(ACC_ADDR, XYZ_DATA_CFG, SCALE_G8);
	// Enable Portrait/Landscape detection 
	i2c_byte_write(ACC_ADDR, PL_CFG, PL_ENABLE);
	// Acc to ACTIVE mode, Enable FAST READ = F_READ is set, only 8-bit data
	i2c_byte_write(ACC_ADDR, CTRL_REG1, ACTIVE_MODE);	
}
//-----------------------------------------------------------------------------------------------
/**
*  Function reads MSB bytes from Accelerometr OUT_X(Y,Z)_MSB registres
*  FAST READ is enabled, only 1 byte of data needed for measurement
**/
void read_xyz_data(){

	// Read X axis
	i2c_byte_read(ACC_ADDR, OUT_X_MSB);
	AXIS_INDEX++;
	// Read Y axis
	i2c_byte_read(ACC_ADDR, OUT_Y_MSB);
	AXIS_INDEX++;
	// Read Z axis
	i2c_byte_read(ACC_ADDR, OUT_Z_MSB);
}
//-----------------------------------------------------------------------------------------------
/**
*  Function computes acceleration for each axis(x,y,z) and send result to UART
*  Result is in m/s units(not in g!)
*  Function is called in ISR(Interrupt Service Routine)
**/
void get_xyz_data(){

	char str[255];
	int x, y, z;				   
	float cx, cy, cz;

	x = XYZ_DATA[0];
	y = XYZ_DATA[1];
	z = XYZ_DATA[2];
	cx = ((float) x / (float)(1<<7) * (float)(G8)) * GRAVITY_CONST;
	cy = ((float) y / (float)(1<<7) * (float)(G8)) * GRAVITY_CONST;
	cz = ((float) z / (float)(1<<7) * (float)(G8)) * GRAVITY_CONST;

	sprintf(str, "[X:%f;Y:%f;Z:%f]", cx, cy, cz);
	SendStrUart(str);
}
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
int main (){

	POWKEY1 = 0x01;
	POWCON = 0x00;		   					// 41.78MHz CHYBA!!! asi je správnì 45.088MHz !
	POWKEY2 = 0xF4;

	init_i2c_uart();
   	init_acc();
    
    while (1)
    {	
		read_xyz_data();
	} 
}

/********************************************************************/
/*                                                                  */
/*              Interrupt Service Rountine                          */
/*                                                                  */
/********************************************************************/

void My_IRQ_Function(){

	int c;
	char str[255];

	// Interrupt from I2C Bus
	if ((IRQSTA & SM_MASTER1_BIT) != 0){

		// Master Recieve IRQ
		if((I2C1MSTA & 0x8) == 0x8){ 			
			// Read X_MSB
			if (AXIS_INDEX == 0) XYZ_DATA[AXIS_INDEX] = I2C1MRX;
			// Read Y_MSB
			if (AXIS_INDEX == 1) XYZ_DATA[AXIS_INDEX] = I2C1MRX;
			// Read Z_MSB + reset AXIS_INDEX + Compute accleration
			if (AXIS_INDEX == 2){
			   	XYZ_DATA[AXIS_INDEX] = I2C1MRX;
				get_xyz_data();
				AXIS_INDEX = 0;
			}
    	}
    }

	// Interrupt from UART Bus
	if ((IRQSTA & UART_BIT) != 0){

		c = uart_read();
		sprintf(str, "(O:%c)", c);
		SendStrUart(str);	
    }

	// TODO 
	if ((IRQSTA & XIRQ0_BIT) != 0){;
    }
	if ((IRQSTA & XIRQ1_BIT) != 0){;
    }	
}

