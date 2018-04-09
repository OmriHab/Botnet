# Botnet
Lightweight botnet written in c++
## Getting started
Currently this botnet works only on linux, tested on Ubuntu 16.04 and should work on mac OS and other version of Unix
### Installing
First clone the package by entering this in your terminal:
```
git clone https://github.com/OmriHab/Botnet.git
```
Then cd into the folder `cd Botnet`, and make it by using `make` <br>
Run the server using `./BotnetServer`<br>
Run the bot using `./BotnetBot`
## Botnet Server Commands
When using the botnet server CLI, you will be givven the options of:<br>
	Print Bots:<br>
		Prints list of connected bots with their IP address and Id<br>
	Syn Flood:<br>
		Gives order to syn flood a requested IP and port<br>
	Stop Flood:<br>
		Gives order to stop the syn flood started by "Syn Flood"<br>
	Get Info:<br>
		Returns info about bots computer<br>
	Get File:<br>
		Attempts to download a file from the bots computer to the directory Botloads<br>
