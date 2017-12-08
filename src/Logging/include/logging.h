/*
 * logging.h
 *
 *  Created on: Jun 7, 2016
 *      Author: honeywell
 */

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

#include <functional>
#include <stdarg.h>
#include <syslog.h>

#define LOG_PRINT 8
#define LOG_NONE  9

void log_print(unsigned char level,const char *tag, const char *format, ...);
void log_set_level(unsigned char level);
void log_set_tag(const char *tag);
void log_register_callback(std::function<void(const char *format, va_list ap)> f);

#endif /* SRC_LOGGING_H_ */
