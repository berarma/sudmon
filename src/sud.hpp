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

#ifndef SUD_HPP
#define SUD_HPP

typedef struct {
    unsigned char mode;
    unsigned char type;
    union {
        struct {
            unsigned char success;
            unsigned char deviceType;
            const char *modelName;
            unsigned char version[3];
        };
        struct {
            int	kelvin;
            int x;
            int	y;
            unsigned par;
            unsigned lux;
            unsigned pur;

            time_t timestamp;

            int temp;

            unsigned short ph;
            unsigned short nh3;

            unsigned short stateT;
            unsigned short statePh;
            unsigned short stateNh3;

            bool isKelvin;
            bool inWater;
            bool slideNotFitted;
            bool slideExpired;
            bool error;

            bool fullReading;
        };
    };
} SudData;

class SudController
{
    static hid_device_info *enumeration;
    hid_device *handle;
    unsigned char buffer[65];
    void (*callback)(int direction, const unsigned char *buffer, size_t size);

    public:
        static int init();
        static int exit();
        static hid_device_info *findDevices();
        static hid_device_info *getDeviceInfo(char *ident);
        static SudController *open(char *path);

        SudController(hid_device *handle);
        int setNonblocking(int nonblock);
        void setDebugCallback(void (*callbck)(int direction, const unsigned char *buffer, size_t size));
        int hello();
        int bye();
        void close();
        SudData *readData();
        const unsigned char *getRawData();
        int request();
        int setLeds(char *ledValues);

    private:
        const char *getModelName(unsigned char deviceType);
        int write(const unsigned char *buffer, size_t size);
        void readAllValues(SudData *data, const unsigned char *buffer);
        void readLmValues(SudData *data, const unsigned char *buffer);
};

#endif
