// src/modules/VehicleDetectModule.h
#pragma once
#include "mesh/ProtobufModule.h"
#include "concurrency/OSThread.h"
#include "mesh/Logging.h"
#include "meshtastic/VehicleDetect.pb.h"

class VehicleDetectModule : public ProtobufModule<VehicleDetect>,
                            private concurrency::OSThread {
 public:
  VehicleDetectModule();

 protected:
  virtual void setup() override;
  virtual int32_t runOnce() override;
  virtual bool handleReceivedProtobuf(
    const meshtastic_MeshPacket &mp,
    VehicleDetect *decoded
  ) override;

 private:
  void readMySensorData(VehicleDetect &msg);
};
