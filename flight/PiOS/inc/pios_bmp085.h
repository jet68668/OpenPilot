/**
 ******************************************************************************
 *
 * @file       pios_bmp085.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      BMP085 functions header.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef PIOS_BMP085_H
#define PIOS_BMP085_H

/* BMP085 Addresses */
#define BMP085_ADDR_READ	0xEF
#define BMP085_ADDR_WRITE	0xEE
#define BMP085_CALIB_ADDR	0xAA
#define BMP085_CTRL_ADDR	0xF4
#define BMP085_OVERSAMPLING	0
#define BMP085_PRES_ADDR	(0x34 + (BMP085_OVERSAMPLING << 6))
#define BMP085_TEMP_ADDR	0x2E
#define BMP085_ADC_MSB		0xF6
#define BMP085_ADC_LSB		0xF7

/* Local Types */
typedef struct {
	uint16_t AC1;
	uint16_t AC2;
	uint16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	uint16_t B1;
	uint16_t B2;
	uint16_t MB;
	uint16_t MC;
	uint16_t MD;
} BMP085CalibDataTypeDef;

/* Public Functions */
void PIOS_BMP085_Init(void);
void PIOS_BMP085_StartADC(uint16_t *PresOrTemp);
int32_t PIOS_BMP085_Read(u16 address, uint8_t *buffer, uint8_t len);
int32_t PIOS_BMP085_Write(uint16_t address, uint8_t *buffer, uint8_t len);

#endif /* PIOS_BMP085_H */
