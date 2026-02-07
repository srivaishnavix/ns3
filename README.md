# How to install ns3

Note: This documentation is made for ns3 3.46 version

Ns3 is a Open source Network simulator.

What OS does ns3 run on?

It runs on Linux or Unix based OS.

Sad news for all the windows users. But we have multiple ways to run ns3 in Windows. 
Obviously running ns3 on Linux is the most effective and hassle free way.

So ns3 can run on MacOS since it is a Unix based system. 

So here are a list of ways to run ns3 on a System

- Directly run it in Linux based system
  - Using a Linux based System
  - Dual boot Linux in your windows based system
  - Dual boot Linux in MacOS (Mac has recently enabled
- To run it in windows
  - Virtual Machine (VM)-basically it is like a disk partition in which you can install a new OS
  - WSL (Windows Subsystem for Linux)
- Macos

#Prerequisites of ns3 
- Python (3.11 or later versions)
- 


## MacOS

For MacOS one thing to keep in mind is that ns3 wouldn't run with python in the m1 and m2 chips. So trying to build it with python bindings would be a waste of time, as the build fails everytime. 

So for systems with m1 or m2 chips it is better to just settle with C++ for compiling the codes. 


