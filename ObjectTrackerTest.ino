String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  Serial1.begin(9600);
  Serial1.println("Ard Object Detection Test- Copyright: 2021");
}

void loop() {
  //
  // Handle Object Tracking:
  //
  if (stringComplete) {
    
    if (inputString.startsWith("FC:0:") || inputString.startsWith("FP:0:") || inputString.startsWith("TO:0:"))
    {
      Serial1.println(inputString);
    }

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent1() {
  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
