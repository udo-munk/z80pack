/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024-2025 by Udo Munk
 *
 * This module configures the machine appropriate for the
 * Z80/8080 software we want to run on it.
 *
 * History:
 * 23-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 implemented configuration dialog
 * 28-MAY-2024 implemented mount/unmount of disk images
 * 18-MAY-2025 separate read/save config file from config
 */

#ifndef SIMCFG_INC
#define SIMCFG_INC

extern void read_config(void);
extern void save_config(void);
extern void config(void);

#endif /* !SIMCFG_INC */
