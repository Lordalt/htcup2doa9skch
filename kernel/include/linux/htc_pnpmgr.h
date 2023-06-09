/* arch/arm/mach-msm/htc_pnpmgr.h
 *
 * pnpmgr driver header
 *
 * Copyright (C) 2014 HTC Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ARCH_ARM_MACH_PNPMGR_H
#define __ARCH_ARM_MACH_PNPMGR_H

enum {
	USER_PERF_LVL_LOWEST,	
	USER_PERF_LVL_LOW,		
	USER_PERF_LVL_MEDIUM,	
	USER_PERF_LVL_HIGH,		
	USER_PERF_LVL_HIGHEST,	
	USER_PERF_LVL_TOTAL,
};

struct user_perf_data {
	int *bc_perf_table;
	int *lc_perf_table;
};

extern void pnpmgr_init_perf_table(struct user_perf_data *pdata);
extern int notify_screen(int val);
#endif

