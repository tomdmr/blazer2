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

There will be a [wiki](wikiurl) for the full documentation of software and hardware. Bear with me...

## Compile ##

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

Compilation is simply done with

``` shell
$ pio run
```

