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

SoftwareSerial mySerial(14, 15);    // RX, TX

// constants
const String OBJ_TO_FIND = "cup:0:";      // object to find
const int SCREEN_WIDTH = 1280;            // max width of Ard Object Tracker screen
const int SCREEN_HEIGHT = 720;            // max height of Ard Object Tracker screen
const int BUTTON_PIN = 18;                // test button data pin
const int TARGET_XPOS = 300;              // posn in tracker app when object is on robot centerline
const unsigned long DEBOUNCE_DELAY = 50;  // delay for button debounce

// calibration data to convert object size and position in tracker to robot movement
int iDistCallib[4][2] = { 
  {400,10},
  {315,15},
  {270,20},
  {225,25}
};
int iTurnCalib = 15;                // object X coord to movement degrees conversion

// object detection data
String sInputString = "";           // incoming data from object detection
String sLastInputString = "";
bool bStringComplete = false;       // has a complete line of object data been received

// button handling
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
int buttonState = 0;                // state of the test button (only used in test mode)
int lastButtonState = LOW;

// state variables
bool bGetPosition = false;          // has an object detection request been made
bool bTestMode = false;             // in test mode, a button press initiates data capture

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  
  // test mode that initiates object data request via
  // button on Arduino board rather than Lego hub
  bTestMode = true;

  // set software serial pins and data rate for BluetoothLE (HM-10 device)
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  mySerial.begin(9600);

  // set up Bluetooth Classic connection (HC-05 device)
  Serial1.begin(9600);

  // internal serial for monitoring
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  Serial.println("Object Detection Starting...");
  
}

void loop() {

  // Get object detection data if a request has been made from Lego hub/test button
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
  // listen for an object detection request
  } else if (!bGetPosition){
    
    if (bTestMode) {
      
      // check if the pushbutton is pressed    
      if (buttonPressed()) {
        bGetPosition = true;
        Serial.println("Read Requested from test button");
      }
      
    } else {
      
      if (Serial1.available()>0) {
        
        Serial.println("HM-10 Available");
        int cInChar = (int)Serial1.read();
        Serial.println(cInChar);
        
        if (cInChar == 1) {
          bGetPosition = true;
          Serial.println("Read Requested from Lego hub");
        }
      
      }      
    }
  }

  // once two full lines of matching object detection data have been received, send to Lego hub
  // checks for two full consecutive lines to ensure a stable set of data
  if (bStringComplete) {
    
    Serial.println("Raw Data=" + sInputString);
    String sOutput = getObjectDetails(sInputString, OBJ_TO_FIND);
    
    if (sOutput != "") {
      
      if (isObjectDataMatching(sOutput, sLastInputString, OBJ_TO_FIND)) {
        
        sLastInputString = "";
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Sending data to Lego Hub...");

        //char cData[sOutput.length()+1];
        //sOutput.toCharArray(cData, sOutput.length());

        String sMvmtData = getLegoMvmtData(sOutput,OBJ_TO_FIND);
        Serial1.print(sMvmtData);
        Serial.println(sMvmtData);     
            
      } else {
        
        sLastInputString = sOutput;
        digitalWrite(LED_BUILTIN, LOW);
      }

    }
    
    bStringComplete = false;
    sInputString = "";
    sOutput = "";
    
  }
  
}

// searches for the specified object in the Ard Object Tracker data
String getObjectDetails(String sData, String sObj) {
  
  String sOutput = "";
  Serial.println("Searching in: " + sData + " for: " + sObj + "...");
  int iObjStart = sData.indexOf(sObj);
  if (iObjStart>=0)
  {
    Serial.println("Start of object at:" + String(iObjStart));
    int iObjEnd = sData.indexOf(sObj, iObjStart+1);
    Serial.println("End of object at:" + String(iObjEnd));
    if (iObjEnd>iObjStart) {
      sOutput = sData.substring(iObjStart, iObjEnd-1);
    } else {
      sOutput = sData.substring(iObjStart, sData.length()-1);
    }
  }

  Serial.println("Object data=" + sOutput);
  return sOutput;
  
}

// gets the x position from a single line of Ard Object Tracker data
int getXPos(String sData, String sObj) {
  
  String sOutput = "";
  Serial.println("Searching in: " + sData + " for: " + "Xpos...");
  int iObjStart = sData.indexOf(sObj);
  if (iObjStart>=0)
  {
    Serial.println("Start of data at: " + String(iObjStart));
    int iObjEnd = sData.indexOf(",", iObjStart+1);
    Serial.println("End of data at: " + String(iObjEnd));
    if (iObjEnd>iObjStart) {
      sOutput = sData.substring(iObjStart+sObj.length(), iObjEnd);
    } 
  }

  Serial.println("XPos=: " + sOutput);
  return sOutput.toInt();
  
}

// gets the object width from a single line of Ard Object Tracker data
int getWidth(String sData) {
  
  String sOutput = "";
  Serial.println("Searching in: " + sData + " for: " + "width...");
  int iObjStart = sData.indexOf(",");
  if (iObjStart>=0)
  {
    iObjStart = sData.indexOf(",", iObjStart+1);
    if (iObjStart>=0)
    {
      Serial.println("Start of data at: " + String(iObjStart));
      int iObjEnd = sData.indexOf(",", iObjStart+1);
      Serial.println("End of data at: " + String(iObjEnd));
      if (iObjEnd>iObjStart) {
        sOutput = sData.substring(iObjStart+1, iObjEnd);
      }
    }
  }

  Serial.println("Width=: " + sOutput);
  return sOutput.toInt();
  
}

// convert X position to robot turn angle
int getTurnAngle(int iXPos) {

  int iTurnAngle = 0;
  int iDX = 0;

  iDX = TARGET_XPOS - (iXPos);
  iTurnAngle = iDX / iTurnCalib;

  return -1*(iTurnAngle);

}

// convert object width to robot movement distance
int getDistance(int iWidth) {

  int iDistance = 0;

  if (iWidth <= iDistCallib[3][0]) {
    iDistance = iDistCallib[3][1];
  } else if ((iWidth > iDistCallib[3][0]) && (iWidth <= iDistCallib[2][0])) {
    iDistance = iDistCallib[3][1];
  } else if ((iWidth > iDistCallib[2][0]) && (iWidth <= iDistCallib[1][0])) {
    iDistance = iDistCallib[2][1];
  } else if ((iWidth > iDistCallib[1][0]) && (iWidth <= iDistCallib[0][0])) {
    iDistance = iDistCallib[1][1];
  } else if (iWidth >= iDistCallib[0][0]) {
    iDistance = iDistCallib[0][1];
  }

  return iDistance;
  
}

// create a move instruction for the Lego hub
String getLegoMvmtData(String sData, String sObj) {

  String sMvmtData = "";

  sMvmtData = "move=," + String(getTurnAngle(getXPos(sData, sObj))) + "," + String(getDistance(getWidth(sData)));

  return sMvmtData;
  
}

// debounced button press
bool buttonPressed() {

  bool bPressed = false;

  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        bPressed = true;
     }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  return bPressed;
  
}

// checks if two lines of object detection data are 'the same'
// uses a threshold test as bounding box data is not exactly consistent from frame to frame
bool isObjectDataMatching(String sObj1, String sObj2, String sObjToFind) {

  bool bResult = false;
  
  int iObj1XPos = getXPos(sObj1, sObjToFind);
  int iObj2XPos = getXPos(sObj2, sObjToFind);

  int iResult = iObj1XPos - iObj2XPos;
  int iAbsResult = abs(iResult);

  Serial.println("Difference=" + String(iAbsResult));

  if (iAbsResult <=10) {
    bResult = true;
  }

  return bResult;
  
}
