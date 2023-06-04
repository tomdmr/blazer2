# Blazer-Project #

The whole thing is inspired by a post from the Volleyballfreak Trainer
Talk. I had seen these flashing things once before at the warm-up of
Rote Raben, and I wanted to have those. Learning what they really cost
was sobering up, so I thought: Well then, build one yourself.

So basically, this is an ESP32 in Wifi mode, with lights and a
button. Lights can be controlled by a web browser. I want realize
things like this in my coachnig session: I have three of these devices
on the other side, long line, diagonally, and one on the market
place. The setter gets a ball and sets to our IV. At some time during
the attacker's approach, one of the devices lights up and the attacker
has this a a target.

## Documentation ##

There will be a [wiki](https://github.com/tomdmr/blazer2/wiki) for the
full documentation of software and hardware. Bear with me...

## Compile and Install ##

**This is outdated!**
This is based on PlatformIO, so you need this framework. For
compilation, you will need to add one file `credentials.h` in the
`include` directory. This file should look like this:

``` c++
#ifndef _credentials_h
#define _credentials_h
#define HOMESSID     "my_ssid"
#define HOMEPASSWD   "homepassword"

#define HALLESSID     "ssid_in_gym"
#define HALLEPASSWD   "password_in_gym"
#endif
``` 

**New procedure**

As of v0.9.9-rc1 the WiFi-credentials are stored in a file in the
SPIFFS-filesystem. To update from an older version, you need to first
create the file `APConfig.txt' in the `data`-directory. This file
contains per network two lines, first the SSID and then the password. After creating this file, you need to

``` shell
$ make uploadfs_ota
```

first, before updating the code. This makes sure that the file is accessible at next boot.

Compilation is simply done with

``` shell
$ pio run
```

The web-pages on the controller are saved on the SPIFFS-Filesystem,
which is really simplisitc. The add some flexibility, all file paths
under `www` are hashed into a 16-char string and saved in `data`. The
webserver on the ESP performs the same hashing operation and serves
the file with the hashed name. On the PC-side, the hashing is done
with the `djb2` program in `tools`. I stole the program from DJB, so
probably the function is pretty good. Adjust the location of the executable in `Makefile`.

Installation must be done first time via USB:

``` shell
$ make all && make uploadfs
$ pio run --target=upload
```

The first command creates and uploads the file system, the second one the executable. 

Further installs or file system updates can be done over Wifi. The ESP must be in the net and awake:

``` shell
$ TARGETIP=x.x.x.x make uploadfs_ota
$ TARGETIP=x.x.x.x make upload_ota

```

