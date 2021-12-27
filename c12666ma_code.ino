 
// This code is a modified from the original sketch from Peter Jansen
// https://github.com/tricorderproject/arducordermini
// and Kurt Kiefer
// https://github.com/groupgets/c12666ma
// This code can be used with an internal or external ADC. 

#include <SerialCommand.h> 

// Spectrometer pins for controlling C12666MA Micro-spectrometer Hamamatsu 
//#define SPEC_GAIN        A0
//#define SPEC_EOS         NA
//#define SPEC_VIDEO       A4
//#define LASER_404        A5
//#define WHITE_LED        4

#define SPEC_ST          6
#define SPEC_CLK         5

// ADS8321 external 16-bit ADC pseudo-SPI connection
#define CLK              9     // Clock 
#define DOUT            10       // MISO 
#define CS               7      // Selection Pin


SerialCommand sCmd;     // SerialCommand object

#include <elapsedMillis.h>  // Integration time calculation
#include <digitalWriteFast.h>  /// look up github and add ZIP file to arduino

#define SPEC_CHANNELS    256
uint16_t data[SPEC_CHANNELS];

int intTime = 100;                   // variable to store the length of integration time (microseconds)
int delay_time = 1;               // delay per half clock (in microseconds).  This ultimately conrols the integration time.

void setup() {

// Set desired pins to OUTPUT for spectrometer
  //pinMode(SPEC_EOS, INPUT);
  //pinModeFast(SPEC_GAIN, OUTPUT);
  //pinModeFast(WHITE_LED, OUTPUT);
  //pinModeFast(LASER_404, OUTPUT);
  pinModeFast(SPEC_ST, OUTPUT);
  pinModeFast(SPEC_CLK, OUTPUT);

  //digitalWriteFast(WHITE_LED, LOW);
  //digitalWriteFast(LASER_404, LOW);
  

  digitalWriteFast(SPEC_ST, HIGH);
  digitalWriteFast(SPEC_CLK, HIGH);
  //digitalWrite(SPEC_GAIN, HIGH); //LOW Gain (want this)
  //digitalWrite(SPEC_GAIN, LOW); //High Gain


// Set desired pints on ADC
  pinMode(CS, OUTPUT); 
  pinMode(DOUT, INPUT); 
  pinMode(CLK, OUTPUT); 
    
//disable device to start with 
  digitalWrite(CS,HIGH); 
  digitalWrite(CLK,LOW); 
  
  //Serial.begin(9600);
  Serial.begin(115200);

// Setup callbacks for SerialCommand commands
  sCmd.addCommand("set_integ",  integ_time);  
  sCmd.addCommand("read", read_value);   /// read the data
  
}

int read_adc(){
  uint16_t adcvalue = 0;
  digitalWriteFast(CS, HIGH);
  
  digitalWriteFast(CS,LOW);       
  
  for(int i = 0; i<=6; i++){
    digitalWriteFast(CLK,HIGH); 
    delayMicroseconds(1);     
    digitalWriteFast(CLK,LOW);
    delayMicroseconds(1);  
  }

  //read bits from adc
  for (int i=15; i>=0; i--){
    //cycle clock
    
    digitalWriteFast(CLK,HIGH);
    adcvalue |= digitalRead(DOUT)<<i; 
    delayMicroseconds(1);   
    digitalWriteFast(CLK,LOW);
    delayMicroseconds(1); 

    //Serial.print(digitalRead(DOUT));
  }
  digitalWriteFast(CS,HIGH);
  //Serial.println("  ");

  //          //turn off device
  //Serial.print(digitalRead(DOUT));
  return adcvalue;
}

void integ_time() {  
  long aNumber;
  char *arg;

  arg = sCmd.next();
  if (arg != NULL) {
    aNumber = atol(arg);         // Converts a char string to an integer
    intTime = aNumber;           // in microseconds

    delay_time = intTime / 4;           // calculate delays (in microseconds) for all integration timing
  }

  Serial.println(intTime);

}

void read_value() {
  
    // Get timing for clock pulses that will be incorporated into integration time 
elapsedMillis conversion_rate = 0;

  int read_time = 156;           // Amount of time that the analogRead() procedure takes to read one pixel (in microseconds) (different micros will have different times)  
  int accumulateMode = false;    
  int i;

// Step 1: start leading clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  } 
  

// Step 2: Send start pulse to signal start of integration/light collection
  digitalWriteFast(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, HIGH);
  digitalWriteFast(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, HIGH);
  digitalWriteFast(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);


// Step 3: Read Data 2 (this is the actual read, since the spectrometer has now sampled data)
  int idx = 0;
  for (int i = 0; i < SPEC_CHANNELS; i++) {    
    // Four clocks per pixel
    // First block of 2 clocks -- measurement
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, LOW);
    
 /* 
  // Analog value is valid on low transition (internal ADC)
    if (accumulateMode == false) {
      data[idx] = analogRead(SPEC_VIDEO);
    } else {
      data[idx] += analogRead(SPEC_VIDEO);
    }
    idx += 1;
    if (delay_time > read_time) delayMicroseconds(delay_time - read_time);   // Read takes about 135uSec
 */
  
  // Accumulate ADC reading data (external ADC)
    uint16_t readvalue = read_adc();
    data[i] = readvalue;

    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);

  // Second block of 2 clocks -- idle
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

// Step 4: trailing clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWriteFast(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWriteFast(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

// Step 5: Send start pulse to signal end of integration/light collection
  digitalWriteFast(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, HIGH);
  digitalWriteFast(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWriteFast(SPEC_CLK, HIGH);
  digitalWriteFast(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);

int timer_usec = conversion_rate;

// Step 6: Print data

  //Serial.write((unsigned char*)data, SPEC_CHANNELS*2);
  //Serial.println(timer_usec);   // time for all clock pulses
}


void print_data()                  // prints the output to csv output to the terminal.
{
  for (int i = 0; i < SPEC_CHANNELS; i++) 
  {
    Serial.print(data[i]);
    Serial.print(',');
  }
  Serial.print("\n");
}

void loop() 
{
//  digitalWrite(LASER_404, HIGH);
//  read_value();
//  digitalWrite(LASER_404, LOW);
//  print_data();
//  delay(10);
  
//  digitalWrite(WHITE_LED, HIGH);
//  read_value();
//  digitalWrite(WHITE_LED, LOW);
//  print_data();
//  delay(10);

//  read_value();
//  print_data();
  sCmd.readSerial();      // fill the buffer
  
  delayMicroseconds(10);   
}
