/***************************************************************************/
#include <data.h>
#include <led.h>

void led_set_redgreen(CO_Data *d, unsigned char state)
{
        if (state & 0x01)
                printf("\e[41m ERROR LED ON \e[m          ");
        else
                printf("\e[31m error led off \e[m         ");

        if (state & 0x02)
                printf("\e[34;42m RUN LED ON \e[m\n");
        else
                printf("\e[32m run led off \e[m\n");
}


