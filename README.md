# 128x64 LCD Module with FPGA



## System requirements

- Windows computer (recommended) with USB port and Ethernet Port
- Ethernet Router
- [Quartus II Standard Version 18.1](http://fpgasoftware.intel.com/18.1/?edition=standard&platform=windows&download_manager=direct) installed with Cyclone V
- [Altera SoC EDS (Embedded Design Suite)](http://fpgasoftware.intel.com/soceds/18.1/?edition=standard&platform=windows&download_manager=direct) software tool installed (Standard version, Direct Download)
- Text Editor ([Sublime text](http://fpgasoftware.intel.com/soceds/18.1/?edition=standard&platform=windows&download_manager=direct) or [VS Code](https://code.visualstudio.com/) recommended)
- [Putty](https://www.putty.org/) installed
- [FTDI Virtual COM driver](https://www.ftdichip.com/Drivers/VCP.htm) installed
- A bootable SD card (min. 4GB of memory, at least class 4)
- [Linux Console](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=205&No=1081&PartNo=4) image downloaded (you will need to register in Terasic webpage)
- [Win32 Disk Imager](https://sourceforge.net/projects/win32diskimager/) installed

## Hardware Requierements

- Altera DE10-Standard Development Kit
- UART-to-USB cable
- Ethernet cable
- USB Blaster cable


## Project in Quartus II

1. Run Quartus and go to File -> Open Project 
2. Go to the folder of this repository and select the file: /DE10_Standard_GHRD/DE10_Standard_GHRD.qpf
3. Go to Tools -> Platform designer (or Qsys) and open the file: /DE10_Standard_GHRD/soc_system.qsys
4. Click the menu item Generate->Generate HDL
5. Close the Platform Designer and open the file: /DE10_Standard_GHRD/LCD_control.sv
6. **In this file you will write all the logic to control the LCD** 
7. Make sure that the file DE10_Standard_GHRD.v is set as Top Leven Entity
8. Compile your project with the button Start Compilation
9. Click on Tools->Programmer
10. Make sure that the Altera device is visible to the program in 
Hardware Setup (if you don't have the drivers installed go to DE10-Standard_My_First_Fpga.pdf in de CD)
11. Click on Auto Detect
12. Select 5CSXFC6D6 and click OK
13. Click Start to program .sof file into the FPGA

## C code
The C code to control the LCD via the HPS is already given, so you don't have to worry about it. The only thing you have to do is to compile the c file and send the executable to the FPGA. To accomplish this, you have to:

1. Browse the SoC EDS installation folder (e.g. "C:/intelFPGA/16.1/embedded")
2. Double click the Embedded_Command_Shell.bat item to launch the Command Shell
3. Change the directory to the /Code folder of this repository
4. Run the following commands:
```bash
$ make clean
$ make
```
5. This commands will generate the file hps_fpga

## Configure Linux

We assume you know how to boot Linux in the DE-10 Standard board (Refer to the chapter 5.1 of the Getting Started Guide of the CD). 

1. Connect the USB-to-UART cable to your computer and the Ethernet Cable to the router
2. Open Putty and configure the UART Terminal as explained in the chapter 5.3 of the Getting Started Guide of the CD
3. Click open. The linux system will start to boot
4. Type root to enter the system
5. Configure the IP address of the device with the following commands:

```bash
$ ifconfig eth0 up
$ udhcpc
$ ifconfig
```

6. Copy the IP address of the eth0. In this case we have 172.17.1.71
7. In another window, open the Bash Terminal in the folder /Code of this repository
8. Send the file hps_fpga with scp with the following command:
```bash
$ scp hps_fpga root@172.17.1.71:/home/root
```
9. Back in the Putty Terminal type ls to make sure that the file was transferred correctly
10. Execute the file
```bash
$ ./hps_fpga
```

## License
[MIT](https://choosealicense.com/licenses/mit/)