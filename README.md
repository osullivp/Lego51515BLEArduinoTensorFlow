# Lego51515ArduinoOpenCV

## Objective

The aim of this project is to create Lego Mindstorms 51515 robot that can detect and retrieve a pre-defined object.

At the time of writing (Feb 2021) there are no official or third party sensors that deliver an object detection capability for the Lego Mindstorms 51515 set. Previously, a rudimentary object detection camera capability was provided by the third party Pixy2 camera (https://pixycam.com/pixy2/). In fact a project using this camera was the inspiration for the current project:

https://www.youtube.com/watch?v=aYrysCYupw8

In searching for a solution to this problem I set out the following criteria:

- Create a project that can be relatively simply copied and extended by other users
- The Lego components should be limited to the stock 51515 set
- Other elements should be as 'off the shelf' as possible i.e. keeping hacking to the minimum
- Avoid having to custom code the object detection element as this is would create a high barrier to entry


## Solution

Having looked at a number of possible solutions I settled on a three element solution as follows:

#### Element 1
Lego 51515 set in stock configuration but loaded with the Spike Prime firmware as at Feb 2021, the Lego 51515 firmware does not include the ubluetooth module for BluetoothLE comms
The robot design was based on the 'Tricky' robot from the 51515 set but with the addition of a grabber using this design: https://www.youtube.com/watch?v=gkszh4ap4pI
The above grabber design has the benefit of grabbing and then lifting the object using a single motor
I then added some additional elements in order to create a deck for the Arduino board and battery etc.
The software consists of a single MicroPython file that contains code to a) connect to the HM-10 module on the Arduino board and b) handle movement, edge detection, controlling the grabber etc.

#### Element 2
An Arduino board based on the Arduino Micro. The board also contains both an HC-05 Bluetooth Classic module and an HM-10 BluetoothLE module
The reason for both types of Bluetooth module is that the object detection app described in the next section communicates via Bluetooth Classic, yet the Lego hub communicates via BluetoothLE

#### Element 3
The final element is the Ard Object Detection app which can be found here (Android version): (https://play.google.com/store/apps/details?id=com.studios.code.gem.ardobjecttracker&hl=en_GB&gl=US)
This is the key element in the solution as it provides an off the shelf way to detect objects without having to custom code a TensorFlow app in either Android or RPi Python. Both these other options are technically challenging.
I did also consider using AppInventor as this contains a number of [object detection extensions] (https://mit-cml.github.io/extensions/), but unfortunately these solutions were limited or too slow. I did
