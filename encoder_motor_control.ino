/***********************************************************/
/ Code to control encoder motors for a small scale robot. 
/***********************************************************/


/****************** Arduino Mega 2560 Direction/PWM Pins ******************/
// Right-Side (Motor A)
const int pwmPinRight = 5;     // ENA - Enable and PWM
const int rightReverse = 22; // IN1 - Reverse Drive
const int rightForward = 23; // IN2 - Forward Drive
const int encIntRight = 2; // Encoder A output, right motor

// Left-Side (Motor B)
const int pwmPinLeft = 6;      // ENB - Enable and PWM
const int leftForward = 24;  // IN3 - Forward Drive
const int leftReverse = 25;  // IN4 - Reverse Drive
const int encIntLeft = 3; // Encoder A output, left motor

const int encCountRev = 960; // Encoder single channel pulses per axle revolution

/****************** Other Variables/Constants *******************/
const int minSpeed = 0; // minimum motor speed
const int maxSpeed = 255; // maximum motor speed
const int encRevPulse = 960; // pulses per wheel revolution from single encoder phase
 
// Pulse count from encoders
volatile long encoderValueRight = 0;
volatile long encoderValueLeft = 0;
 
// One-second interval for measurements
int interval = 100;
 
// Counters for milliseconds during interval
long previousMillis = 0;
long currentMillis = 0;
 
// Variable for RPM measurement
float rpmRight = 0;
float rpmLeft = 0;
 
// Variables for PWM motor speed outputs
int motorPwmRight = 128;
int motorPwmLeft = 52;
 
void setup()
{
  // Setup Serial Monitor
  Serial.begin(57600); 
  
  // Set encoders as inputs with internal pullup  
  pinMode(encIntRight, INPUT_PULLUP); 
  pinMode(encIntLeft, INPUT_PULLUP); 
 
  // Set PWM and DIR connections as outputs
  pinMode(pwmPinRight, OUTPUT);
  pinMode(pwmPinLeft, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightReverse, OUTPUT);
  pinMode(leftForward, OUTPUT);
  pinMode(leftReverse, OUTPUT);
  
  // Attach interrupt 
  attachInterrupt(digitalPinToInterrupt(encIntRight), updateEncoderRight, RISING);
  attachInterrupt(digitalPinToInterrupt(encIntLeft), updateEncoderLeft, RISING);
  
  // Setup initial values for timer
  previousMillis = millis();
}
 
void loop()
{
    digitalWrite(rightForward, HIGH);
    digitalWrite(rightReverse, LOW);
    digitalWrite(leftForward, HIGH);
    digitalWrite(leftReverse, LOW);
    
    // Write PWM to controller
    analogWrite(pwmPinRight, motorPwmRight);
    analogWrite(pwmPinLeft, motorPwmLeft);
  
    // Update RPM value every 1/10 second
    currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;

      // Calculate RPM
      rpmRight = (encoderValueRight * 60.0 * 10.0 / encRevPulse);
      rpmLeft = (encoderValueLeft * 60.00 * 10.0 / encRevPulse);

      Serial.print("PWM VALUE RIGHT: ");
      Serial.print(motorPwmRight);
      Serial.print('\t');
      Serial.print(" PULSES RIGHT: ");
      Serial.print(encoderValueRight);
      Serial.print('\t');
      Serial.print(" SPEED RIGHT: ");
      Serial.print(rpmRight);
      Serial.print(" RPM");
      Serial.print('\t');      
      Serial.print("PWM VALUE LEFT: ");
      Serial.print(motorPwmLeft);
      Serial.print('\t');
      Serial.print(" PULSES LEFT: ");
      Serial.print(encoderValueLeft);
      Serial.print('\t');
      Serial.print(" SPEED LEFT: ");
      Serial.print(rpmLeft);
      Serial.println(" RPM");

      // Slowly adjusts PWM to the left motor to try to synchronize speed to the right motor
      if (encoderValueRight > encoderValueLeft && motorPwmLeft < maxSpeed) {
        motorPwmLeft++;
      }

      if (encoderValueRight < encoderValueLeft && motorPwmLeft > minSpeed) {
        motorPwmLeft--;
      }
    
      encoderValueRight = 0;
      encoderValueLeft = 0;
    }
}
 
void updateEncoderRight()
{
  // Increment value for each pulse from encoder
  encoderValueRight++;
}

void updateEncoderLeft()
{
  // Increment value for each pulse from encoder
  encoderValueLeft++;
}
