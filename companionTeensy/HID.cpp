#include "HID.h"
bool HIDDumpController::changed_data_only = false;
extern void buttonPressed(const void *ptr, uint32_t len);


void HIDDumpController::init(){
  USBHost::contribute_Transfers(mytransfers, sizeof(mytransfers) / sizeof(Transfer_t));
  USBHIDParser::driver_ready_for_hid_collection(this);
}



hidclaim_t HIDDumpController::claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage){
  if (mydevice != NULL && dev != mydevice) {
    return CLAIM_NO;
  }
  if (usage_ && (usage_ != topusage)) {
    return CLAIM_NO;
  }
  
  mydevice = dev;
  collections_claimed++;
  usage_ = topusage;
  driver_ = driver;
  driver_->setTXBuffers(hid_tx_buffer1, hid_tx_buffer2, 8192);
  
  return CLAIM_INTERFACE;
}



void HIDDumpController::disconnect_collection(Device_t *dev){
  if (--collections_claimed == 0) {
    mydevice = NULL;
    usage_ = 0;
  }
}



bool HIDDumpController::hid_process_in_data(const Transfer_t *transfer){
  hid_input_begin_level_ = 0;
  count_usages_ = index_usages_;
  index_usages_ = 0;
  buttonPressed(transfer->buffer, transfer->length);
  return true;
}



bool HIDDumpController::hid_process_out_data(const Transfer_t *transfer){
  return true;
}

void HIDDumpController::hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax){
  //
}

void HIDDumpController::hid_input_data(uint32_t usage, int32_t value){
  //
}

void HIDDumpController::hid_input_end(){
  //
}
