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

#include <hidapi/hidapi.h>
#include <string.h>
#include <stdlib.h>
#include "sud.hpp"

#define VID 0x24f7
#define PID 0x2204

#define WORD16(buffer) ((buffer)[0] + ((buffer)[1] << 8))
#define WORD32(buffer) (WORD16(buffer) + ((buffer)[2] << 16) + ((buffer)[3] << 24))

hid_device_info *SudController::enumeration = NULL;

int SudController::exit()
{
    if (enumeration != NULL) {
        hid_free_enumeration(enumeration);
    }

    return hid_exit();
}

int SudController::init()
{
    return hid_init();
}

hid_device_info *SudController::findDevices()
{
    if (enumeration != NULL) {
        hid_free_enumeration(enumeration);
    }

    enumeration = hid_enumerate(VID, PID);

    return enumeration;
}

hid_device_info *SudController::getDeviceInfo(char *ident)
{
    hid_device_info *list = findDevices();

    if (ident == NULL) {
        return list;
    }

    wchar_t wident[200];
    mbstowcs(wident, ident, 200);

    while (list != NULL) {
        if (strcmp(ident, list->path) == 0 || wcscmp(wident, list->serial_number) == 0) {
            return list;
        }
        list = list->next;
    }

    return NULL;
}

SudController *SudController::open(char *ident)
{
    hid_device *handle = hid_open_path(ident);
    if (handle == NULL) {
        wchar_t wident[200];
        mbstowcs(wident, ident, 200);
        handle = hid_open(VID, PID, wident);
    }

    if (handle == NULL) {
        return NULL;
    }

    return new SudController(handle);
}

SudController::SudController(hid_device *handle) : handle(handle), callback(NULL)
{
}

int SudController::setNonblocking(int nonblock)
{
    return hid_set_nonblocking(handle, 1);
}

void SudController::setDebugCallback(void (*callback)(int direction, const unsigned char *buffer, size_t size))
{
    this->callback = callback;
}

int SudController::hello()
{
    memset(buffer, 0x00, 65);

    buffer[1] = 'H';
    buffer[2] = 'E';
    buffer[3] = 'L';
    buffer[4] = 'L';
    buffer[5] = 'O';
    buffer[6] = 'S';
    buffer[7] = 'U';
    buffer[8] = 'D';
    return write((const unsigned char*)&buffer, 65) == 65;
}

int SudController::bye()
{
    memset(buffer, 0x00, 65);

    buffer[1] = 'B';
    buffer[2] = 'Y';
    buffer[3] = 'E';
    buffer[4] = 'S';
    buffer[5] = 'U';
    buffer[6] = 'D';

    return write((const unsigned char*)&buffer, 65) == 65;
}

void SudController::close()
{
    if (handle != NULL) {
        hid_close(handle);
    }
}

int SudController::request()
{
    memset(buffer, 0x00, 65);

    buffer[1] = 'R';
    buffer[2] = 'E';
    buffer[3] = 'A';
    buffer[4] = 'D';
    buffer[5] = 'I';
    buffer[6] = 'N';
    buffer[7] = 'G';

    return write((const unsigned char*)&buffer, 65) == 65;
}

int SudController::setLeds(char *ledValues)
{
    memset(buffer, 0x00, 65);

    buffer[1] = 'L';
    buffer[2] = 'E';
    buffer[3] = 'D';
    for (int i = 0; i < 5; i++)
    {
        char value = ledValues[i];
        if (value < '0') {
            value += '0';
        }
        buffer[i + 4] =  value;
    }

    return write((const unsigned char*)&buffer, 65) == 65;
}

SudData *SudController::readData()
{
    memset(buffer, 0x00, 65);

    int res = hid_read_timeout(handle, buffer, 64, 5000);
    if (res <= 0) {
        return NULL;
    }

    if (callback != NULL) {
        callback(1, buffer, 64);
    }

    SudData *data = new SudData();
    data->mode = buffer[0];
    data->type = buffer[1];

    switch (data->mode) {
        case 0x00:
            switch (data->type) {
                case 1:
                    readAllValues(data, &buffer[2]);
                    data->fullReading = true;
                    break;
                case 2:
                    data->isKelvin = buffer[2] & 1;
                    readLmValues(data, &buffer[6]);
                    data->fullReading = false;
                    break;
            }
            break;
        case 0x77:
            data->success = buffer[2];
            break;
        case 0x88:
            data->success = buffer[2];
            switch (data->type) {
                case 0x01:
                    {
                        int deviceVersion = (buffer[5] << 8) + buffer[4];
                        data->deviceType = buffer[3];
                        data->modelName = getModelName(buffer[3]);
                        data->version[0] = deviceVersion / 10000;
                        data->version[1] = (deviceVersion / 100) % 100;
                        data->version[2] = deviceVersion % 100;
                    }
                    break;
                case 0x02:
                case 0x03:
                    break;
            }
            break;
    }

    return data;
}

const unsigned char *SudController::getRawData()
{
    return (const unsigned char *)&buffer;
}

const char *SudController::getModelName(unsigned char deviceType)
{
    const char *modelName;
    switch (deviceType) {
        case 0:
            modelName = "Home";
            break;
        case 1:
            modelName = "Home";
            break;
        case 2:
            modelName = "Pound";
            break;
        case 3:
            modelName = "Reef";
            break;
        default:
            modelName = "<unknown>";
            break;
    }

    return modelName;
}

int SudController::write(const unsigned char *buffer, size_t size)
{
    int res = hid_write(handle, buffer, size);
    if (callback != NULL) {
        callback(0, buffer, 64);
    }

    return res;
}

void SudController::readAllValues(SudData *data, const unsigned char *buffer)
{
    data->timestamp = WORD32(&buffer[0]);
    data->inWater = (buffer[4] >> 2) & 1;
    data->slideNotFitted = (buffer[4] >> 3) & 1;
    data->slideExpired = (buffer[4] >> 4) & 1;
    data->stateT = (buffer[4] >> 5) & 3;
    data->statePh = (buffer[4] >> 7) + ((buffer[5] << 1) & 2);
    data->stateNh3 = (buffer[5] >> 1) & 3;
    data->error = (buffer[5] >> 3) & 1;
    data->isKelvin = (buffer[5] >> 4) & 1;
    data->ph = WORD16(&buffer[8]);
    data->nh3 = WORD16(&buffer[10]);
    data->temp = WORD32(&buffer[12]);
    readLmValues(data, &buffer[32]);
}

void SudController::readLmValues(SudData *data, const unsigned char *buffer)
{
    data->kelvin = WORD32(&buffer[8]);
    data->x = WORD32(&buffer[12]);
    data->y = WORD32(&buffer[16]);
    data->par = WORD32(&buffer[20]);
    data->lux = WORD32(&buffer[24]);
    data->pur = buffer[28];
}
