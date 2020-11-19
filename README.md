# HTTP Proxy Lab

## Usage
This is a multi-threaded, threadpool implemented HTTP proxy built in C, that is able to handle both local and global requests. To use the Proxy, start the Tiny server in one terminal window:
```
./tiny <portnumber>
```
Then start the proxy in another tab using a different port: 
```
./proxy <portnumber>
```
Finally use the proxy to fetch data with curl:
```
curl -v --proxy http://localhost:<proxyportnumber>/ http://localhost:<tinyportnumber>/
```
Alternatively, you can curl directly from websites, not having to start up the tiny server at all:
```
curl -v --proxy http://localhost:<proxyportnumber> http://google.com/
```

## Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

## port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

## free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

## driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

## nop-server.py
     helper for the autograder.         

## tiny
    Tiny Web server from the CS:APP text
