# Botnet
Lightweight botnet written in c++
## Getting started
Currently this botnet works only on Linux, tested on Ubuntu and should work on mac OS and other versions of Unix
### Installing
First clone the package by entering this in your terminal:
```
git clone https://github.com/OmriHab/Botnet.git
```
Then cd into the folder `cd Botnet`, and make it by using `make`  
Run the server using `./BotnetServer`  
Run the bot using `./BotnetBot`
## Botnet Server Commands
***Botnet Server and Client both use symetric encryption***  
When using the botnet server CLI, you will be given the options of:
+ Print Bots:
  + Prints list of connected bots with their IP address and Id.
+ Syn Flood:
  + Gives order to syn flood a requested IP and port.
+ Stop Flood:
  + Gives order to stop the syn flood started by "Syn Flood".
+ Get Info:
  + Returns info about bots computer.
+ Get File:
  + Attempts to download a file from the bots computer to the directory Botloads.
+ Update Bot:
  + Updates bot with given executable. **Read the warning printed by the command before using it**.
