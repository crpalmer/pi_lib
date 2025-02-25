#ifndef __PICO_SD_CARDS_H__
#define __PICO_SD_CARDS_H__

#ifdef __cplusplus
extern "C" {
#endif

int pico_get_n_sd_cards();

const char *pico_get_sd_card_root(int i);

int pico_add_sd_card(const char *path, int bus, int sck, int mosi, int miso, int cs, int baud);

int pico_add_pico_board_sd_card(const char *path);

#ifdef __cplusplus
};
#endif

#endif
