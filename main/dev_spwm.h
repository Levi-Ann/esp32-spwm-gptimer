#ifndef __DEV_SPWM_H__
#define __DEV_SPWM_H__


//generate 50hz SPWM wave
//pa and pb is gpio_num
void spwm_init(int pa, int pb);

//if x > 0, generate sin spwm
//if x == 0, output pa==pb==0
//if x < 0, output pa==pb==1.
void spwm_enable(int x);

#endif
