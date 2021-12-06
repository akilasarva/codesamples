#include "MPU9250.h"
#include "math.h"
MPU9250 IMU(Wire, 0x68);

int status;
// Low pass filter (LPF) frequency cutoff
const float fc_lp = 1;

// LPF RC value
const float RC_lp = 1/(2*PI*fc_lp);

// High pass filter (HPF) frequency cutoff
const float fc_hp = 0.5;

// HPF RC value
const float RC_hp = 1/(2*PI*fc_hp);

// Keeps track of dt
unsigned long prev_time = 0;

// Raw accel and gyro data
float acc_x = 0, acc_y = 0, acc_z = 0;
float gy_x = 0, gy_y = 0, gy_z = 0;

// Previous values from the gyro (used for the HPF)
float p_gy_x = 0, p_gy_y = 0, p_gy_z = 0;

// Filtered accel and gyro data
float f_acc_x = 0, f_acc_y = 0, f_acc_z = 0;
float f_gy_x = 0, f_gy_y = 0, f_gy_z = 0;

// Calculated gyro attitude data 
float pitch_gy = 0, roll_gy = 0, yaw_gy = 0;

// Complementry filter attitude
float pitch_cf = 0, roll_cf = 0;
float alpha = 0.05;

void setup() {
  
  Serial.begin(115200);
  while(!Serial){}
  
  // Initiates and calibrates IMU
  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
  IMU.setGyroRange(MPU9250::GYRO_RANGE_1000DPS);
}

void loop() {
  // Calculates time difference
  float dt = (micros() - prev_time) / 1000000.0;
  prev_time = micros();
  
  // Since the filter depends on dt, we have to recalculate the constants every loop
  // This can be avoided if we used a loop that allows for a constant update rate
  float lp_const = dt/(dt + RC_lp);
  float hp_const = RC_hp/(dt + RC_hp);
  
  // Reads the IMU and puts new values in their corresponding variables
  readSensorValues(&acc_x, &acc_y, &acc_z, &gy_x, &gy_y, &gy_z);
  
  // low pass
  f_acc_x = lp_const * acc_x + (1-lp_const) * f_acc_x;
  f_acc_y = lp_const * acc_y + (1-lp_const) * f_acc_y;
  f_acc_z = lp_const * acc_z + (1-lp_const) * f_acc_z;
  
  // high pass
  f_gy_x = hp_const * (f_gy_x + gy_x - p_gy_x);
  f_gy_y = hp_const * (f_gy_y + gy_y - p_gy_y);
  f_gy_z = hp_const * (f_gy_z + gy_z - p_gy_z);
  
  // gyro attitude calculation
  pitch_gy += rad2deg(gy_x * dt);
  roll_gy += rad2deg(gy_y * dt);
  yaw_gy += rad2deg(gy_z * dt);
  
  // Complementry filter
  pitch_cf = (1 - alpha) * (phi + gy_x * dt) + alpha * atan2f(acc_y, acc_z);
  roll_cf = (1 - alpha) * (theta + gy_y * dt) + alpha *  atan2f(-acc_x, sqrt(acc_z * acc_z + acc_y * acc_y));
  p_gy_x = gy_x;
  p_gy_y = gy_y;
  p_gy_z = gy_z;
  
  // x-axis acceleration vs filtered x-axis acceleration
  // print2ln(acc_x, f_acc_x);
  
  // x-axis rotation vs filtered x-axis rotation
  // print2ln(gy_x, f_gy_x);
  
  // Attitude calculation: 
  // Raw and filtered accelerometer-based pitch
  // print2ln(calcAngle(acc_y, acc_z, acc_x), calcAngle(f_acc_y, f_acc_z, f_acc_x));
  
  // Raw and filtered accelerometer-based roll
  // print2ln(calcAngle(acc_x, acc_z, acc_y), calcAngle(f_acc_x, f_acc_z, f_acc_y));
  
  // Raw gyroscope-based attitude
  // print3ln(pitch_gy, roll_gy, yaw_gy);
  
  // Complementary Filter 
  // print2ln(pitch_cf, roll_cf);
  
  delay(1);
}

void readSensorValues(float *ax, float* ay, float *az, float *gx, float *gy, float *gz) {
  IMU.readSensor();
    
  *ax = IMU.getAccelX_mss();
  *ay = IMU.getAccelY_mss();
  *az = IMU.getAccelZ_mss();
  *gx = IMU.getGyroX_rads();
  *gy = IMU.getGyroY_rads();
  *gz = IMU.getGyroZ_rads();
}

void printAccel() {
  print3ln(acc_x, acc_y,acc_z);
}

void printFilteredAccel() {
  print3ln(f_acc_x, f_acc_y,f_acc_z);
}

void printGyro() {
  print3ln(gy_x, gy_y, gy_z);
}

void printFilteredGyro() {
  print3ln(f_gy_x, f_gy_y, f_gy_z);
}

float rad2deg(float rad) {
  return rad / PI * 180;
}

void print2ln(float a, float b) {
  Serial.print(a, 6);
  Serial.print("\t");
  Serial.println(b, 6);
}

void print3ln(float a, float b, float c) {
  Serial.print(a, 6);
  Serial.print("\t");
  Serial.print(b, 6);
  Serial.print("\t");
  Serial.print(c, 6);
  Serial.println();
}
