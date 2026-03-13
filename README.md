GIMX
====

[![Build Status](https://travis-ci.com/matlo/GIMX.svg?branch=master)](https://travis-ci.com/matlo/GIMX)

GIMX is a free software that allows to use a computer as a hub for your gaming devices. It works on Windows® and GNU/Linux platforms. It is compatible with Playstation® and Xbox® gaming consoles. The connection between the computer and the gaming console is performed using a USB adapter – [get one on the GIMX shop!](https://blog.gimx.fr/product/gimx-adapter/) – or a Bluetooth® dongle (PS3/PS4 only). The capabilities depend on the platform, the connection method, and the gaming platform.

GIMX on Linux now has evdev support, which is more supported on modern Linux kernels.  

This is a fork of GIMX for Linux systems where evdev support has resulted in joydev neglect and degraded functionality.  

If you find you're using GIMX by matlo on a certain non-ubuntu distro and it just refuses to do something like, accept proper analog joystick or throttle inputs, it might be a critical kernel level difference in expected behaviour and your best bet is to either swithc to the OS distro he built the software on, or try out evdev.  

**WARNING** this version is not officially supported by matlo, and while you can download all the configs from their repository, they will NOT work!  This is because evdev ID numbers are different and names of devices might be fetched differently.  You won't be able to match any old configs.  But you CAN still download them and use them as references for remapping, as many of them have nice advanced features and offer structure to work from.

Please report any bugs with identifying your devices, and send your device and device ID found with `evdev-test` or `cat /proc/bus/input/devices`

Links:
* [Documentation](https://wiki.gimx.fr)  
* [Source code](https://gimx.fr/source)  
* [Issue tracker](https://gimx.fr/buglist)  
* Licence: [GPLv3](https://www.gnu.org/copyleft/gpl.html)  
* [Donations](https://blog.gimx.fr/give/gimx-donations-current/)
