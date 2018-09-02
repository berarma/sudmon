/*
SudMon - Seneye USB Device Monitor
Copyright (C) 2018  Bernat Arlandis (berarma@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <ctime>
#include <hidapi/hidapi.h>
#include "sud.hpp"

#ifndef SUD_IO_HPP
#define SUD_IO_HPP

typedef struct {
    bool debug;
    bool fullReadings;
    bool machineReadable;
    bool useDevTs;
    bool humanizeTs;
    bool farenheit;
    bool cmdList;
    bool cmdReading;
    bool cmdContReading;
    bool cmdSetLeds;
    int headerRows;
    int waitTime;
    int commands;
    char *ident;
    char *leds;
} Options;

void printHelp();
bool parseOpts(Options *options, int argc, char * const argv[]);
void printDeviceList(const hid_device_info *devices);
void printDeviceInfo(const hid_device_info *device, const SudData *data);
void printHeader();
void printReading(const SudData *data, const Options *options);
void hexDump(const unsigned char *data, size_t size);
void debugSud(int direction, const unsigned char *buffer, size_t size);

#endif
