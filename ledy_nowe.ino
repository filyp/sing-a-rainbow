#include <arduinoFFT.h>

#include <Adafruit_NeoPixel.h>

#include <Adafruit_NeoPixel.h>

 
#define SAMPLES 64             //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be less than 10000 due to ADC

#define NUMPIXELS      60

#define LEDS_PIN 5
#define BUTTON_PIN 3

#define MICROPHONE A0
#define JACK A1
#define R_POTENTIOMETER A3
#define G_POTENTIOMETER A4
#define B_POTENTIOMETER A5

 
arduinoFFT FFT = arduinoFFT();



 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];
int mic_level, jack_level;
int prev_button_state;
int mode;



Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDS_PIN, NEO_GRB + NEO_KHZ800);

uint32_t Rainbow(unsigned int i) {
  if (i > 1529+512) return Rainbow(i % 1530);
  if (i > 1274+512) return pixels.Color(255, 0, 255 - (i % 255));   //violet -> red
  if (i > 1019+512) return pixels.Color((i % 255), 0, 255);         //blue -> violet
  if (i > 764+512) return pixels.Color(0, 255 - (i % 255), 255);    //aqua -> blue
  if (i > 509+512) return pixels.Color(0, 255, (i % 255));          //green -> aqua
  if (i > 255+512) return pixels.Color(255 - (i % 255), 255, 0);    //yellow -> green
  return pixels.Color(255, i/3, 0);                               //red -> yellow
}

uint8_t split(uint32_t color, uint8_t i ) {

  //0 = Red, 1 = Green, 2 = Blue

  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}






void setup() {
    pinMode(LEDS_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    
    pixels.begin();
    
    Serial.begin(115200);
 
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

    prev_button_state = LOW;
    mode = 2;
}



void loop() {

    if(digitalRead(BUTTON_PIN) == HIGH && prev_button_state == LOW){
        // someone just pressed a button
        mode = (mode+1) % 3;
        prev_button_state = HIGH;
    }
    if(digitalRead(BUTTON_PIN) == LOW && prev_button_state == HIGH){
        prev_button_state = LOW;
    }
//        mic_level = analogRead(MICROPHONE);
//        jack_level = analogRead(JACK);
//        Serial.println(jack_level);


    switch(mode){

        case 0:
            // microphone fft mode
            analogReference(INTERNAL);      //its the lowest possible
            /*SAMPLING*/
            for(int i=0; i<SAMPLES; i++)
            {
                microseconds = micros();    //Overflows after around 70 minutes!
             
                vReal[i] = analogRead(MICROPHONE);
                vImag[i] = 0;
             
                while(micros() < (microseconds + sampling_period_us)){
                }
            }
         
            /*FFT*/
            FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
            FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
            //double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
            //Serial.println(peak);     //Print out what frequency is the most dominant.
        
            //for(int i=0; i<(SAMPLES/2); i++)
            for(int i=0; i<NUMPIXELS; i++)
            {
                  uint32_t col = Rainbow(1050 * (NUMPIXELS - i) / NUMPIXELS);
                  int bin_num = i/3 + 3;  // divide to scale up the spectrum, add to cut out bass
                  float amplitude = min(500, vReal[bin_num]) / 500;
                  amplitude *= amplitude;
                  pixels.setPixelColor(i, split(col, 0) * amplitude,
                                    split(col, 1) * amplitude,
                                    split(col, 2) * amplitude);
                
//                Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
//                Serial.print(" ");
//                Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
            }
            
            pixels.show();
            break;

        case 1:
            // jack fft mode
            analogReference(INTERNAL);      //its the lowest possible
            /*SAMPLING*/
            for(int i=0; i<SAMPLES; i++)
            {
                microseconds = micros();    //Overflows after around 70 minutes!
             
                vReal[i] = analogRead(JACK);
                vImag[i] = 0;
             
                while(micros() < (microseconds + sampling_period_us)){
                }
            }
         
            /*FFT*/
            FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
            FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
            //double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
            //Serial.println(peak);     //Print out what frequency is the most dominant.
        
            //for(int i=0; i<(SAMPLES/2); i++)
            for(int i=0; i<NUMPIXELS; i++)
            {
                  uint32_t col = Rainbow(1050 * (NUMPIXELS - i) / NUMPIXELS);
                  float amplitude = min(500, vReal[i/3 + 3]) / 500;
                  amplitude *= amplitude;
                  pixels.setPixelColor(i, split(col, 0) * amplitude,
                                    split(col, 1) * amplitude,
                                    split(col, 2) * amplitude);
                
//                Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
//                Serial.print(" ");
//                Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
            }
            
            pixels.show();
            break;

        case 2:
            // rgb potentiometers mode
            analogReference(DEFAULT);      //5V reference for potentiometers
            
            long int red = 0;
            long int green = 0;
            long int blue = 0;
            int iter = 300;
            for(int i = 0; i < iter; i++){
              red += analogRead(R_POTENTIOMETER);
              green += analogRead(G_POTENTIOMETER);
              blue += analogRead(B_POTENTIOMETER);
            }
            red = red * 255 / 1023 / iter;
            green = green * 255 / 1023 / iter;
            blue = blue * 255 / 1023 / iter;
            
            for(int i=NUMPIXELS-1; i>=1; i--){
                pixels.setPixelColor(i, pixels.getPixelColor(i-1));
            }
            pixels.setPixelColor(0, red, green, blue);
            pixels.show();
            break;
    }
}
