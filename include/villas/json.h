#pragma once

#ifdef WITH_JANSSON

#include <jansson.h>
#include <libconfig.h>

#include "sample.h"

/* Convert a libconfig object to a libjansson object */
json_t * config_to_json(config_setting_t *cfg);

int json_to_config(json_t *json, config_setting_t *parent);

int sample_io_json_pack(json_t **j, struct sample *s, int flags);

int sample_io_json_unpack(json_t *j, struct sample *s, int *flags);

int sample_io_json_fprint(FILE *f, struct sample *s, int flags);

int sample_io_json_fscan(FILE *f, struct sample *s, int *flags);
#endif
