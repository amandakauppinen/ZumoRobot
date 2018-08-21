#include <project.h>
#include <stdio.h>
#include "Motor.h"
#include "Reflectance.h"
#include "IR.h"

void motor_forward_real(uint8 speed,uint32 delay);
void motor_backward_real(uint8 speed,uint32 delay);
int rread(void);

int main()
{

//SENSORS:
    struct sensors_ ref;
    struct sensors_ dig;
    CyGlobalIntEnable; 
    UART_1_Start();
    sensor_isr_StartEx(sensor_isr_handler);
    reflectance_start();
    IR_led_Write(1);
/*  SENSOR READING for setting threshold
    for(;;)
    {
        reflectance_read(&ref);
        printf("%d %d %d %d \r\n", ref.l3, ref.l1, ref.r1, ref.r3);       //print out each period of reflectance sensors
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        printf("%d %d %d %d \r\n", dig.l3, dig.l1, dig.r1, dig.r3);        //print out 0 or 1 according to results of reflectance period
        
        CyDelay(500);
    }
SENSORS END */
    
    reflectance_set_threshold(15000, 15000, 15000, 15000);
    //read 63,44,41,82 (values only used for ratio between sensors reference)
    //77,58,55,91 working for race
    
    unsigned int IR_val;
    
//loop 1 for going to first line:
    do {
        reflectance_read(&ref);
        reflectance_digital(&dig);

        motor_start();
        motor_forward_real(100,15);
    }while (dig.l3 == 1 && dig.r3 == 1);
    
    motor_stop();
    
//loop 2 for reading signal:
    do
    {
        IR_val = get_IR();
    }
    while (!IR_val);
    //printf("%x\r\n\n",IR_val);
    
    motor_start();
    motor_forward_real(150,250);
    
//loop 3 for general movements inside circle:
    
    do {
        reflectance_read(&ref);
        reflectance_digital(&dig);

    //all sensors same:
        if ((dig.l3 == 1) && (dig.l1 == 1) && (dig.r1 == 1) && (dig.r3 == 1)) {
            motor_forward_real(225,10);
        }
        
        else if ((dig.l3 == 0) && (dig.l1 == 0) && (dig.r1 == 0) && (dig.r3 == 0)) {
            motor_backward_real(225,900);
            motor_turn(244,0,440);
            motor_forward_real(200,10);
        }
        
    //sharpest turns:
        //right:
        else if ((dig.l3 == 0) && (dig.l1 == 0) && (dig.r1 == 0) && (dig.r3 == 1)) {
            motor_backward_real(225,900);
            MotorDirLeft_Write(0);      // set LeftMotor forward mode
            MotorDirRight_Write(0);     // set RightMotor forward mode
            motor_turn(244,0,440);
            motor_forward_real(200,10);
        }
        //left:
        else if ((dig.l3 == 1) && (dig.l1 == 0) && (dig.r1 == 0) && (dig.r3 == 0)) {
            motor_backward_real(225,900);
            MotorDirLeft_Write(0);      // ^
            MotorDirRight_Write(0);     // ^
            motor_turn(0,244,440);
            motor_forward_real(200,10);
        }
        
    //soft turns:
        //right:
        else if ((dig.l3 == 0) && (dig.l1 == 0) && (dig.r1 == 1) && (dig.r3 == 1)) {
            motor_backward_real(244,800);
            MotorDirLeft_Write(0);      // ^
            MotorDirRight_Write(0);     // ^
            motor_turn(150,0,500);
            motor_forward_real(200,10);
        }
        //left:
        else if ((dig.l3 == 1) && (dig.l1 == 1) && (dig.r1 == 0) && (dig.r3 == 0)) {
            motor_backward_real(244,800);
            MotorDirLeft_Write(0);      // ^
            MotorDirRight_Write(0);     // ^
            motor_turn(0,150,500);
            motor_forward_real(200,10);
        }
    //medium turns:
        //right:
        else if ((dig.l3 == 0) && (dig.l1 == 1) && (dig.r1 == 1) && (dig.r3 == 1)) {
            motor_backward_real(225,500);
            MotorDirLeft_Write(0);      // ^
            MotorDirRight_Write(0);     // ^
            motor_turn(150,0,440);
            motor_forward_real(200,10);
        }
        //left:
        else if ((dig.l3 == 1) && (dig.l1 == 1) && (dig.r1 == 1) && (dig.r3 == 0)) {
            motor_backward_real(225,500);
            MotorDirLeft_Write(0);      // ^
            MotorDirRight_Write(0);     // ^
            motor_turn(0,150,440);
            motor_forward_real(200,10);
        }
    } while (1);
return 0;
}
//function for actually going forward:
void motor_forward_real(uint8 speed,uint32 delay){
            
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(speed+(speed/100)*7.5);   //left motor was much slower
    PWM_WriteCompare2(speed);
    CyDelay(delay);
}
//function for actually going backward:
void motor_backward_real(uint8 speed,uint32 delay)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(speed+(speed/100)*7.5);   //left motor was much slower
    PWM_WriteCompare2(speed); 
    CyDelay(delay);
}

/* Don't remove the functions below */
int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') UART_1_PutChar('\r');
		UART_1_PutChar(*ptr++);
	}
	return len;
}

int _read (int file, char *ptr, int count)
{
    int chs = 0;
    char ch;
 
    (void)file; /* Parameter is not used, suppress unused argument warning */
    while(count > 0) {
        ch = UART_1_GetChar();
        if(ch != 0) {
            UART_1_PutChar(ch);
            chs++;
            if(ch == '\r') {
                ch = '\n';
                UART_1_PutChar(ch);
            }
            *ptr++ = ch;
            count--;
            if(ch == '\n') break;
        }
    }
    return chs;
}
/* [] END OF FILE */