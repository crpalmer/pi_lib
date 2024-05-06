#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include "mem.h"

#include "talker-auto-gain.h"

#define SAMPLES_TO_PREDICT	100

struct talker_auto_gainS {
    float *history;
    size_t history_i;
    double sum_history;
    bool history_full;
    double gain;
    double epsilon;
    double gain_target;
    int n_history;
} stats_t;

static void
update_history_and_gain(talker_auto_gain_t *t, double pos)
{
    t->sum_history = t->sum_history - t->history[t->history_i] + pos;
    t->history[t->history_i] = pos;
    t->history_i = (t->history_i + 1) % t->n_history;
    if (t->history_i == 0) {
	t->history_full = true;
    }
    if (t->history_full || t->history_i > SAMPLES_TO_PREDICT) {
	int n = t->history_full ? t->n_history : t->history_i;
	t->gain = t->gain_target / (t->sum_history / n);
    }
}

talker_auto_gain_t *
talker_auto_gain_new(double gain_target, double epsilon, int n_history)
{
    talker_auto_gain_t *t = (talker_auto_gain_t *) fatal_malloc(sizeof(*t));

    memset(t, 0, sizeof(*t));
    t->gain = 1;
    t->gain_target = gain_target;
    t->epsilon = epsilon;
    t->n_history = n_history;

    t->history = fatal_malloc(sizeof(*t->history) * n_history);
    memset(t->history, 0, sizeof(*t->history) * n_history);

    return t;
}

void
talker_auto_gain_destroy(talker_auto_gain_t *t)
{
    fatal_free(t->history);
    fatal_free(t);
}

double
talker_auto_gain_add(talker_auto_gain_t *t, double pos)
{
    if (pos >= t->epsilon) {
	update_history_and_gain(t, pos);
    }

    pos *= t->gain;
    if (pos > 100) pos = 100;

    return pos;
}
