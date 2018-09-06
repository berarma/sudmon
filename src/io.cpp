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
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <hidapi/hidapi.h>
#include "io.hpp"
#include "sud.hpp"
#include "ProjectConfig.h"

void printHelp() {
    printf("%s v%d.%d.%d, Copyright (C) 2018 Bernat Arlandis\n", PROJECT_NAME, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
	printf("\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY. This is free software,\n");
    printf("and you are welcome to redistribute it under certain conditions.\n");
    printf("\n");
    printf("Available commands (one mandatory):\n");
    printf("  -h This help\n");
    printf("  -l List connected compatible devices\n");
    printf("  -r Read values once\n");
    printf("  -c Continuous reading of sensor values\n");
    printf("  -s <status> Set Led status\n");
    printf("\n");
    printf("Available modifiers (optional):\n");
    printf("  -d Debug mode\n");
    printf("  -i <path> or <serial number> Select device by path or serial number (defaults to first one)\n");
    printf("  -f Full readings (with temp, pH and NH3)\n");
    printf("  -F Use Farenheit units (default is Celsius)\n");
    printf("  -w <seconds> Wait time between reads (only for full readings)\n");
    printf("  -H <rows> Display the header every X rows\n");
    printf("  -m Machine readable output\n");
    printf("  -t Convert timestamp to date/time\n");
    printf("  -D Use device timestamp\n");
    printf("\n");
}

bool parseOpts(Options *options, int argc, char * const argv[]) {
    int c;

    options->debug = false;
    options->fullReadings = false;
    options->machineReadable = false;
    options->useDevTs = false;
    options->humanizeTs = false;
    options->farenheit = false;
    options->cmdList = false;
    options->cmdReading = false;
    options->cmdContReading = false;
    options->cmdSetLeds = false;
    options->headerRows = 0;
    options->waitTime = 0;
    options->commands = 0;
    options->ident = NULL;

    while ((c = getopt(argc, argv, "cdDfFhH:i:lmrs:tw:")) != -1) {
        switch (c) {
            case 'c':
                options->cmdContReading = true;
                options->commands++;
                break;
            case 'd':
                options->debug = true;
                break;
            case 'D':
                options->useDevTs = true;
                break;
            case 'f':
                options->fullReadings = true;
                break;
            case 'F':
                options->farenheit = true;
                break;
            case 'h':
                printHelp();
                return 0;
            case 'H':
                options->headerRows = (int)strtol(optarg, NULL, 10);
                break;
            case 'i':
                options->ident = optarg;
                break;
            case 'l':
                options->cmdList = true;
                options->commands++;
                break;
            case 'm':
                options->machineReadable = true;
                break;
            case 'r':
                options->cmdReading = true;
                options->commands++;
                break;
            case 's':
                options->cmdSetLeds = true;
                options->leds = optarg;
                options->commands++;
                break;
            case 't':
                options->humanizeTs = true;
                break;
            case 'w':
                options->waitTime = (int)strtol(optarg, NULL, 10);
                break;
            case '?':
                if (optopt == 'c') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint (optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                return false;
            default:
                return false;
        }
    }

    return true;
}

void printDeviceList(const hid_device_info *devices) {
    printf("\n");
    while (devices) {
        printf("  Path    : %s\n", devices->path);
        printf("  Product : %ls\n", devices->product_string);
        printf("  Release : %x.%x\n", devices->release_number >> 8, devices->release_number & 0xff);
        printf("  Serial #: %ls\n", devices->serial_number);
        printf("\n");
        devices = devices->next;
    }
}

void printDeviceInfo(const hid_device_info *device, const SudData *data) {
    printf("Device: Seneye %s v.%d.%d.%d / Release: %x.%x / Serial #: %32ls\n",
            data->modelName,
            data->version[0],
            data->version[1],
            data->version[2],
            device->release_number >> 8,
            device->release_number & 0xff,
            device->serial_number
          );
}

void printHeader() {
    printf("==========================================================================================\n");
    printf("| Timestamp          | Wet | Temp.  | Slide  | pH   | NH3   | Kelvin | PAR  | Lux  | PUR |\n");
    printf("------------------------------------------------------------------------------------------\n");
}

void printReading(const SudData *data, const Options *options) {
    char
        timestamp[20],
        inWater[20],
        temp[20],
        slide[20],
        ph[20],
        nh3[20],
        kelvin[20],
        par[20],
        lux[20],
        pur[20];

    time_t ts;

    if (data->fullReading) {
        if (options->useDevTs) {
            ts = data->timestamp;
        }
        snprintf(inWater, 20, "%s",data->inWater ? "Yes" : "No");
        if (options->farenheit) {
            snprintf(temp, 20, "%.3f", ((double)data->temp / 1000) * 1.8 + 32);
        } else {
            snprintf(temp, 20, "%.3f", (double)data->temp / 1000);
        }
        if (data->slideNotFitted) {
            snprintf(slide, 20, "No");
        } else {
            if (data->slideExpired) {
                snprintf(slide, 20, "Expired");
            } else {
                snprintf(slide, 20, "Yes");
            }
        }
        if (!data->slideNotFitted) {
            snprintf(ph, 20, "%.2f", (double)data->ph / 100);
            snprintf(nh3, 20, "%.3f", (double)data->nh3 / 1000);
        }
    } else {
        snprintf(temp, 20, "-");
        snprintf(inWater, 20, "-");
        snprintf(slide, 20, "-");
    }

    if (!data->fullReading || data->slideNotFitted) {
        snprintf(ph, 20, "-");
        snprintf(nh3, 20, "-");
    }

    if (!data->fullReading || !options->useDevTs) {
        ts = time(NULL);
    }

    if (data->isKelvin) {
        snprintf(kelvin, 20, "%u", data->kelvin / 1000);
    } else {
        snprintf(kelvin, 20, "-");
    }
    snprintf(par, 20, "%d", data->par);
    snprintf(lux, 20, "%d", data->lux);
    snprintf(pur, 20, "%d%%", data->pur);

    if (options->humanizeTs) {
        struct tm *timeinfo;
        timeinfo = localtime(&ts);
        strftime(timestamp, 20, "%F %T", timeinfo);
    } else {
        snprintf(timestamp, 20, "%ld", ts);
    }

    if (options->machineReadable) {
        printf("%s %s %s %s %s %s %s %s %s %s\n",
                timestamp,
                inWater,
                temp,
                slide,
                ph,
                nh3,
                kelvin,
                par,
                lux,
                pur
              );
    } else {
        printf("|%19s | %-4s|%7s | %-7s|%5s |%6s |%7s |%5s |%5s |%4s |\n",
                timestamp,
                inWater,
                temp,
                slide,
                ph,
                nh3,
                kelvin,
                par,
                lux,
                pur
              );
    }
}

void hexDump(const unsigned char *data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        fprintf(stderr, "%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            fprintf(stderr, " ");
            if ((i+1) % 16 == 0) {
                fprintf(stderr, "|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    fprintf(stderr, " ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    fprintf(stderr, "   ");
                }
                fprintf(stderr, "|  %s \n", ascii);
            }
        }
    }
}

void debugSud(int direction, const unsigned char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    fprintf(stderr, "\n");
    if (direction) {
        fprintf(stderr, " * Reading at: ");
    } else {
        fprintf(stderr, " * Writing at: ");
    }
    fprintf(stderr, "%f\n", tv.tv_sec + tv.tv_usec / 1e6);
    hexDump(buffer, size);
    fprintf(stderr, "\n");
}
