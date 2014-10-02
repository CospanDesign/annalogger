
#include <stdio.h>
#include <stdint.h>
#include <delay.h>
#include <port.h>
#include <conf_board.h>
#include "ft2248.h"

typedef struct _ft2248_t ft2248_t;


struct _ft2248_t {
	uint8_t speed;
	uint32_t *data;
};


sensor_t * new_ft2248(uint8_t address){	

	sensor_t * sensor = NULL;
	ft2248_t * ft2248;
	sensor = new_sensor(LTC2448_SENSOR, address);
	
	//Set the channel count to the sensor type
	sensor->channel_count = LTC2448_SENSOR_CHANNEL_COUNT;
	
	//Setup the ft2248 Specific values
	ft2248 = (ft2248_t *)malloc(sizeof(ft2248_t));
	ft2248->speed = (uint8_t) ADC_SAMPLE_RATE_3P52KHZ; //Slow baudrate to begin with
	ft2248->data = (uint32_t *) calloc(LTC2448_SENSOR_CHANNEL_COUNT, sizeof(uint32_t));
	sensor->data = ft2248;

	sensor->update_all_sensor_data_fp = &ft2248_update_all_sensor_data;
	sensor->update_sensor_data_fp = &ft2248_update_sensor_data;
	sensor->get_sensor_data_fp = &ft2248_get_sensor_data;
	sensor->get_all_sensor_data_fp = &ft2248_get_all_sensor_data;
	sensor->set_sensor_data_fp = &ft2248_set_sensor_data;
	sensor->set_all_sensor_data_fp = &ft2248_set_all_sensor_data;
	
	return sensor;
}

void delete_ft2248(sensor_t * sensor){
	ft2248_t * ft2248 = (ft2248_t *) sensor->data;
	
	
	free(ft2248->data);
	free(ft2248);
	
	delete_sensor(sensor);
}

void ft2248_update_all_sensor_data(sensor_t *sensor){
	
	printf ("%s Entered\r\n", __func__);
	
}
#define WAIT_TIME 5
void ft2248_update_sensor_data(sensor_t *sensor, uint8_t channel){
	ft2248_t * ft2248 = (ft2248_t *) sensor->data;
	//printf ("%s Entered\r\n", __func__);
	
	uint32_t output_value = 0xB000FFFF;
	//uint32_t output_value = 0xA000FFFF;
	uint32_t input_value  = 0x00000000;
	output_value |= channel << 24;
	output_value |= ft2248->speed << 19;
	
	
	port_pin_set_output_level(CARD_SCK, LOW);
	port_pin_set_output_level(CARD_CS_N, LOW);
	
	for (int i = 0; i < 32; i ++){
		//Set MOSI to the 15th bit
		port_pin_set_output_level(CARD_MOSI, ((output_value >> 31) & 1));
//XXX: DELAY will be different with and without the RTOS
		delay_cycles_ms(WAIT_TIME);
		output_value = output_value << 1;
		port_pin_set_output_level(CARD_SCK, HIGH);

		
		//Shift the value from the digital input in
		input_value |= (0x01 & port_pin_get_input_level(CARD_MISO));
		input_value = input_value << 1;

//XXX: DELAY will be different with and without the RTOS
		delay_cycles_ms(WAIT_TIME);		
		
		port_pin_set_output_level(CARD_SCK, LOW);

	}
	
	port_pin_set_output_level(CARD_CS_N, HIGH);
	
//iXXX: f the highest data bit in the input is 1 then I think this signal is negative
	ft2248->data[channel] = (int32_t) input_value;
	
}
void ft2248_get_sensor_data(sensor_t * sensor, uint8_t channel, void * data){
	ft2248_t * ft2248 = (ft2248_t *) sensor->data;
	uint32_t * user_data = (uint32_t *) data;
	
	//printf ("%s Entered\r\n", __func__);
	//printf ("Data: 0x%08X\r\n", ft2248->data[channel]);
	*user_data = ft2248->data[channel];
}
void ft2248_get_all_sensor_data(sensor_t * sensor, void * data){
	printf ("%s Entered\r\n", __func__);
}
void ft2248_set_sensor_data(sensor_t * sensor, uint8_t channel, void * data){
	//ft2248_t * ft2248 = (ft2248_t *) sensor->data;
	printf ("%s Entered\r\n", __func__);
}
void ft2248_set_all_sensor_data(sensor_t *sensor, void *data){
	printf ("%s Entered\r\n", __func__);
}

void ft2248_set_speed(sensor_t * sensor, uint8_t speed){
	ft2248_t *ft2248 = (ft2248_t *) sensor->data;
	ft2248->speed = speed;
}