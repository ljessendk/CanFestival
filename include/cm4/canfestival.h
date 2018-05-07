#include "applicfg.h"

struct CO_Data;
struct Message;

void initTimer(void);
void clearTimer(void);

unsigned char canSend(CAN_PORT notused, Message *m);
unsigned char canInit(CO_Data * d, uint32_t bitrate);
void canClose(void);

void disable_it(void);
void enable_it(void);
