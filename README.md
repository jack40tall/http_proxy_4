# HTTP Proxy Lab

## Overview
This is a HTTP proxy built in C, that is able to both local and global requests. 

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
