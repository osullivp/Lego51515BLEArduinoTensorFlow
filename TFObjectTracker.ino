#include <SoftwareSerial.h>

SoftwareSerial mySerial(14, 15);  // RX, TX
String sInputString = "";          // a String to hold incoming data
bool bStringComplete = false;      // whether the string is complete
bool bGetPosition = false;      // has an object detection request been made

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

  Serial.println("Ard Object Detection Test- Copyright: 2021");
  
}

void loop() {

  // Get object detection data if a request has been made from the Lego Spike Prime/51515 hub
  if (bGetPosition) {
    while (Serial1.available()>0) {
      // get the new byte:
      char cInChar = (char)Serial1.read();
      // add it to the sInputString:
      sInputString += cInChar;
      // if the incoming character is a newline, set a flag so the main loop can
      // do something about it:
      if (cInChar == '\n') {
        bStringComplete = true;
        bGetPosition = false;
      }
    }
  // otherwise, listen for a request
  } else if (!bGetPosition){
    if (mySerial.available()>0) {
      Serial.println("HM-10 Available");
      int cInChar = (int)mySerial.read();
      Serial.println(cInChar);
      if (cInChar == 1) {
        bGetPosition = true;
        Serial.println("Read Requested");
      }
    }
  }

  // once a full line of object detection data has been received, send to the Lego hub
  if (bStringComplete) {
    //mySerial.println(sInputString);
    mySerial.write('1');
    Serial.println(sInputString);
    bStringComplete = false;
  }
  
}
