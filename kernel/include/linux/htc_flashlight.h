/*
 * include/linux/htc_flashlight.h - The flashlight header
 *
 * Copyright (C) 2016 HTC Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __HTC_FLASHLIGHT_H
#define __HTC_FLASHLIGHT_H

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#define FLASHLIGHT_NAME "flashlight"

#define FLASHLIGHT_OFF   0
#define FLASHLIGHT_TORCH 1
#define FLASHLIGHT_FLASH 2
#define FLASHLIGHT_NUM   3


enum flashlight_mode_flags {
	FL_MODE_OFF = 0,
	FL_MODE_TORCH = 1,
	FL_MODE_FLASH = 2,
	FL_MODE_PRE_FLASH = 3,
	FL_MODE_TORCH_LED_A,
	FL_MODE_TORCH_LED_B,
	FL_MODE_TORCH_LEVEL_0,
	FL_MODE_TORCH_LEVEL_1,
	FL_MODE_TORCH_LEVEL_2,
	FL_MODE_CAMERA_EFFECT_FLASH,
	FL_MODE_CAMERA_EFFECT_PRE_FLASH,
	FL_MODE_FLASH_LEVEL1,
	FL_MODE_FLASH_LEVEL2,
	FL_MODE_FLASH_LEVEL3,
	FL_MODE_FLASH_LEVEL4,
	FL_MODE_FLASH_LEVEL5,
	FL_MODE_FLASH_LEVEL6,
	FL_MODE_FLASH_LEVEL7,
	FL_MODE_VIDEO_TORCH = 30,
	FL_MODE_VIDEO_TORCH_1,
	FL_MODE_VIDEO_TORCH_2,
	FL_MODE_VIDEO_TORCH_3,
	FL_MODE_VIDEO_TORCH_4,
};


enum flashlight_brightness_attribute_definition
{ 
    FBAD_OFF        = 0,
    FBAD_TORCH1     = 125, 
    FBAD_TORCH2     = 126, 
    FBAD_TORCH      = 127, 
    FBAD_PREFLASH   = 128, 
    FBAD_FLASH1     = 130, 
    FBAD_FLASH2     = 131, 
    FBAD_FLASH3     = 132, 
    FBAD_FLASH4     = 133, 
    FBAD_FLASH5     = 134, 
    FBAD_FLASH6     = 135, 
    FBAD_FLASH7     = 136, 
    FBAD_FULL       = 255, 
};

#if defined(CONFIG_FLASHLIGHT_TPS61310) || defined(CONFIG_FLASHLIGHT_TPS61310_FRONT)
struct TPS61310_flashlight_platform_data {
	void (*gpio_init) (void);
	uint32_t flash_duration_ms;
	uint32_t led_count; 
	uint32_t tps61310_strb0;
	uint32_t tps61310_strb1;
	uint32_t tps61310_reset;
	uint8_t mode_pin_suspend_state_low;
	uint32_t enable_FLT_1500mA;
	uint32_t disable_tx_mask;
	uint32_t power_save; 
	uint32_t power_save_2;
};
#endif
#if defined(CONFIG_FLASHLIGHT_TPS61310)
int tps61310_flashlight_control(int mode);
int tps61310_flashlight_mode(int mode);
int tps61310_flashlight_mode2(int mode2, int mode13);
#endif

#if defined(CONFIG_FLASHLIGHT_LM3643)
struct LM3643_flashlight_platform_data {
	void (*gpio_init) (void);
	uint32_t flash_duration_ms;
	uint32_t led_count; 
	uint32_t lm3643_hwen;
	uint32_t lm3643_hwen_ioexp;
	uint32_t lm3643_strobe;
	uint32_t lm3643_torch;
	uint32_t lm3643_reset;
	uint8_t mode_pin_suspend_state_low;
	uint32_t enable_FLT_1500mA;
	uint32_t disable_tx_mask;
	uint32_t power_save; 
	uint32_t power_save_2;
};
int lm3643_flashlight_flash(int led1,int led2);
int lm3643_flashlight_torch(int led1, int led2);
#endif

extern int (*htc_flash_main)(int ,int);
extern int (*htc_torch_main)(int ,int);
extern int (*htc_flash_front)(int ,int);
extern int (*htc_torch_front)(int ,int);

#undef __HTC_FLASHLIGHT_H
#endif

