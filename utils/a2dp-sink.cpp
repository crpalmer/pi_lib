#include "pi.h"
#include "audio-pico.h"
#include "audio-player.h"
#include "fillable-buffer.h"
#include "bluetooth/a2dp-sink.h"
#include "bluetooth/avrcp.h"
#include "bluetooth/bluetooth.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wifi.h"

class Sink : public A2DPSink {
public:
   Sink();
   void on_configure(AudioConfig *config) override;
   void on_pcm_data(uint8_t *buffer, size_t n_bytes) override;

private:
   Audio *audio;
   AudioPlayer *player;
   FillableBuffer *buffer;
   AudioBuffer *audio_buffer = NULL;
};

Sink::Sink() {
    audio = new AudioPico();
    player = new AudioPlayer(audio);
}

void Sink::on_configure(AudioConfig *config) {
    if (audio_buffer) {
	printf("A2DPSink: Stopping player.\n");
	buffer->set_is_eof(true);
	player->stop();
	player->wait_all_done();
	delete audio_buffer;
	printf("A2DPSink: Player stopped.\n");
    }

    printf("A2DPSink: Configured as %d channels, %d khz\n", config->get_num_channels(), config->get_rate());
    buffer = new FillableBuffer();
    audio_buffer = new AudioBuffer(buffer, config);

    printf("A2DPSink: Starting player.\n");
    player->play(audio_buffer);
    pi_threads_dump_state();
    printf("A2DPSink: %ld\n", (long) pi_threads_get_free_ram());
}

void Sink::on_pcm_data(uint8_t *data, size_t n_bytes) {
    buffer->fill(data, n_bytes);
}

void thread_main(int argc, char **argv) {
    ms_sleep(1000);

#ifdef WITH_WIFI
    wifi_init(CYW43_HOST_NAME);
    //wifi_wait_for_connection();
#endif

    bluetooth_init();

    new Sink();
    new AVRCP();
    bluetooth_start_a2dp_sink();

    while (1) ms_sleep(1000000);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
