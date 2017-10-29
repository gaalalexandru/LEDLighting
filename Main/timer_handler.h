/*
 * timer_handler.h
 *
 * Created: 10/5/2017 9:32:37 PM
 *  Author: Gaal Alexandru
 */ 

#ifndef TIMEHDL_H_
#define TIMEHDL_H_

#define timer_counter_running() (timer_counter_target_ms > timer_ms())

void timer0_init(void);
void timer1_init(void);
void timer2_init(void);

uint32_t timer_ms(void);
void timer_delay_ms(uint32_t delay);
void timer_counter_setup(uint16_t second);

#endif /* TIMEHDL_H_ */