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

#ifndef UP_VERSION_H
#define UP_VERSION_H

#define UPHY_GIT_REVISION "release-v0.7.0"

#if !defined(UP_VERSION_BUILD) && defined(UPHY_GIT_REVISION)
#define UP_VERSION_BUILD UPHY_GIT_REVISION
#endif

/* clang-format-off */

#define UP_VERSION_MAJOR 1
#define UP_VERSION_MINOR 0
#define UP_VERSION_PATCH 1

#if defined(UP_VERSION_BUILD)
#define UP_VERSION \
   "1.0.1+"UP_VERSION_BUILD
#else
#define UP_VERSION \
   "1.0.1"
#endif

/* clang-format-on */

#endif /* UP_VERSION_H */
