#ifndef DS1_UTILITY_H
#define DS1_UTILITY_H
#include <utils/String16.h>
using namespace android;
void ds1ConfigureHdmiSupportedChannel(int channelCount);

void ds1ConfigureRoutingDevice(unsigned int audioDevice);

bool ds1SendBroadcastMessage(String16 action, int value);

bool checkFlagsToMoveDs(audio_output_flags_t flags);

#endif  /* DS1_UTILITY_H */
