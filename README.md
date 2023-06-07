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

The build system is based on
[PlatformIO](https://www.platformio.org/), with a bit of GNU-make.
Personally, I get along perfectly with the CLI-Version. 

The directory structure is:

  * `include`: CPP-Headers
  * `src`: Sources
  * `www`: html, js and css files that will be uploaded to the ESP32
  * `tools`: source of a little hashing tool for filenames and a helper for `make`
  * `data` : Initially empty, receives the files that are finally uploaded.

### Before you start ###

Create the file `include/credentials.h`, like this:

``` c++
#ifndef _credentials_h
#define _credentials_h
#define HOMESSID     "my_ssid"
#define HOMEPASSWD   "homepassword"
#endif
``` 
This will allow your device to connect to your home/development WiFi.

Additionally, as of v0.9.9-rc1 WiFi-credentials are stored in a file
in the SPIFFS-filesystem. The file `APConfig.txt` contains
**additional** networks the device can connect to. The format is a
pair of lines per network, first the SSID and then the password.

Check the file `include/config.h` and select if your hardware has a
button on top, or if you want to have a touch-sensitive area on top.

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

Congratulations, almost done. By default, every chip believes that its
name is `esp-default`. If you have several of them, your local
dns-resolver may run into trouble. to fix this, start the device and point your browser to this URL:

`http://device_ip/config?lan=xxx&pw=xxx&name=NEWNAME`

The triple-x don't mean anything, NEWNAME is the new device name. Yes,
this is a hack. All my ESPs have a unique name, and I completely
forgot about this detail...
