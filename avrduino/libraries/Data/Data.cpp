#include "Data.h"

Data::Data() {
  EDB channeldb(&reader, &writer);
  EDB programdb(&reader, &writer);

  if(isempty()) {
    init();
  }
}

bool Data::isempty() {
  EDB_Status status;
  status = channeldb.open(START_CHANNELS);
  if(status == EDB_OK) {
    return true;
  }
  else {
    return false;
  }
}

void Data::init() {
  channeldb.create(START_CHANNELS, SIZE_CHANNELS, sizeof(Channel));
  programdb.create(START_PROGRAMS, SIZE_PROGRAMS, sizeof(Program));
}

