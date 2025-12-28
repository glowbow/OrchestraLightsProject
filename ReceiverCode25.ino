#include <ESP8266WiFi.h>
#include <espnow.h>
#include "FastLED.h"

#undef MIN
#define BROADCAST_ADDRESS_5 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define NUM_LEDS 27 //change for the longer podium lights
#define DATA_PIN 2
#define GROUP_ID 1 //assign different for lights in different groups

CRGB leds[NUM_LEDS];
CRGB currentColor = CRGB::Black;

const CRGB colors[] = {CRGB(255, 20, 147), //pink
                       CRGB(0, 0, 255), //blue
                       CRGB(0, 255, 0)}; //green

unsigned long lastEffectUpdate = 0; //for coldplay and fire and other preset
unsigned long lastFlashTime = 0; //for synced
int fadeTime = 1000; //1 second fade
int bright = 100;
unsigned long effectDelay = 666;


int currentEffect = 0;

typedef struct {
  int groupId;
  int number;
  int value;
  uint32_t messageId;
} MidiData;

MidiData receivedData;

//special effects
void coldplayEffect();
void pokemon();
void flashEffect();
void TwinklePixels();
void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue);
void fadeEffect();
void stopEffects();
void setColor(CRGB color);
void wavyEffect();

bool isFlashing = false; //for the waves
unsigned long flashStartTime = 0;

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    static uint32_t lastMessageId[5] = {0}; 

    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.printf("Received: Number=%d, Group=%d, MessageId=%d\n", receivedData.number, receivedData.groupId, receivedData.messageId);

    
    if (receivedData.groupId != GROUP_ID && receivedData.groupId != 0) { //ignore messages not matching ID
        Serial.printf("ignored. Received Group=%d, Expected Group=%d\n", receivedData.groupId, GROUP_ID);
        return;
    }

    //no dups
    if (receivedData.messageId <= lastMessageId[receivedData.groupId]) {
        Serial.printf("Duplicate or older message ignored for Group=%d\n", receivedData.groupId);
        return;
    }

    //update message
    lastMessageId[receivedData.groupId] = receivedData.messageId;
//protect against bright/speed changes
    if (receivedData.number != 1 && receivedData.number != 7) { //1 = brightness, 7 = speed controls
      if (currentEffect == 2 || currentEffect == 3 || currentEffect == 4 || currentEffect == 5) {
        currentEffect = 0;
      }
    }
    

   
    if (receivedData.number == 78 || receivedData.number == 82) { //for the groups in waves effect
        if (receivedData.groupId == GROUP_ID) {
            Serial.println("Flash effect triggered!");
            //setColor(CRGB(0, 0, 0));
            flashOnEffect();  
        }
    } else if (receivedData.number == 80) {
      oneFlash();
      delay(300);
      oneFlash();
    } else {
        Serial.println("Setting color or effect"); //effects meant for all lights at once
        switch (receivedData.number) {

//brightness of all except fading ones
            case 1: 
              bright = receivedData.value;
              FastLED.setBrightness(bright);
              FastLED.show();
              break;
            
//speed of all except fading ones
            case 7:
              if (receivedData.value >= 0 && receivedData.value <= 127) {
                effectDelay = map(receivedData.value, 0, 127, 800, 80); //map midivalue to delay range
              }
          break; 
//colors
            case 53: setColor(CRGB(220, 20, 20)); break;
            case 55: setColor(CRGB(225, 0, 0)); break;
            case 57: setColor(CRGB(225, 50, 0)); break;
            case 59: setColor(CRGB(225, 102, 0)); break;
            case 60: setColor(CRGB(225, 150, 0)); break;
            case 62: setColor(CRGB(124, 252, 0)); break; //lime
            case 64: setColor(CRGB(34, 170, 0)); break; //darker green
            case 65: setColor(CRGB(20, 255, 100)); break; //teal
            case 67: setColor(CRGB(0, 191, 255)); break;
            case 69: setColor(CRGB(20, 50, 200)); break;
            case 71: setColor(CRGB(0, 0, 255)); break;
            case 72: setColor(CRGB(50, 20, 255)); break;
            case 74: setColor(CRGB(225, 0, 255)); break;
            case 76: setColor(CRGB(225, 100, 255)); break;
            case 84: setColor(CRGB(0, 0, 0)); break;
//54, 56, 58, 61, 63, 66, 68, 70, 73, 75 = color-changeable patterns

            case 54: stopEffects(); break; //stops
            case 56:
              currentEffect = 1; //flash 
              break;
            case 58:
              currentEffect = 6; //fade
              break;
            case 61:
              currentEffect = 7; //color comet left->right
              break;
            case 63:
              currentEffect = 8; //color comet right->left
              break;
            case 66:
              currentEffect = 9; //alternating
              break;
            case 68:
              currentEffect = 10; //alternating chase
              break;
            case 70:
              currentEffect = 11; //wave
              break;
            case 73:
              currentEffect = 12; //two crossing
              break;
            case 75:
              currentEffect = 13; //heartbeat
              break;
//77, 79, 81, 83 = preset patterns 
            case 77: //coldplay
              currentEffect = 2;
              break;
            case 79: //fire
              currentEffect = 3;
              break;
            case 81: //twinkle
              currentEffect = 4;
            break;
            case 83:
              currentEffect = 5; //rainbow
            break;


            default: Serial.println("Unknown number");
        }
    }
}



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  
  esp_now_register_recv_cb(OnDataRecv);

  uint8_t broadcastAddr5[6] = BROADCAST_ADDRESS_5; //same address for all lights
  if (esp_now_add_peer(broadcastAddr5, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0) {
    Serial.println("Failed to add broadcast peer 5");
    return;
  }

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS); //addressable LED type
  FastLED.setBrightness(bright); //changes based on key ID 1's value
  fill_solid(leds, NUM_LEDS, CRGB::Red); //initialization flash
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
 
  if (isFlashing && (millis() - flashStartTime >= effectDelay)) {
    isFlashing = false;  //end
    fill_solid(leds, NUM_LEDS, CRGB::Black);  
    FastLED.show();
  }


  switch(currentEffect) {
    case 1: wavyEffect(); break;
    case 2: coldplayEffect(); break;
    case 3: pokemon(); break;
    case 4: TwinklePixels(170, 50, 20, 50, 100); break;
    case 5: rainbow_wave(100,10); break;
    case 6: fadeEffect();
    case 7: fireEffect(currentColor.r, currentColor.g, currentColor.b, random(10, 50), 40, 1400, 1); break;
    case 8: fireEffect(currentColor.r, currentColor.g, currentColor.b, random(10, 50), 40, 1400, -1); break; //swap direction
    case 9: oddEven(); break;
    case 10: chaseEffect(100); break;
    case 11: bloomEffect(); break;
    case 12: crossoverEffect(); break;
    case 13: heart(); break; //LOL heartbreak
    default: break;
  }
}

void oneFlash() {
    unsigned long flashStart = millis();  
    fill_solid(leds, NUM_LEDS, currentColor);
    FastLED.show();

    while (millis() - flashStart < 300) {
        
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}


void wavyEffect() {
    static unsigned long lastUpdate = 0;
    static float wavePhase = 0;  

    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= 40) {
        lastUpdate = currentMillis;
        wavePhase += 0.05; //change for speed

        for (int i = 0; i < NUM_LEDS; i++) {
            
            float wave = (sin(i * 0.6) * cos(wavePhase)) * 0.5 + 0.5; //wavy function between 0-1, 0.6 wave width

            
            int brightnessScale = map(wave * 100, 0, 100, 0, 100);  

            //no brightness jump
            if (brightnessScale < 5) {
                leds[i] = CRGB::Black;
            } else {
                leds[i] = currentColor;  
                leds[i].nscale8(brightnessScale);  //scaling
            }
        }

        FastLED.show();
    }
}






void oddEven() {
    static bool showOdds = true; 
    unsigned long currentMillis = millis();

    if (currentMillis - lastEffectUpdate >= effectDelay) {
        lastEffectUpdate = currentMillis;
        showOdds = !showOdds; 

        for (int i = 0; i < NUM_LEDS; i++) { //alternating which LEDs are lit
            if ((i % 2 == 0) == showOdds) {  
                leds[i] = currentColor;  
            } else {
                leds[i] = CRGB::Black;
            }
        }

        FastLED.show();
    }
}


void setColor(CRGB color) {

  if (currentEffect == 2 || currentEffect == 3 || currentEffect == 4 || currentEffect == 5) { //these are the preset and dont need color assigned
    currentEffect = 0;
  }

  currentColor = color; 
  
  if (currentEffect != 6 && currentEffect < 9) { //cant be
    for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }

  FastLED.show();
  }
  
}

void fireEffect(int red, int green, int blue, int tail_length, int delay_duration, int interval, int direction) {
  static int count = 0;
  unsigned long currentMillis = millis();   

if (currentEffect == 6) { //stop fade effect from overlapping
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        return;  
    }

  if (currentMillis - lastEffectUpdate >= interval) {
    lastEffectUpdate = currentMillis;         
    count = 0;                           
  }
  if (direction == -1) {     
    if (count < NUM_LEDS) {
      leds[NUM_LEDS - (count % (NUM_LEDS+1))].setRGB(red, green, blue);  
      count++;
    }
  }
  else {
    if (count < NUM_LEDS) {    
      leds[count % (NUM_LEDS+1)].setRGB(red, green, blue);  
      count++;
    }
  }
  fadeToBlackBy(leds, NUM_LEDS, tail_length);               
  FastLED.show();
  delay(delay_duration);  
}

void pokemon() {
    static unsigned long lastUpdate = 0;
    static float offset = 0;

    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= 100) { 
        lastUpdate = currentMillis;
        offset += 0.5;  //adjust for speed

        for (int i = 0; i < NUM_LEDS; i++) {
            float wave = (sin((i * 0.5)+ offset) + 1) / 2;

            //blend between red and blue using the wave function
            leds[i] = blend(CRGB(225, 100, 255), CRGB(20, 255, 100), wave * 255);
        }
        FastLED.show();
    }
}


void coldplayEffect() {
    static int colorIndex = 0;  
    static unsigned long lastChange = 0;
    unsigned long now = millis();
    
    if (now - lastChange >= fadeTime) { 
        lastChange = now;
        colorIndex = random(3); //randomly switch between r g and b, can be same multiple times
    }

    float brightnessFactor = (sin((millis() % fadeTime) * (PI / fadeTime)) + 1) / 2.0; //trying wavy fade for brightness

    CRGB fadedColor = CRGB(
        colors[colorIndex].r * brightnessFactor,
        colors[colorIndex].g * brightnessFactor,
        colors[colorIndex].b * brightnessFactor
    );

    fill_solid(leds, NUM_LEDS, fadedColor);
    FastLED.show();
}

void TwinklePixels(int Color, int ColorSaturation, int PixelVolume, int FadeAmount, int DelayDuration) { //random sparkles
  for (int i = 0; i < NUM_LEDS; i++) {
  
    if (random(PixelVolume) < 2) {    
      uint8_t intensity = random(100, 255);     
      CRGB set_color = CHSV(Color, ColorSaturation, intensity);
      leds[i] = set_color;    
    }

   
    if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0) {    
      leds[i].fadeToBlackBy(FadeAmount); //fading each out until gone
    }
  }

  FastLED.show();
  delay(DelayDuration);
}

void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) { //can change speed/amount of color change    
  static uint8_t thisHue = 0; 
  static unsigned long lastUpdate = 0;  

  if (millis() - lastUpdate > thisSpeed) { 
    lastUpdate = millis();
    thisHue += deltaHue;  
    
    fill_rainbow(leds, NUM_LEDS, thisHue, deltaHue);  
    FastLED.show();  
  }
}

void chaseEffect(int speed) {
        static int position = 0;  
    unsigned long currentMillis = millis();

   //interval update
    if (currentMillis - lastEffectUpdate >= speed) {
        lastEffectUpdate = currentMillis;
        position++;  

       
        if (position >= 3) {  
            position = 0;
        }

       
        for (int i = 0; i < NUM_LEDS; i++) {
            if ((i % 3) == position) {
                leds[i] = currentColor; //lights go down the line
            } else {
                leds[i] = CRGB::Black;  
            }
        }

        FastLED.show();
    }
}

void bloomEffect() {
    static int step = 0;  
    static int direction = 1; 
    static unsigned long startMillis = millis();

    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - startMillis;
    
   
    if (elapsedTime >= effectDelay * 3) { //adjusted for effect 
        startMillis = currentMillis;
        step = 0;  
        direction = 1; //outward
    }

   
    float progress = (float)elapsedTime / (effectDelay * 3); //0 to 1 start to end of cycle
    step = (NUM_LEDS / 2) * (progress < 0.5 ? progress * 2 : (1 - (progress - 0.5) * 2)); //first 0 to 0.5 goes out then 0.5 to 1 goes in

    fill_solid(leds, NUM_LEDS, CRGB::Black); 


    for (int i = 0; i <= step; i++) {
        int leftIndex = (NUM_LEDS / 2) - i; //symmetrically expand/shrink
        int rightIndex = (NUM_LEDS / 2) + i;
        
        if (leftIndex >= 0) leds[leftIndex] = currentColor;
        if (rightIndex < NUM_LEDS) leds[rightIndex] = currentColor;
    }

    FastLED.show();
}

void heart() {
    static int brightness = 0;
    static int fadeDirection = 1;
    static int beatStep = 0;
    static unsigned long lastUpdate = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate > 15) {
        lastUpdate = currentMillis;

       
        brightness += fadeDirection * 10;
        brightness = constrain(brightness, 0, 230); //not TOO bright

       
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = currentColor;
            leds[i].fadeLightBy(255 - brightness); //fade out
        }
        FastLED.show();

        //rhythm
        if (brightness == 230) {
            fadeDirection = -1; //fade out
        }
        if (brightness == 0 && fadeDirection == -1) {
            fadeDirection = 1; //switch to fade in
            beatStep++;

            if (beatStep == 2) {
                delay(400); //mini pause
                beatStep = 0; //repeat cycle
            }
        }
    }
}


void crossoverEffect() {
    static unsigned long startMillis = millis();
    static int step = 0;  
    static int direction = 1; //1 = right and vice versa
    
    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - startMillis;
    
   //adjust speed
    if (elapsedTime >= effectDelay * 3) {
        startMillis = currentMillis; 
        step = 0;
        direction *= -1; //other way to bounce back
    }

  
    float progress = (float)elapsedTime / (effectDelay * 3);
    int maxSteps = NUM_LEDS / 2; //half strip distance
    step = maxSteps * (progress < 0.5 ? progress * 2 : (1 - (progress - 0.5) * 2)); //in the same way as the bloom effect, first 0 to 0.5 goes one way then 0.5 to 1 goes the other

    fill_solid(leds, NUM_LEDS, CRGB::Black); 

    
    for (int i = 0; i < 3; i++) { //symmetrical, 3 LEDs thick, can change later
        int leftPos = i + step;   
        int rightPos = (NUM_LEDS - 1) - (i + step);  

        if (leftPos < NUM_LEDS) leds[leftPos] = currentColor;
        if (rightPos >= 0) leds[rightPos] = currentColor;
    }

    FastLED.show();
}

void fadeEffect() {
    static int brightness = 0;
    static int fadeDirection = 1;
    static unsigned long lastUpdate = 0;
    static unsigned long nextFadeStart = 0;
    static bool waitingForNextFade = false;

    unsigned long currentMillis = millis();

    if (waitingForNextFade) {
        if (currentMillis >= nextFadeStart) {
            waitingForNextFade = false; //restart the cycle
            fadeDirection = 1;
        }
        return; 
    }

    if (currentMillis - lastUpdate > 15) { //was 15
        lastUpdate = currentMillis;

        brightness += fadeDirection * 6; //step by 6
        brightness = constrain(brightness, 0, 255); //limiting the brightness

        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = currentColor;
            leds[i].nscale8_video(brightness); //scaling intensity and trying to prevent glitchiness
        }
        FastLED.show();

        if (brightness == 255) {
            fadeDirection = -1;
        } 
        
        if (brightness == 0 && fadeDirection == -1) {
            fadeDirection = 0;
            waitingForNextFade = true;
            nextFadeStart = currentMillis + random(500, 3500); //random delay, looks neat when big crowd of lights randomly fade in/out
        }
    }
}



void stopEffects() {
    currentEffect = 0;
    //fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}




void flashOnEffect() {
  isFlashing = true;  

  Serial.println("Starting flash");

  fill_solid(leds, NUM_LEDS, CRGB::Black);  
  FastLED.show();
  delay(200);

  fill_solid(leds, NUM_LEDS, currentColor); 
  FastLED.show();
  flashStartTime = millis(); 


  delay(effectDelay);  

  fill_solid(leds, NUM_LEDS, CRGB::Black); 
  FastLED.show();

  Serial.println("End flash");
}