#ifndef TALKER_AUTO_GAIN_H__
#define TALKER_AUTO_GAIN_H__

typedef struct talker_auto_gainS talker_auto_gain_t;

talker_auto_gain_t *talker_auto_gain_new(double open_pct, double epsilon, int n_history);
double talker_auto_gain_add(talker_auto_gain_t *t, double pos);
void talker_auto_gain_destroy(talker_auto_gain_t *t);

#endif
