/***************************************************************************/
#include <led.h>

void led_set_redgreen(CO_Data *d, int state)
{

        printf("LEDS %d\n",bits);

        if (bits & 0x01)
                printf("\e[41m ERROR LED ON \e[m\n");
        else
                printf("error led off\n");

        if (bits & 0x02)
                printf("\e[34;42m RUN LED ON \e[m\n");
        else
                printf("run led off\n");
}


