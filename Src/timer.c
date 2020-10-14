/*
 * timer.c
 *
 *  Created on: 2020. 6. 10.
 *      Author:
 */

#include <stdio.h>
#include <string.h>

uint32_t	g_sSysTickCnt = 0;
uint32_t	jiffies = 0;

#define MAXTIMER	10

struct timer_manager
{
	int index;
	uint32_t expires[MAXTIMER];
}timer_list;


#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)((a) - (b)) >= 0))


char set_apptimer(int id, int expires)
{
    if (id > MAXTIMER || id < 0)
    {
        printf("MAXTIMER = %d, id = %d\r\n", MAXTIMER, id);
        return 0;
    }

    memset(&timer_list.expires[id], 0, sizeof(timer_list.expires[id]));

    if (timer_list.expires[id] == 0)
    {
    	timer_list.expires[id] = jiffies + expires;

    	return 1;
    }
    else
    	return 0;
}

char chk_apptimer(int id)
{
    if (id > MAXTIMER || id < 0)
    {
        printf("MAXTIMER = %d, id = %d\r\n",MAXTIMER,id);
        return 0;
    }

    if (time_after_eq(jiffies, timer_list.expires[id]))
    {
    	timer_list.expires[id] = 0;
    	return 1;
    }
    else
    	return 0;
}

void alltimer_release(void)
{
	memset(&timer_list, 0, sizeof(timer_list));
	timer_list.index = 0;
}

void HAL_SYSTICK_Callback(void)
{
	g_sSysTickCnt++;
	jiffies++;
}


