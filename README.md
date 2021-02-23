# Lego51515BLEArduinoOpenCV

## Objective

The aim of this project is to create a Lego Mindstorms 51515 robot that can detect, locate and then pick up a pre-defined object using computer vision.

At the time of writing (Feb 2021) there are no 'off the shelf' official or third party sensors that deliver an object detection capability for the Lego Mindstorms 51515 set. Previously, a simple object detection camera capability was provided for the EV3 system by the third party [Pixy2 camera](https://pixycam.com/pixy2/). In fact, this EV3 solution using the Pixy camera was the inspiration for the current project:

- https://www.youtube.com/watch?v=aYrysCYupw8

In searching for a solution to this problem I established the following criteria:

- Demonstrate the integration of computer vision into a Lego 51515 project
- Demonstrate Bluetooth communication with a non-Lego peripheral
- Create a project that can be relatively simply copied and extended by other users
- For the Lego part of the solutoion, only use components from the stock 51515 set

## Solution

I settled on a three part solution. Note that only the code elements are contained in this repository as the basic robot design is based on the Tricky robot in the 51515 set and the link to the grabber design is included below. The following gives an overview of the solution elements and design choices:

#### Part 1
- A single Lego 51515 set in stock configuration but loaded with the Spike Prime firmware. This is because as at Feb 2021, the Lego 51515 firmware does not include the ubluetooth module required for BluetoothLE comms
- The robot design was based on the 'Tricky' robot from the 51515 set but with the addition of a grabber [using this design](https://www.youtube.com/watch?v=gkszh4ap4pI). This grabber design has the benefit of grabbing and then lifting the object using a single motor
- I then added some additional elements in order to create a deck for the Arduino board and battery etc.
- The software consists of a single MicroPython file that contains code to a) connect to the HM-10 module on the Arduino board and b) handle movement, edge detection, controlling the grabber etc.

#### Part 2
- An Arduino board based on the Arduino Micro. The board also contains both an HC-05 Bluetooth Classic module and an HM-10 BluetoothLE module
- The reason for using both types of Bluetooth module is that the object detection app described in the next section communicates via Bluetooth Classic, yet the Lego hub communicates via BluetoothLE

#### Part 3
- The final element in the solution is the Ard Object Detection app which can be found [here](https://play.google.com/store/apps/details?id=com.studios.code.gem.ardobjecttracker&hl=en_GB&gl=US) (Android app)
- The Ard Object Detection app provides an object detection capability via the OpenCV libraries and then exposes this as text over a Bluetooth Classic connection
- This is the key element in the solution as it provides a way to detect objects without having to custom code a TensorFlow app in either Android or RPi Python
- I did also consider using AppInventor, as this contains a number of [object detection extensions](https://mit-cml.github.io/extensions/), but unfortunately these solutions were limited or too slow
- I also experimented with a strategy of sticking simple text to the objects to be picked up and programmatically taking pics via AppInventor and then passing these to the [Google Cloud Text Detection API](https://cloud.google.com/vision/docs/ocr). This actually worked quite well but was too slow vs the Ard Detection App
- Note that the Ard Object Detection app implements the [COCO dataset](https://cocodataset.org/#home). Therefore to avoid having the train a custom model, I decided to use a 'cup' as the object for the robot to pick up. Being a cyclinder, a cup has the added benefit that the robot grabber tool can pick it up consistently from all angles

## Key Challenges and Learnings

#### BluetoothLE
Using BluetoothLE on the Lego Prime/51515 hub is currently not well supported and there are very few forum posts. I started to make much more progess however by reading the following:

- MicroPython 1.12 documentation for the uBluetooth module: http://docs.micropython.org/en/v1.12/library/ubluetooth.html - I wasted a lot of time looking at the most recent documentation 1.14 whereas the Spike Prime firmware uses the older version
- This excellent thread on Eurobricks: https://www.eurobricks.com/forum/index.php?/forums/topic/180029-spike-prime-powerup-remote-in-python-noob-needs-help/
- These official MicroPython Bluetooth code examples (warning - they need to be adapted for the earlier version of MicroPython to work with the Lego hub): https://github.com/micropython/micropython/tree/master/examples/bluetooth

Even with the above resources, there are a lot of device specific implementation issues. For instance:

- I needed to use 16bit UUIDs to identify the services and characteristics of the HM-10. You will see a lot of code examples that use 128bit
- The HM-10 doesn't seem to supply service data in a passive Bluetooth scan so I had to identify the device by the MAC address (contrary to most examples)
- I had a lot of issues testing the Bluetooth features on the Lego hub and it took me ages to realise that for some reason you need to completely power down the hub and power up again between code executions. User Vinz alludes to this issue in the Eurobricks thread above. His solution was to detach the USB cable from the Lego hub. I didn't try that myself as just using the hub display for debug would be too limiting for me
- I would strongly recommend reading an article [like this](https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch04.html) to get an overview of how the BluetoothLE system works before delving into any code

#### Arduino
I used the Arduino Micro which has one hardware UART in addition to the one for the USB connector. This meant that I needed to put one of the Bluetooth adaptors on a SoftwareSerial connection. Let me save you a lot of time if you are going to use a different Arduino by sharing these link:

- Table of UARTs available by Arduino board type: https://www.arduino.cc/reference/en/language/functions/communication/serial/
- Check the limitations section in this link. Only certain pins can be used for Rx with SoftwareSerial: https://www.arduino.cc/en/Reference/softwareSerial

Other than that, I would say that the Arduino part of the solution was one of the simplest parts. The Arduino ecosystem is very well supported and coding is very immediate. In fact I decided to use Arduino rather than RPi for this element of the solution to avoid all the issues with Python setup on a RPi and the fact that there are multiple RPi Bluetooth libraries and they don't seem that well covered on forums. The benefit of the HC-05 and HM-10 modules on Arduino is that they use a simple UART serial implementation which (for me at any rate) simplified things enormously.

One thing to look out for when using the HC-05 and HM-10 modules is that internally they operate at 3.3v vs 5v used by most Arduinos. This necessitates a voltage divider to be implemented between the Tx of the Arduino and the Rx of the Bluetooth module. Here are a couple of useful articles on the HC-05 and HM-10 modules:

- HC-05: https://create.arduino.cc/projecthub/electropeak/getting-started-with-hc-05-bluetooth-module-arduino-e0ca81
- HM-10 (excellent article): http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/

Here are some other helpful articles I found on my journey:

##### Spike Prime Python
- Some good Lego Spike Prime/51515 Python tutorials: https://primelessons.org/en/Lessons.html
- Great example of connecting to BluetoothLE devices from Spike Prime in Python: https://github.com/Vinz1911/PrimePowerUP
- Issues with async on Lego Spike Prime/51515: https://www.eurobricks.com/forum/index.php?/forums/topic/182199-ri5-how-to-do-parallel-activities-using-python/&tab=comments#comment-3333700

##### BluetoothLE
- Official Micropython BluetoothLE code examples: https://github.com/micropython/micropython/tree/master/examples/bluetooth
- O'Reilly article explain GATT services and characteristics in BluetoothLE: https://www.oreilly.com/library/view/getting-started-with/9781491900550/ch04.html
- Helpful forum post about Bluetooth scanning: https://forum.micropython.org/viewtopic.php?t=7206
- Forum question about subscribing to notifications on a Bluetooth device: https://forum.micropython.org/viewtopic.php?t=8549
- Forum question about subscribing to notifications on a Bluetooth device: https://github.com/micropython/micropython/issues/6185

##### Ard Object Tracking App
- Github page for the Ard Object Tracking app: https://github.com/GemcodeStudios/ObjectDetectionTracking

##### TensorFlow
- TensorFlow getting started guide: https://www.tensorflow.org/lite/guide/get_started
- Forum post about converting .pb format to .tflite: https://stackoverflow.com/questions/44329185/convert-a-graph-proto-pb-pbtxt-to-a-savedmodel-for-use-in-tensorflow-serving-o/44329200#44329200
- Convert frozen inference graph to saved model: https://stackoverflow.com/questions/59657166/convert-frozen-model-pb-to-savedmodel
- How to get input and output tensors from a frozen inference graph: https://stackoverflow.com/questions/54495686/obtain-input-array-and-output-array-items-to-convert-model-to-tflite-format
- Forum post about converting .pb format to .tflite: https://stackoverflow.com/questions/62159812/tensorflow-converting-savedmodel-pb-file-to-tflite-using-tensorflow-2-2-0#comment109938229_62160223
- Platform to simplify creation of custom training data for object detection: https://app.roboflow.com/
- TensorFlow Python docs: https://www.tensorflow.org/lite/guide/python
- Implementing TensorFlow on Raspberry PI: https://github.com/tensorflow/examples/tree/master/lite/examples/image_classification/raspberry_pi

##### AppInventor
- Sending BluetoothLE advertisements in AppInventor: https://community.appinventor.mit.edu/t/create-and-send-ble-advertisements/17557/19
- HM-10 Bluetooth LE module and AppInventor:http://www.martyncurrey.com/arduino-hm-10-and-app-inventor-2/
- BluetoothLE extension for AppInventor: http://iot.appinventor.mit.edu/#/bluetoothle/bluetoothleintro
- Extension for converting image to Base64: http://kio4.com/appinventor/277_extension_imagen_string.htm

##### RPi/Jetson Nano
- Making a RPi OS Image: https://www.raspberrypi.org/blog/raspberry-pi-imager-imaging-utility/
- Controlling Spike Prime using serial terminal: https://github.com/beemsoft/lego-mindstorms-51515-jetson-nano
- Example of EV3/RPi/TensorFlow sorting machine: https://youtu.be/wSSh9iV70ng

##### Arduino
- RPi and Arduino Serial comms: http://www.penguintutor.com/electronics/rpi-arduino
- Opening serial on Arduino Micro does not reset board: https://www.arduino.cc/en/Guide/ArduinoLeonardoMicro
- How to wire up single RGB LED:https://learn.sparkfun.com/tutorials/experiment-guide-for-the-johnny-five-inventors-kit/experiment-8-driving-an-rgb-led#:~:text=The%20RGB%20LED%20in%20your,longer%20than%20the%20other%20legs.
- Specifics of Arduino Micro serial implementation: https://arduino.stackexchange.com/questions/20836/ifserial-not-working-correctly-on-micro-or-pro-micro


