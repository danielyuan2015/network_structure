/*
 * realtime.h
 *
 *  Created on: Jun 7, 2016
 *      Author: honeywell
 */

#ifndef SRC_REALTIME_H_
#define SRC_REALTIME_H_

#define PRIORITY_SIGNALING_THREAD  40

#define PRIORITY_TRIGGER_MODE_MANAGER_THREAD  35
#define PRIORITY_KEY_LISTENER_THREAD  30

#define PRIORITY_CAPTURE_THREAD    20
#define PRIORITY_BARCODE_PROCESS_MANAGER_THREAD  18
#define PRIORITY_FRAMERATE_EVT_MANAGER_THREAD	15
#define PRIORITY_FRAMERATE_DECODER_THREAD  10

int set_sched_fifo(int priority);
int set_sched_other(int priority);

void set_thread_name(const char *name);

#endif /* SRC_REALTIME_H_ */
