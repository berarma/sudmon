# SUDMON

Command line monitor and library for Seneye USB sensor devices.

I've written this software to be able to use the sensors from any computer,
automate tasks and log readings without an internet connection. Currently, the
SCA works only on Windows systems. Having to install Windows to get a device
running is overkill and sometimes inconvenient or plainly impossible. The
Server device requires sending the values to a remote server and then reading
them from there.

This code is based on what I've learned from reading
[the code that Seneye has
kindly released](https://github.com/seneye/SUDDriver). The [writing about the protocol that Doug Hall has written](https://github.com/dhallgb/Seneye-MQTT/blob/master/protocol.mdown) has been also very useful.

The goals of this release are helping debug the protocol and making possible
the automation of the readings at home.

**I'm not employed by or affiliated with Seneye Ltd.**

**This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.**

## Requirements

- Seneye USB device (v2)
- C++ compiler
- CMake 3.7+
- Libraries: HIDAPI

## Build

Installing requirements on Debian GNU/Linux Stretch:

```
apt install g++ cmake libhidapi-dev
```

Building the project:

```
git clone https://github.com/berarma/sudmon sudmon
cd sudmon
mkdir build
cd build
cmake ..
make
sudo make install
```

You will have to run this program as root so it can access the USB device.

There's a udev rule file *99-sud.rule* included that allows using the device to
users of the "sud" group. Copying this file to */etc/udev/rules.d* is
recommended so that you don't have to run the command as root.

Create the "sud" group and add your user to it:

```
sudo addgroup sud
sudo adduser mysuser sud
```

Now you should restart udev and close all your user sessions (or simply reboot).

## Usage

There's many options available. You can read the command help by using:

```
sudmon -h
```

An example command to monitor continuously:

```
sudmon -c -f -t -H 40
```

## Known Problems

- The device has to be registered before using it the first time using the SCA
provided by Seneye.

- Readings stop working and the devices disconnects under some circumstances,
like being out of water or maybe fast changes in light that may confuse the
out-of-water sensor.

