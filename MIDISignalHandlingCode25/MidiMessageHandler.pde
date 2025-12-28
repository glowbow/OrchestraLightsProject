//determines if a key/slider on keyboard has changed by updating and storing the most recent key ID num/val
public class MidiMessageHandler {
  int lastNumber;
  int lastValue;
  boolean isNew = false;

  public MidiMessageHandler() {
  }

  public void noteOn(int channel, int noteNum, int velocity) {
    if (velocity>0) {
      lastNumber = noteNum;
      lastValue = 127; //value for notes is always 127 or 0
      isNew = true;
      
    System.out.println("Note On: channel=" + channel + ", number=" + noteNum + ", value=" + velocity);
    }
  }
/*
  public void noteOff(int channel, int noteNum, int velocity) {
    //System.out.println("Note Off: channel=" + channel + ", pitch=" + pitch + ", velocity=" + velocity);
  }

*/
 
  public void controllerChange(int channel, int number, int value) {
    System.out.println("Controller Change: channel=" + channel + ", number=" + number + ", value=" + value);

    lastNumber = number;
    lastValue = value; //value for sliders can range 0 to 127
    isNew = true;
  }
 
}
