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

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "sud.hpp"
#include "io.hpp"

#define TIMEOUT 5

SudData *readData(SudController *sud, unsigned char mode, unsigned char type) {
    SudData *data;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    do {
        data = sud->readData();
        gettimeofday(&t1, NULL);
    } while ((t1.tv_sec - t0.tv_sec) < TIMEOUT && (data == NULL || data->mode != mode || data->type != type));

    return data;
}

int main(int argc, char *argv[])
{
    Options options;
    hid_device_info *device;
    SudController *sud = NULL;
    SudData *data;

    if (!parseOpts(&options, argc, argv)) {
        return 1;
    }

    if (options.commands != 1) {
        fprintf(stderr, "Missing command option.\n");

        return -1;
    }

    if (SudController::init()) {
        fprintf(stderr, "Error initialising the control library.\n");

        return -1;
    }

    if (options.cmdList) {
        hid_device_info *devices = SudController::findDevices();
        if (devices == NULL) {
            printf("No devices found.\n");
        }
        printDeviceList(devices);
    }

    if (!options.cmdSetLeds && !options.cmdReading && !options.cmdContReading) {
        return 0;
    }

    device = SudController::getDeviceInfo(options.ident);
    if (device != NULL) {
        if (options.ident == NULL) {
            options.ident = device->path;
        }
        sud = SudController::open(options.ident);
    }

    if (!sud) {
        fprintf(stderr, "Unable to open device.\n");

        return -1;
    }

    if (options.debug) {
        sud->setDebugCallback(debugSud);
    }

    if (!sud->hello()) {
        fprintf(stderr, "Error greeting device.\n");

        return -1;
    }

    data = readData(sud, 0x88, 0x01);

    if (data == NULL) {
        fprintf(stderr, "Error establishing communication with device.\n");

         return -1;
    }

    if (data->mode != 0x88 || data->type != 0x01) {
        fprintf(stderr, "Error establishing connection with device, wrong message.\n");

        return -1;
    }

    if (!data->success) {
        fprintf(stderr, "This device need to be connected to SCA or SWS\n");

        return -1;
    }

    if (!options.machineReadable) {
        printDeviceInfo(device, data);
    }

    if (options.cmdSetLeds) {
        int res = sud->setLeds(options.leds);
        if (res == 0) {

            return 0;
        } else {
            fprintf(stderr, "Error setting leds status. %d\n", res);

            return -1;
        }
    }

    int rows = 0;

    while (1) { 
        if (options.fullReadings) {
            sud->request();
            data = readData(sud, 0, 1);
        } else {
            data = readData(sud, 0, 2);
        }
        if (data == NULL) {
            fprintf(stderr, "Error reading sensor values.\n");

            if (!options.cmdContReading) {
                return -1;
            }

            continue;
        }

        if (!options.machineReadable && (rows == 0 || (options.headerRows != 0 && rows % options.headerRows == 0))) {
            printHeader();
        }
        rows++;

        printReading(data, &options);

        if (!options.cmdContReading) {
            break;
        }

        if (options.fullReadings) {
            sleep(options.waitTime);
        }
    }

    sud->bye();

    data = readData(sud, 0x77, 1);
    if (!data || !data->success) {
        fprintf(stderr, "Error closing the communication with the device.\n");
    }

    sud->close();

    SudController::exit();

    return 0;
}
