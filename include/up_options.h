/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifndef UP_OPTIONS_H
#define UP_OPTIONS_H

/* #undef LOG_ENABLE */
#define OPTION_UP_MONO

#ifndef UP_LOG_LEVEL
#define UP_LOG_LEVEL            (LOG_LEVEL_INFO)
#endif

#ifndef UP_TICK_IN_US
#define UP_TICK_IN_US           (1000)
#endif

#ifndef UP_RPC_TIMEOUT
#define UP_RPC_TIMEOUT         (500)
#endif

#ifndef UP_CORE_WATCHDOG_IN_US
#define UP_CORE_WATCHDOG_IN_US  (10000)
#endif

#ifndef UP_APP_POLL_INTERVAL_IN_US
#define UP_APP_POLL_INTERVAL_IN_US  (10000)
#endif

#endif  /* UP_OPTIONS_H */
