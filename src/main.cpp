// Initially based on https://github.com/gizatt/braccio_driver/tree/main
#include <Arduino.h>
#include <BraccioV2.h>
#include <PacketSerial.h>

Braccio arm;
PacketSerial ps;

float qhome[6] = {90.0, 90.0, 90.0, 90.0, 90.0, 70.0};
float qh[6]; 
float q[6]; 
float k = 0.25;

double last_arm_update_t = 0.0;
const double update_dt = 0.001;

void onPacketReceived(const uint8_t* buffer, size_t size)
{
  int ndata = (int)(size/sizeof(float));
  if (ndata != 6) {
    Serial.println("Did not receive right amount of data");
    Serial.println(ndata);
    return;
  }

  float* data = (float*)buffer;
  for (int i = 0; i < 6; i++) {
    qh[i] = data[i];
  }
}

void home(){
  for (int i = 0; i < 6; i++){
    qh[i] = qhome[i];
  }
}

void setup() {
  // Home config
  for (int i = 0; i < 6; i++) {
    qh[i] = q[i] = qhome[i];
  }

  // Set up Serial comms (baud rate may change depending on board)
  int baudrate = 9600;
  Serial.begin(baudrate); 
  ps.begin(baudrate);
  ps.setPacketHandler(onPacketReceived);

  // Wait for serial
  while (!Serial){
    digitalWrite(LED_BUILTIN, false);
    delay(200);
    digitalWrite(LED_BUILTIN, true);
    delay(200);
  } 
  Serial.write("Braccio Driver connected and arm starting up.\n");
  
  // Arm default calibration and setup.
  arm.setJointCenter(WRIST_ROT, 90);
  arm.setJointCenter(WRIST, 90);
  arm.setJointCenter(ELBOW, 90);
  arm.setJointCenter(SHOULDER, 90);
  arm.setJointCenter(BASE_ROT, 90);
  arm.setJointCenter(GRIPPER, 70);//Rough center of gripper, default opening position
  arm.setJointMax(GRIPPER, 110);//Gripper closed, can go further, but risks damage to servos
  arm.setJointMin(GRIPPER, 50);//Gripper open, can't open further

  // Start arm to this default position.
  arm.begin(true);
  Serial.write("Arm started.\n");
}

void loop() {
  ps.update();

  double t = ((double)micros()) * 1e-6;
  if (t - last_arm_update_t > update_dt){
    for (int i = 0; i < 6; i++) {
      q[i] += (qh[i] - q[i])*k;
    }
    arm.setAllNow(q[0], q[1], q[2], q[3], q[4], q[5]);
    arm.update();
    last_arm_update_t = t;
  }
}