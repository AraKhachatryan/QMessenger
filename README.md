# QMessenger
Linux/Unix GUI messenger with Qt, POSIX sockets and threads

## Files

<img width="700" height="100%" src="https://pbs.twimg.com/media/EOq7WF9X4AIbsrF?format=jpg">

## Requirements
- g++ compiler with C++11 standart
- Supporting POSIX standarts
- GNU make
- OpenSSL version 1.1.1 or higher
- QMake version 3.1 or higher
- Qt version 5.9.5 or higher

## Compilation and usage:

on server side:

    $ sudo apt-get install libssl-dev
    $ openssl version
    $ make
    $ ./server
    
on client side:

    
    $ sudo apt-get install qt5-default
    $ qmake -v
    $ qmake
    $ make
    $ ./client

### Screenshot:

<img width="875" height="100%" src="https://pbs.twimg.com/media/EOrCgyzWAAEcsoH?format=jpg&name=large">
