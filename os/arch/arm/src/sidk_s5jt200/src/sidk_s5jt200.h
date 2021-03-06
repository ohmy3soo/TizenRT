/****************************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * arch/arm/src/sidk_s5jt200/src/sidk_s5jt200.h
 *
 *   Copyright (C) 2009, 2011, 2013, 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *           Laurent Latil <laurent@latil.nom.fr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
#ifndef __ARCH_ARM_SRC_SIDK_S5JT200_SRC_SIDK_S5JT200_H__
#define __ARCH_ARM_SRC_SIDK_S5JT200_SRC_SIDK_S5JT200_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <tinyara/config.h>

/* procfs file system */
#ifdef CONFIG_FS_PROCFS
#define SIDK_S5JT200_PROCFS_MOUNTPOINT	"/proc"
#endif

enum configdata_id {
	SIDK_S5JT200_CONFIGDATA_PLATFORM	= 0xff00,
	SIDK_S5JT200_CONFIGDATA_WIFI_NVRAM,
};

#ifdef CONFIG_SIDK_S5JT200_TLC59116
#define TLC59116_ADDR		0x60
#define TLC59116_MAX_LEDS	16

#define TLC59116_REG_MODE1	0x00
#define TLC59116_REG_MODE2	0x01
#define TLC59116_REG_LED(x)	(0x14 + (x))
#define TLC59116_REG_PWM(x)	(0x02 + (x))

#define TLC59116_LED_0		0
#define TLC59116_LED_1		1
#define TLC59116_LED_2		2
#define TLC59116_LED_3		3

#define TLC59116_LED_R		0
#define TLC59116_LED_G		1
#define TLC59116_LED_B		2

int tlc59116_initialize(void);
#endif /* CONFIG_SIDK_S5JT200_TLC59116 */

#endif /* __ARCH_ARM_SRC_SIDK_S5JT200_SRC_SIDK_S5JT200_H__ */
