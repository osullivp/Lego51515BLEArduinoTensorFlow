# Lego51515ArduinoOpenCV

###Objective

The aim of this project is to create Lego Mindstorms 51515 robot that can detect and retrieve a pre-defined object.

At the time of writing (Feb 2021) there are no official or third party sensors that deliver an object detection capability for the Lego Mindstorms 51515 set. Previously, a rudimentary object detection camera capability was provided by the third party Pixy2 camera (https://pixycam.com/pixy2/). In fact a project using this camera was the inspiration for the current project:

https://www.youtube.com/watch?v=aYrysCYupw8

In searching for a solution to this problem I set out the following criteria:

- Create a project that can be relatively simply copied and extended by other users
- The Lego components should be limited to the stock 51515 set
- Other elements should be as 'off the shelf' as possible i.e. keeping hacking to the minimum
- Avoid having to custom code the object detection element as this is would create a high barrier to entry

##Solution




Test Arduino project that reads from the Arduino Object Tracking app (https://play.google.com/store/apps/details?id=com.studios.code.gem.ardobjecttracker&hl=en_GB&gl=US) and prints to console.
