DESIGN A PIG FARMING SYSTEM <br> <br>
1/ Graduation thesis defense grade: 8.0/10.0 <br> <br>
2/ Motivation: It is an Iot project that help the farm owner work more convenience and properly. <br> <br>
3/ Components: Sensors, Peripheral devices, Relay 8-channel, Power supply, LCD2004 which displays the data of sensor, state of devices, Module SIM. <br> <br>
4/ Features: <br> <br>
5/ Block diagram of overall system:

![Block_Diagram_MODIFIED drawio](https://github.com/user-attachments/assets/898711a0-d6e6-46fa-97a2-71a228c5fdeb)

Microcontroller
+ Arduino's Board: Control devices.
+ ESP32's Module: Connect to Wifi.

Input:

+ Sensors: DHT11, flame sensors, MQ135 sensor.
+ Buttons: turn on/off devices without internet connection.
  
Output:

+ 8-channel relay module: on/off devices.
+ LCD2004: Display the status (on/off) of devices.
+ GSM Module Sim: will send the alert message to mobile phone when the MQ135 or flame sensor detect smoke or fire.
+ Buzzer: will alerts when the MQ135 or flame sensor detect smoke or fire. <br>

Power Supply:
+ 12V power supply for Arduino's board and GSM.
+ Buck Converter LM2596.
+ 5V power supply for ESP32, sensors, buttons, LCD, Relay module and buzzer.  <br>  <br> 

6/ Flowchart of overall system:

- Arduino's Flowchart:

![Chuong_trinh_arduino drawio (3)](https://github.com/user-attachments/assets/e0b0f613-7dcd-45b9-bfa8-b7a19bff69fd)

- ESP32's Flowchart:

![ESP32 drawio (2)](https://github.com/user-attachments/assets/cfb42545-f387-46a5-ae08-1a8b5a586bad)  <br> 


7/ Schematic: 
![final-1](https://github.com/user-attachments/assets/1f9c0318-6fc8-43f7-94f5-7b9696efbcf1) <br> 

8/ PCB Layout:

![image](https://github.com/user-attachments/assets/7443ec51-5919-4b4e-acfe-1d489d96bbc7) <br> 

9/ 3D Layout:

![image](https://github.com/user-attachments/assets/733c8c5b-8267-4ce3-9f7c-036726f7c27f) <br> 

10/ Mobile GUI:

![image](https://github.com/user-attachments/assets/b2aa8cc2-ca91-42a3-9bca-fbe703c36b3c) ![image](https://github.com/user-attachments/assets/c3ce51bd-a07a-44a6-87de-6f53b4f3cc52)  
<br> 

11/ Final product:


![z5588712647974_7ea4d7834b79aa95a4724e01ca06e532](https://github.com/user-attachments/assets/955dd354-057e-4877-b89e-ed0e7a45aeff) <br> 

![z5588683793541_be4eea91575632ce1d72c8d7e3c11ac4](https://github.com/user-attachments/assets/091c49b2-4dd5-404a-8059-8feea2fa4d4e) <br> 

