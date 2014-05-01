#include "applicfg.h"
#include "data.h"

void initTimer(void);
void clearTimer(void);

unsigned char canSend(CAN_PORT notused, Message *m);
unsigned char canInit(CO_Data * d, unsigned int bitrate);
void canClose(void);

void disable_it(void);
void enable_it(void);
