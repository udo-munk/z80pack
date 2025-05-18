/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2025 by Udo Munk
 *
 * This module contains göobal variables for the network configuration of Pico W.
 * Not compiled conditionally because the configuration is saved on MicroSD, and
 * I want files with identical format for Pico and Pico W.
 *
 * History:
 * 18-MAY-2025 first implementation
 */

#include "net_vars.h"

char wifi_ssid[WIFI_SSID_LEN+1];
char wifi_password[WIFI_PWD_LEN+1];
