/*
 * ESP8266_HAL.h
 *
 *  Created on: Apr 14, 2020
 *      Author: Controllerstech
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_


void ESP_Init (char *SSID, char *PASSWD);

void Server_Start (void);
int Server_Send (char *str, int Link_ID);


#endif /* INC_ESP8266_H_ */
