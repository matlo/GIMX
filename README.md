GIMX
====

[![Build Status](https://travis-ci.com/matlo/GIMX.svg?branch=master)](https://travis-ci.com/matlo/GIMX)

This document needs updating.  This is a fork of what I assume is abandoned software since the dev stopped selling GIMX adapters and his forum and the repo are dead for over 5 years.  

I've updated code in GIMX to use modern Linux dependencies so it builds on updated systems, tested on cachyos.  I've also updated joypad calls and functions to use evdev now instead of joydev on Linux.  This makes the app work again where it only functioned with simple digital inputs before and not analog axis or other features.  

At this time the config editor is only capable of detecting the evdev controller in "slot 1" so you must disconnect and reconnect stuff to get into slot 1 to recognize it, but it supports full features of Linux devices again.  

```
make
sudo make install
```

That's all you (should) need to do to install it on Linux.

I am assuming the original code works on Windows still as is, so Windows users should try that first, the original project has executable binaries for Windows ready for you.

I do not know how far I will take this project but I at least intend to make evdev work a little better and see where it goes after that.

....

GIMX is a free software that allows to use a computer as a hub for your gaming devices. It works on Windows® and GNU/Linux platforms. It is compatible with Playstation® and Xbox® gaming consoles. The connection between the computer and the gaming console is performed using a USB adapter – [get one on the GIMX shop!](https://blog.gimx.fr/product/gimx-adapter/) – or a Bluetooth® dongle (PS3/PS4 only). The capabilities depend on the platform, the connection method, and the gaming platform.

Links:
* [Documentation](https://wiki.gimx.fr)  
* [Source code](https://gimx.fr/source)  
* [Issue tracker](https://gimx.fr/buglist)  
* Licence: [GPLv3](https://www.gnu.org/copyleft/gpl.html)  
* [Donations](https://blog.gimx.fr/give/gimx-donations-current/)
