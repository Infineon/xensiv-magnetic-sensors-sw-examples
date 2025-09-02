# MOTIX: TLE5012B Angle Sensor interfacing with TLE9879 Motix Controller and using PWM protocol

This code example demonstrates how to interface [TLE5012B](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/angle-sensors/tle5012b-e1000/) angle sensor with [TLE9879](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/angle-sensors/tle5012b-e1000/) Motix Controller using PWM protocol. The sensor sends pulse width modulated signal with duty cycle in the range of 6.25% to 93.5% which corresponds to 0 deg to 360 deg. This duty cycle is then modified to retrive angular information

This setup is implemented using the Keil IDE and Tera Term for UART communication, providing real-time angle monitoring and accurate angle value being displayed on the terminal.

[View this README on GitHub.](replace_code_example_github_readme_url)


## Requirements

- Keil uVision5 IDE
- Programming language: C
- [TLE5012B](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/angle-sensors/tle5012b-e1000/)
- [TLE9879](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/angle-sensors/tle5012b-e1000/)



## Supported kits (make variable 'TARGET')

- [TLE9879 EVALKIT](https://www.infineon.com/evaluation-board/TLE9879-EVALKIT)
- [TLE5012B MS2GO KIT](https://www.infineon.com/evaluation-board/TLE5012B-E5000-MS2GO)

## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

Use jumper wires to establish a connection between TLE5012B Angle sensor and the MOTORCONTROL_12V kit as mentioned below.

1. Connect **VCC** of the sensor to the **3.3V** supply on the TLE9879 EVALKIT kit.
2. Connect the **GND** pin of the sensor to the **GND** of the TLE9879 EVALKIT kit.
3. Connect the **PWM** (IFA) **Pin 5** of the sensor to  **P2.3** of the TLE9879 EVALKIT kit.

Refer to the Motorcontrol Kit.

   **Figure 2. Connection Pin out of between TLE9879 EVALKIT**
   
   ![](Images/1_Motorcontrol_Kit.png)


## Software setup

**Step 1**: Install Keil uVision5 IDE.

**Step 2**: Install a terminal emulator if you don't have one. Instructions in this document use [Tera Term](https://teratermproject.github.io/index-en.html).

**Step 3**: Open uVision, Click on **project**, then click on **New uVision project** and select destination of project and then target selection window pops up. 

**Step 4**: Inside target selection, select the target controller.

![](Images/2_Controller_Selection.png)

**Step 5**: Select Run-Time Environment dependencies. Select the dependenices shown with tick marks.

![](Images/3_1_Run-time%20env.png)
![](Images/3_2_Run-time%20env.png)

**Step 6**: After selecting dependies, project will be created and files will be available at in project window. 

![](Images/4_project_window.png)

**Step 7**: Add the main file to the existing project. 

![](Images/5_main.png)

**Step 8**: Build, Compile, Flash and Execute the code. 

## Operation

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector.

2. Supply 12 V to the MOTORCONTROLKIT board.
    
3. Using Config Wizard for Motix MCU configure the SSC and GPIO peripherals.

4. Configuring SSC:

   a) Select **Configure Timer21** block.
  
   b) Select **Capture Mode**.

   c) Enable External Capture/ Reload Event Enable and Edge select **Rising edge at pin T21EX**.

   d) Enable Start Enable and Edge Select for Ext Start to **Falling edge at pin T21EX**

   e) Enable interrupt.

   f) Assign **pin 2.3** to External Control Input. 
  
   ![](Images/6_ssc_interface.png)


5) Configuring GPIO for Chip Select.

   a) Configure PORT 1 Pin1 as output. This pins will be used as for UART communication.

   ![](Images/7_Slave_select.png)

6) Open the terminal and select baud rate to 115200.

7) Flash the code into the controller and the real time angle value will be visible on the terminal.

![](Images/8_AngleReadout.png)   
   



## Related resources

Resources  | Links
-----------|----------------------------------
Device documentation | [TLE5012B Datasheet](https://www.infineon.com/dgdl/Infineon-TLE5012B_Exxxx-DataSheet-v02_01-EN.pdf?fileId=db3a304334fac4c601350f31c43c433f) <br> [TLE5012B UserManual](https://www.infineon.com/dgdl/Infineon-Angle_Sensor_TLE5012B-UM-v01_02-en-UserManual-v01_02-EN.pdf?fileId=5546d46146d18cb40146ec2eeae4633b&da=t)<br> [TLE5012B Product Page](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/angle-sensors/tle5012b-e1000/) <br>[TLE9879 datasheet](https://www.infineon.com/dgdl/Infineon-TLE9879QXA40-DataSheet-v02_11-EN.pdf?fileId=8ac78c8c81ae03fc0181d840096a3c2f)
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board).
Tools  | Keil uVision5

<br>



## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.




