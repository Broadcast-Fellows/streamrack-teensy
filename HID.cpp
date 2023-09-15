#include "HID.h"
bool HIDDumpController::changed_data_only = false;
extern void keyPressedEvent(const void *ptr, uint32_t len);


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
  // driver_->setTXBuffers(hid_tx_buffer1, hid_tx_buffer2, (uint8_t)8192);
  driver_->setTXBuffers(hid_tx_buffer1, hid_tx_buffer2, 0);
  
  return CLAIM_INTERFACE;
}



void HIDDumpController::disconnect_collection(Device_t *dev){
  if (--collections_claimed == 0) {
    mydevice = NULL;
    usage_ = 0;
  }
}

void dump_hexbytes(const void *ptr, uint32_t len)
{
  if (ptr == NULL || len == 0) return;
  uint32_t count = 0;
//  if (len > 64) len = 64; // don't go off deep end...
  const uint8_t *p = (const uint8_t *)ptr;
  while (len--) {
    if (*p < 16) Serial.print('0');
    Serial.print(*p++, HEX);
    count++;
    if (((count & 0x1f) == 0) && len) Serial.print("\n");
    else Serial.print(' ');
  } 
  Serial.println();
}



bool HIDDumpController::hid_process_in_data(const Transfer_t *transfer){
  hid_input_begin_level_ = 0;
  count_usages_ = index_usages_;
  index_usages_ = 0;

  Serial.print(": ");
    dump_hexbytes(transfer->buffer, transfer->length);

  keyPressedEvent(transfer->buffer, transfer->length);
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
