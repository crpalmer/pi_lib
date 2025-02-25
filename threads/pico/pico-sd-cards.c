#include "pi.h"
#include "hw_config.h"
#include "pico-sd-cards.h"

static spi_t spi[2];
static bool spi_used[2];

typedef struct {
    sd_spi_if_t spi_if;
    sd_card_t sd_card;
} sd_t;

#define MAX_CARDS 10
static sd_t cards[MAX_CARDS];
int n_cards = 0;

size_t sd_get_num() { return n_cards; }
int pico_get_n_sd_cards() { return n_cards; }

sd_card_t *sd_get_by_num(size_t num) {
    if (num < n_cards) {
        return &cards[num].sd_card;
    } else {
        return NULL;
    }
}

int pico_add_sd_card(const char *path, int bus, int sck, int mosi, int miso, int cs, int baud) {
    if (n_cards >= MAX_CARDS) return -1;
    
    sd_t *sd = &cards[n_cards];

    if (! spi_used[bus]) {
	/* TODO: try adding .mode = 3, to this structure to improve speed */
	spi[bus].hw_inst = bus == 0 ? spi0 : spi1;  // RP2040 SPI component
	spi[bus].sck_gpio = sck;    // GPIO number (not Pico pin number)
	spi[bus].mosi_gpio = mosi;
	spi[bus].miso_gpio = miso;
	spi[bus].baud_rate = baud;
	spi[bus].DMA_IRQ_num = bus == 0 ? DMA_IRQ_1 : DMA_IRQ_0;
    } else {
	assert(spi[bus].sck_gpio == sck);
	assert(spi[bus].mosi_gpio] == mosi);
	assert(spi[bus].miso_gpio == miso);
    }

    sd->spi_if.spi = &spi[bus];
    sd->spi_if.ss_gpio = cs;

    sd->sd_card.device_name = path;
    sd->sd_card.mount_point = path;
    sd->sd_card.type = SD_IF_SPI;
    sd->sd_card.spi_if_p = &sd->spi_if;

    n_cards++;

    return 0;
};

const char *pico_sd_get_path(int n) {
    if (n < n_cards) return cards[n].sd_card.mount_point;
    return NULL;
}

int pico_add_pico_board_sd_card(const char *path) {
    return pico_add_sd_card(path, 0, 18, 19, 16, 17, 20*1000*1000);
}

const char *pico_get_sd_card_root(int i) {
    if (i >= n_cards) return NULL;
    return cards[i].sd_card.mount_point;
}
