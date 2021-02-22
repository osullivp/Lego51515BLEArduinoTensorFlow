/*
  Bluetooth Classic to Bluetooth Low Energy bridge for Ard Object Tracker app
  Language: Wiring/Arduino

  This program sends reads object detection data from the Ard Object Tracker app
  As this data is sent via Bluetooth Classic, this module bridges the data to
  BluetoothLE so it can be read by a Lego Spike Prime/Robot Inventor 51515 hub

  The circuit:
  - HM-10 BluetoothLE module on Serial1 (Rx and Tx pins)
  - HC-05 Bluetooth Classic module on SoftwareSerial (pins 14(Rx) and 15(Tx))

  created 22 Feb 2021
  by Paul O'Sullivan

*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(14, 15);  // RX, TX
String sInputString = "";         // a String to hold incoming data
String sObjToFind = "cup:0:";     // object to find - see Ard Object Detection app help for details of format
bool bStringComplete = false;     // whether the string is complete
bool bGetPosition = false;        // has an object detection request been made

void setup() {

  // set software serial pins and data rate for BluetoothLE (HM-10 device)
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  mySerial.begin(9600);

  // set up Bluetooth Classic connection (HC-05 device)
  Serial1.begin(9600);

  // internal serial for monitoring
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Object Detection Starting...");
  
}

void loop() {

  // get object detection data if a request has been made from the Lego hub
  if (bGetPosition) {
    while (mySerial.available()>0) {
      // get the new byte:
      char cInChar = (char)mySerial.read();
      // add it to the sInputString:
      sInputString += cInChar;
      // if the incoming character is a newline, set a flag so the main loop can
      // do something about it:
      if (cInChar == '\n') {
        bStringComplete = true;
        bGetPosition = false;
      }
    }
  // listen for a data request from the Lego hub
  } else if (!bGetPosition){
    if (Serial1.available()>0) {
      Serial.println("HM-10 Available");
      int cInChar = (int)Serial1.read();
      Serial.println(cInChar);
      if (cInChar == 1) {
        bGetPosition = true;
        Serial.println("Read Requested");
      }
    }
  }

  // once a full line of object detection data has been received, send it to the Lego hub
  if (bStringComplete) {
    Serial.println("Raw Data=" + sInputString);
    String sOutput = getObjectDetails(sInputString, sObjToFind);
    if (sOutput != "") {
      Serial.println("Sending data to Lego Hub...");
      char cData[sOutput.length()+1];
      sOutput.toCharArray(cData, sOutput.length());
      Serial.println("Serial1 available...Sending...");
      Serial1.println(sOutput);
    }
    bStringComplete = false;
    sInputString = "";
    sOutput = "";
  }
  
}

// searches for a specific object detection event from the received data
String getObjectDetails(String sData, String sObj) {
  
  String sOutput = "";
  Serial.println("Searching in: " + sData + " for: " + sObj + "...");
  int iObjStart = sData.indexOf(sObj);
  if (iObjStart>=0)
  {
    Serial.println("Object found!");
    int iObjEnd = sData.indexOf(sObj, iObjStart+1);
    if (iObjEnd>iObjStart) {
      sOutput = sData.substring(iObjStart, iObjEnd-1);
    }
  }

  Serial.println("Object data=" + sOutput);
  return sOutput;
  
}
