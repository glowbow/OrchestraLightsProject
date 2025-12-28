//most recent ID number/value of a slider/key gets displayed on screen and sent to Serial Monitor in Arduino IDE
import processing.serial.*;
import controlP5.*;
import themidibus.*;

MidiBus myBus;
MidiMessageHandler msgh;

int number;
int value;

//boolean isNew;

Serial myport;
ControlP5 cp5;
Slider brightness;

void setup() {
  MidiBus.list();
  msgh = new MidiMessageHandler();
  myBus = new MidiBus(msgh, 1, -1);
  println("opened AKM320"); //name of MIDI keyboard
  
  printArray(Serial.list());
  size(700, 350);//window size

  myport = new Serial(this, "COM4", 115200);
  delay(1000);
  println("Ready");

  cp5 = new ControlP5(this);
  
}


void draw() {
  background(50);
  
  fill(255);
  textAlign(CENTER);
  textSize(16);
   text("Number: " + number + ", Value: " + value, width / 2, height / 2); //laptop display
  
  if (msgh.isNew) {
    number = msgh.lastNumber;
    value = msgh.lastValue;
    msgh.isNew = false;
    
      sendToSerial(number, value); //sending to arduinoIDE
    }
    //isNew = true;
  }
  
  
 
void sendToSerial(int number, int value) {
  String message = number + "," + value + "\n"; //only require key ID and the value, since the receiver sorts out the key type (color/effect/speed/brightness) and uses value for only some effects
  myport.write(message);
  println("Sent: " + message);
  
}

void exit() {
  println("exiting");
  myBus.close();
}
