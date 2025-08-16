// src/modules/VehicleDetectModule.cpp
#include "modules/VehicleDetectModule.h"
#include "mesh/MeshService.h"
#include "configuration.h"
extern MeshService *service;

static constexpr uint8_t APP_PORT = meshtastic_PortNum_PRIVATE_APP;

VehicleDetectModule::VehicleDetectModule()
  : ProtobufModule("vehicledetect", APP_PORT, &VehicleDetect_msg),
    OSThread("VehicleDetect")
{}

void VehicleDetectModule::setup() {
  ProtobufModule<VehicleDetect>::setup();
  setIntervalFromNow(30*1000);
  LOG_INFO("VehicleDetect initialized");
}

int32_t VehicleDetectModule::runOnce() {
  VehicleDetect msg = VehicleDetect_init_zero;
  msg.time = millis()/1000;
  readMySensorData(msg);
  auto *p = allocDataProtobuf(msg);
  p->to = NODENUM_BROADCAST;
  service->sendToMesh(p);
  return 30*1000;  // schedule next in 30s
}

bool VehicleDetectModule::handleReceivedProtobuf(
  const meshtastic_MeshPacket &mp,
  VehicleDetect *decoded
) {
  // we’re pure-sender; just swallow
  return true;
}

void VehicleDetectModule::readMySensorData(VehicleDetect &msg) {
  // e.g. digitalRead or analogRead on your GPIO7 SW180 sensor:
  bool triggered = (digitalRead(GPIO_NUM_7) == LOW);
  if (triggered) {
    msg.magnetometer_x = 1;
  }
  // …etc for your real fields…
}
