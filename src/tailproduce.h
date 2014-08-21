// TODO(dkorolev): Add a Makefile target to ensure that each header can be compiled independently.
// TODO(dkorolev): Retire/refactor StreamsRegistry?

#ifndef TAILPRODUCE_H
#define TAILPRODUCE_H

#include <glog/logging.h>

#include "config_values.h"
#include "event_subscriber.h"
#include "listeners.h"
#include "publishers.h"
#include "serialize.h"
#include "static_framework.h"
#include "storage.h"
#include "stream.h"
#include "stream_manager_params.h"
#include "tp_exceptions.h"

#include "tailproduce.macros"

#endif  // TAILPRODUCE_H
