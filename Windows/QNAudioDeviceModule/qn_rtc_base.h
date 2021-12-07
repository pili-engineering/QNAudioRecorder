#ifndef QN_RTC_BASE_H
#define QN_RTC_BASE_H

#define NOMINMAX
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <string>
#include <chrono>
#include "api/peer_connection_interface.h"
#include "modules/audio_device/include/audio_device_data_observer.h"

#define QN_LOG_DEBUG RTC_LOG(LS_VERBOSE) << __FUNCTION__ << " "
#define QN_LOG_INFO  RTC_LOG(LS_INFO) << __FUNCTION__ << " "
#define QN_LOG_WARN  RTC_LOG(LS_WARNING) << __FUNCTION__ << " "
#define QN_LOG_ERROR RTC_LOG(LS_ERROR) << __FUNCTION__ << " "

using namespace webrtc;
using namespace rtc;

#endif //QN_RTC_BASE_H
