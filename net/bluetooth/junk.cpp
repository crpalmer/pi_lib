#include "pi.h"
#include "avrcp.h"
#include "bluetooth/bluetooth.h"
#include "consoles.h"
#include "pi-threads.h"
#include "sbc-configuration.h"

// logarithmic volume reduction, samples are divided by 2^x
#define VOLUME_REDUCTION 3

#define NUM_CHANNELS                2
#define AUDIO_TIMEOUT_MS            10 
#define SBC_STORAGE_SIZE 1030

typedef struct {

    uint32_t time_audio_data_sent; // ms
    uint32_t acc_num_missed_samples;
    btstack_timer_source_t audio_timer;
    uint8_t  streaming;
} a2dp_media_sending_context_t;

/* ---- sine wave --- */

#define SAMPLE_RATE 44100

// input signal: pre-computed int16 sine wave, 44100 Hz at 441 Hz
static const int16_t sine_int16_44100[] = {
     0,    2057,    4107,    6140,    8149,   10126,   12062,   13952,   15786,   17557,
 19260,   20886,   22431,   23886,   25247,   26509,   27666,   28714,   29648,   30466,
 31163,   31738,   32187,   32509,   32702,   32767,   32702,   32509,   32187,   31738,
 31163,   30466,   29648,   28714,   27666,   26509,   25247,   23886,   22431,   20886,
 19260,   17557,   15786,   13952,   12062,   10126,    8149,    6140,    4107,    2057,
     0,   -2057,   -4107,   -6140,   -8149,  -10126,  -12062,  -13952,  -15786,  -17557,
-19260,  -20886,  -22431,  -23886,  -25247,  -26509,  -27666,  -28714,  -29648,  -30466,
-31163,  -31738,  -32187,  -32509,  -32702,  -32767,  -32702,  -32509,  -32187,  -31738,
-31163,  -30466,  -29648,  -28714,  -27666,  -26509,  -25247,  -23886,  -22431,  -20886,
-19260,  -17557,  -15786,  -13952,  -12062,  -10126,   -8149,   -6140,   -4107,   -2057,
};

static const int num_samples_sine_int16_44100 = sizeof(sine_int16_44100) / 2;

static int sine_phase;

static void produce_audio(int16_t * pcm_buffer, int num_samples_to_write){
    int count;
    for (count = 0; count < num_samples_to_write ; count++){
	pcm_buffer[count * 2]     = sine_int16_44100[sine_phase];
	pcm_buffer[count * 2 + 1] = sine_int16_44100[sine_phase];
	sine_phase++;
	if (sine_phase >= num_samples_sine_int16_44100){
	    sine_phase -= num_samples_sine_int16_44100;
	}
    }
#ifdef VOLUME_REDUCTION
    int i;
    for (i=0;i<num_samples_to_write*2;i++){
        if (pcm_buffer[i] > 0){
            pcm_buffer[i] =     pcm_buffer[i]  >> VOLUME_REDUCTION;
        } else {
            pcm_buffer[i] = -((-pcm_buffer[i]) >> VOLUME_REDUCTION);
        }
    }
#endif
}

static const char * device_addr_string = "BF:AC:94:0A:99:2E";
#ifdef HAVE_BTSTACK_STDIN
static void stdin_process(char cmd);
#endif

static AVRCP *global_avrcp;	// Temporary until everything into A2DSource object

class SBCEncoder {
public:
    SBCEncoder();

    void receive_configuration(uint8_t *packet) { configuration->receive(packet); }
    void init(uint16_t a2dp_cid, uint16_t local_seid);
    void enqueue_pcm_data(int16_t *pcm, int n_samples);
    void send_media_payload_rtp();
    void resume() { cond->signal(); }

    int get_max_buffer_size() {
        return btstack_min(a2dp_max_media_payload_size(a2dp_cid, local_seid), SBC_STORAGE_SIZE);
    }

    void dump_state() { configuration->dump(); }

private:
    SBCConfiguration *configuration;
    PiMutex *lock;
    PiCond *cond;

    uint16_t a2dp_cid;
    uint16_t local_seid;

    btstack_sbc_encoder_state_t state;

    uint32_t rtp_timestamp = 0;
    uint8_t  sbc_buffer[SBC_STORAGE_SIZE];
    int16_t *pcm_buffer = NULL;
    uint16_t n_sbc_buffer = 0;
    uint16_t a_pcm_buffer = 0;
    uint16_t n_pcm_buffer = 0;
    bool ready_to_send = false;
};

SBCEncoder::SBCEncoder() {
    configuration = new SBCConfiguration();
    lock = new PiMutex();
    cond = new PiCond();
}

void SBCEncoder::init(uint16_t a2dp_cid, uint16_t local_seid) {
    configuration->encoder_init(&state);
    this->a2dp_cid = a2dp_cid;
    this->local_seid = local_seid;

    if (pcm_buffer) fatal_free(pcm_buffer);
    a_pcm_buffer = btstack_sbc_encoder_num_audio_frames();
    pcm_buffer = (int16_t *) fatal_malloc(a_pcm_buffer * sizeof(pcm_buffer) * NUM_CHANNELS);
    cond->signal();
}

// enqueue_pcm_data will block if too much data has built up and we are
// waiting for it to be sent.

void SBCEncoder::enqueue_pcm_data(int16_t *pcm, int n_samples) {
    lock->lock();

    while (! a_pcm_buffer) {
	printf("%s: not intialized\n", __func__);
	cond->wait(lock);
    }

    while (n_samples) {
	// If we have less than a total buffer to process, just save the pcm data for the
	// next time we get more data

	if (n_pcm_buffer + n_samples < a_pcm_buffer) {
	    memcpy(&pcm_buffer[n_pcm_buffer * NUM_CHANNELS], pcm, n_samples * NUM_CHANNELS * sizeof(pcm_buffer));
	    n_pcm_buffer += n_samples;
	    break;
	}

	// We have atleast one full buffer for encoding, encode it (the new encoded
	// buffer is stored in the sbc object until we can copy it to our local storage)

	if (1 || n_pcm_buffer) {
	    // Combine the existing unprocessed data with the new data
	    int n_to_copy = a_pcm_buffer - n_pcm_buffer;
	    assert(n_to_copy <= n_samples);	// If it wasn't, we would have handled it (above)
	    memcpy(&pcm_buffer[n_pcm_buffer * NUM_CHANNELS], pcm, n_to_copy * NUM_CHANNELS * sizeof(pcm_buffer));
printf("%d + %d of %d\n", n_pcm_buffer, n_to_copy, n_samples);
	    btstack_sbc_encoder_process_data(pcm_buffer);
	    n_pcm_buffer = 0;
	    pcm += n_to_copy;
	    n_samples -= n_to_copy;
	} else {
	    // No previous unprocessed data, skip the extra copying and
	    // process it directly.
	    btstack_sbc_encoder_process_data(pcm);
	    pcm += a_pcm_buffer;
	    n_samples -= a_pcm_buffer;
	}

	uint8_t * sbc_frame = btstack_sbc_encoder_sbc_buffer();
	int sbc_len = btstack_sbc_encoder_sbc_buffer_length();

	// If there is no room for more data, send the buffer and wait until it is
	// sent so that we have space to store the new data

	while (n_sbc_buffer + sbc_len > get_max_buffer_size()) {
	    void a2dp_source_request_can_send_now();  // TODO forward declaration not neeed after spliting headers/src
	    a2dp_source_request_can_send_now();
	    cond->wait(lock);
	}

	memcpy(&sbc_buffer[n_sbc_buffer], sbc_frame, sbc_len);
	n_sbc_buffer += sbc_len;
    }

    lock->unlock();
}

void SBCEncoder::send_media_payload_rtp() {
    lock->lock();

printf("send\n");
    uint8_t num_frames = n_sbc_buffer / btstack_sbc_encoder_sbc_buffer_length();
    // Prepend SBC Header
    sbc_buffer[0] = num_frames;  // (fragmentation << 7) | (starting_packet << 6) | (last_packet << 5) | num_frames;
    a2dp_source_stream_send_media_payload_rtp(a2dp_cid, local_seid, 0, rtp_timestamp, sbc_buffer, n_sbc_buffer + 1);

    // update rtp_timestamp
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    rtp_timestamp += num_frames * num_audio_samples_per_sbc_buffer;

    n_sbc_buffer = 0;

    cond->signal();
    lock->unlock();
}

class A2DPSource {
public:
    A2DPSource();

    int get_recommended_buffer_size();

    uint8_t connect(const char *address);
    uint8_t disconnect();
    uint8_t play_stream();
    uint8_t pause_stream();

    bool is_playing() { return avrcp->is_playing(); }

    void play(int16_t *pcm, int n_samples);

    bool set_volume(int volume);
    bool volume_up(int inc = 10);
    bool volume_down(int inc = 10);

    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

    void request_can_send_now();
    void call_request_can_send_now();

private:
    AVRCP *avrcp;
    uint8_t sdp_a2dp_source_service_buffer[150];
    uint8_t sdp_avrcp_target_service_buffer[200];
    uint8_t sdp_avrcp_controller_service_buffer[200];

    int volume = 50;

    SBCEncoder *encoder;

    uint8_t media_sbc_codec_capabilities[4] = {
         (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
         0xFF,//(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
         2, 53
    }; 
    uint8_t media_sbc_codec_configuration[4];

    btstack_context_callback_registration_t request_can_send_now_callback_registration;
    avrcp_playback_status_t playback_status = AVRCP_PLAYBACK_STATUS_STOPPED;
    uint16_t a2dp_cid;
    uint8_t  local_seid;
};

class A2DPAVRCP : public AVRCP {
public:
    A2DPAVRCP(A2DPSource *a2dp) : AVRCP(), a2dp(a2dp) {
    }

    void on_button_pressed(avrcp_button_t button) override;

private:
    A2DPSource *a2dp;
};

static A2DPSource *global_a2dp_source = NULL;

/* SBC-Encoder */

#if 0
#include "time-utils.h"

class Player : PiThread {
public:
    Player() : PiThread("player") {
	lock = new PiMutex();
	cond = new PiCond();
	start();
    }

    void main() {
	nano_gettime(&start_time);
	missed_samples = 0;

	while (1) {
	    while (is_paused) {
		cond->wait(lock);
	    }

	    int ms = nano_elapsed_ms_now(&start_time);

    	    uint32_t num_samples = (ms * SAMPLE_RATE) / 1000;
    	    missed_samples += (ms * SAMPLE_RATE) % 1000;

	    while (missed_samples >= 1000) {
		num_samples++;
		missed_samples -= 1000;
	    }
	    
	    while (num_samples > 0) {
		int n_now = num_samples > 128 ? 128 : num_samples;
		produce_audio(pcm_frame, n_now);
		lock->unlock();
		global_a2dp_source->play(pcm_frame, n_now);
		ms_sleep(AUDIO_TIMEOUT_MS);
		lock->lock();
		num_samples -= n_now;
	    }
	}
    }

    void pause() {
	lock->lock();
	is_paused = true;
	lock->unlock();
    }

    void resume() {
printf("resuming player\n");
	lock->lock();
	if (is_paused) {
	    nano_gettime(&start_time);
	    missed_samples = 0;
	    is_paused = false;
	}
	cond->signal();
	lock->unlock();
printf("resumed player\n");
    }

private:
    bool is_paused = true;
    PiMutex *lock;
    PiCond *cond;

    int16_t pcm_frame[256*NUM_CHANNELS];
    struct timespec start_time;
    uint32_t missed_samples = 0;

};

static class Player *global_player;

#endif
/* END SBC-Encoder */

void A2DPAVRCP::on_button_pressed(avrcp_button_t button) {
    switch (button) {
    case AVRCP_BUTTON_PLAY: a2dp->play_stream();
    case AVRCP_BUTTON_PAUSE: a2dp->pause_stream();
    case AVRCP_BUTTON_STOP: a2dp->disconnect();
    default: break;
    }
}

static void C_a2dp_source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    assert(global_a2dp_source);
    global_a2dp_source->packet_handler(packet_type, channel, packet, size);
}

void a2dp_source_request_can_send_now() {
    global_a2dp_source->request_can_send_now();
}

static void C_call_request_can_send_now_callback(void *a2dp_source_as_vp) {
    A2DPSource *a2dp_source = (A2DPSource *) a2dp_source_as_vp;
    a2dp_source->call_request_can_send_now();
}

A2DPSource::A2DPSource() {
    assert( ! global_a2dp_source);
    global_a2dp_source = this;

    request_can_send_now_callback_registration.context = this;
    request_can_send_now_callback_registration.callback = C_call_request_can_send_now_callback;

    encoder = new SBCEncoder();

    // Request role change on reconnecting headset to always use them in slave mode
    hci_set_master_slave_policy(0);

    // Initialize  A2DP Source
    a2dp_source_init();
    a2dp_source_register_packet_handler(C_a2dp_source_packet_handler);

    // Create stream endpoint
    avdtp_stream_endpoint_t * local_stream_endpoint = a2dp_source_create_stream_endpoint(AVDTP_AUDIO, AVDTP_CODEC_SBC, media_sbc_codec_capabilities, sizeof(media_sbc_codec_capabilities), media_sbc_codec_configuration, sizeof(media_sbc_codec_configuration));
    if (!local_stream_endpoint){
        consoles_fatal_printf("A2DP Source: not enough memory to create local stream endpoint\n");
    }

    // Store stream enpoint's SEP ID, as it is used by A2DP API to indentify the stream endpoint
    local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(local_seid);

    // Initialize AVRCP Service
    avrcp = new A2DPAVRCP(this);
    global_avrcp = avrcp;

    // Create A2DP Source service record and register it with SDP
    memset(sdp_a2dp_source_service_buffer, 0, sizeof(sdp_a2dp_source_service_buffer));
    a2dp_source_create_sdp_record(sdp_a2dp_source_service_buffer, 0x10001, AVDTP_SOURCE_FEATURE_MASK_PLAYER, NULL, NULL);
    sdp_register_service(sdp_a2dp_source_service_buffer);
    
    // Create AVRCP Target service record and register it with SDP. We receive Category 1 commands from the headphone, e.g. play/pause
    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t supported_features = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10002, supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // Create AVRCP Controller service record and register it with SDP. We send Category 2 commands to the headphone, e.g. volume up/down
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features = AVRCP_FEATURE_MASK_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, 0x10003, controller_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

#ifdef HAVE_BTSTACK_STDIN
    btstack_stdin_setup(stdin_process);
#endif
}

void A2DPSource::request_can_send_now() {
    btstack_run_loop_execute_on_main_thread(&request_can_send_now_callback_registration);
}

void A2DPSource::call_request_can_send_now() {
    a2dp_source_stream_endpoint_request_can_send_now(a2dp_cid, local_seid);
}

int A2DPSource::get_recommended_buffer_size() {
    return encoder->get_max_buffer_size();
}

uint8_t A2DPSource::connect(const char *address) {
    bd_addr_t bd_addr;
    sscanf_bd_addr(address, bd_addr);
    uint8_t status = a2dp_source_establish_stream(bd_addr, &a2dp_cid);
    printf("Create A2DP Source connection to addr %s, cid 0x%02x.\n", address, a2dp_cid);
    return status;
}

uint8_t A2DPSource::disconnect() {
    printf("A2DP Source Disconnect from cid 0x%2x\n", a2dp_cid);
    return a2dp_source_disconnect(a2dp_cid);
}

uint8_t A2DPSource::pause_stream() {
    printf("Pause stream.\n");
    return a2dp_source_pause_stream(a2dp_cid, local_seid);
}

uint8_t A2DPSource::play_stream() {
    printf("Resume stream.\n");
    bool ret = a2dp_source_start_stream(a2dp_cid, local_seid);
    encoder->resume();
    return ret;
}

void A2DPSource::play(int16_t *pcm, int n_samples) {
    encoder->enqueue_pcm_data(pcm, n_samples);
}

void A2DPSource::packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    uint8_t status;
    bd_addr_t address;
    uint16_t cid;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) return;

    switch (hci_event_a2dp_meta_get_subevent_code(packet)){
        case A2DP_SUBEVENT_SIGNALING_CONNECTION_ESTABLISHED:
            a2dp_subevent_signaling_connection_established_get_bd_addr(packet, address);
            cid = a2dp_subevent_signaling_connection_established_get_a2dp_cid(packet);
            status = a2dp_subevent_signaling_connection_established_get_status(packet);

            if (status != ERROR_CODE_SUCCESS){
                printf("A2DP Source: Connection failed, status 0x%02x, cid 0x%02x, a2dp_cid 0x%02x \n", status, cid, a2dp_cid);
                a2dp_cid = 0;
                break;
            }
            a2dp_cid = cid;

            printf("A2DP Source: Connected to address %s, a2dp cid 0x%02x, local seid 0x%02x.\n", bd_addr_to_str(address), a2dp_cid, local_seid);
            break;

         case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION:
            cid  = avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(packet);
            if (cid != a2dp_cid) return;

	    encoder->receive_configuration(packet);
	    encoder->dump_state();
	    encoder->init(a2dp_cid, local_seid);
            break;

        case A2DP_SUBEVENT_SIGNALING_DELAY_REPORTING_CAPABILITY:
            printf("A2DP Source: remote supports delay report, remote seid %d\n", 
                avdtp_subevent_signaling_delay_reporting_capability_get_remote_seid(packet));
            break;
        case A2DP_SUBEVENT_SIGNALING_CAPABILITIES_DONE:
            printf("A2DP Source: All capabilities reported, remote seid %d\n", 
                avdtp_subevent_signaling_capabilities_done_get_remote_seid(packet));
            break;

        case A2DP_SUBEVENT_SIGNALING_DELAY_REPORT:
            printf("A2DP Source: Received delay report of %d.%0d ms, local seid %d\n", 
                avdtp_subevent_signaling_delay_report_get_delay_100us(packet)/10, avdtp_subevent_signaling_delay_report_get_delay_100us(packet)%10,
                avdtp_subevent_signaling_delay_report_get_local_seid(packet));
            break;
       
        case A2DP_SUBEVENT_STREAM_ESTABLISHED:
            a2dp_subevent_stream_established_get_bd_addr(packet, address);
            status = a2dp_subevent_stream_established_get_status(packet);
            if (status != ERROR_CODE_SUCCESS){
                printf("A2DP Source: Stream failed, status 0x%02x.\n", status);
                break;
            }
            
            local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
            cid = a2dp_subevent_stream_established_get_a2dp_cid(packet);
            
            printf("A2DP Source: Stream established a2dp_cid 0x%02x, local_seid 0x%02x, remote_seid 0x%02x\n", cid, local_seid, a2dp_subevent_stream_established_get_remote_seid(packet));
            
            status = a2dp_source_start_stream(a2dp_cid, local_seid);
            break;

        case A2DP_SUBEVENT_STREAM_RECONFIGURED:
            status = a2dp_subevent_stream_reconfigured_get_status(packet);
            local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
            cid = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

            if (status != ERROR_CODE_SUCCESS){
                printf("A2DP Source: Stream reconfiguration failed, status 0x%02x\n", status);
                break;
            }

            printf("A2DP Source: Stream reconfigured a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
            status = a2dp_source_start_stream(a2dp_cid, local_seid);
            break;

        case A2DP_SUBEVENT_STREAM_STARTED:
            local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
            cid = a2dp_subevent_stream_started_get_a2dp_cid(packet);

            playback_status = AVRCP_PLAYBACK_STATUS_PLAYING;
	    global_avrcp->set_playback_status(AVRCP_PLAYBACK_STATUS_PLAYING);

            printf("A2DP Source: Stream started, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
            break;

        case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
            local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
	    encoder->send_media_payload_rtp();
            break;        

        case A2DP_SUBEVENT_STREAM_SUSPENDED:
            local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
            cid = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);
            
            playback_status = AVRCP_PLAYBACK_STATUS_PAUSED;
	    global_avrcp->set_playback_status(AVRCP_PLAYBACK_STATUS_PAUSED);
            printf("A2DP Source: Stream paused, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
            
            break;

        case A2DP_SUBEVENT_STREAM_RELEASED:
            playback_status = AVRCP_PLAYBACK_STATUS_STOPPED;
            cid = a2dp_subevent_stream_released_get_a2dp_cid(packet);
            local_seid = a2dp_subevent_stream_released_get_local_seid(packet);
            
            printf("A2DP Source: Stream released, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);

            if (cid == a2dp_cid) {
                printf("A2DP Source: Stream released.\n");
            }
	    global_avrcp->set_now_playing_info(NULL, 1);
	    global_avrcp->set_playback_status(AVRCP_PLAYBACK_STATUS_STOPPED);
            break;
        case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
            cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
	    printf("A2DP Source: Signaling released.\n\n");
            break;
        default:
            break; 
    }
}

bool A2DPSource::set_volume(int new_volume) {
    if (new_volume > 100) new_volume = 100;
    if (new_volume < 0) new_volume = 0;
    volume = new_volume;
    return global_avrcp->set_volume(volume) == ERROR_CODE_SUCCESS;
}

bool A2DPSource::volume_up(int inc) {
    return set_volume(volume + inc);
}

bool A2DPSource::volume_down(int inc) {
    return set_volume(volume - inc);
}

#ifdef HAVE_BTSTACK_STDIN
static void show_usage(void){
    bd_addr_t      iut_address;
    gap_local_bd_addr(iut_address);
    printf("\n--- Bluetooth  A2DP Source/AVRCP Demo %s ---\n", bd_addr_to_str(iut_address));
    printf("b      - A2DP Source create connection to addr %s\n", device_addr_string);
    printf("B      - A2DP Source disconnect\n");
    printf("c      - AVRCP create connection to addr %s\n", device_addr_string);
    printf("C      - AVRCP disconnect\n");
    printf("D      - delete all link keys\n");

    printf("p      - pause streaming\n");
    printf("P      - resume streaming\n");
    printf("t      - volume up\n");
    printf("T      - volume down\n");

    printf("---\n");
}

static void stdin_process(char cmd){
    uint8_t status = ERROR_CODE_SUCCESS;
    switch (cmd){
        case 'b':
	    global_a2dp_source->connect(device_addr_string);
            break;
        case 'B':
	    global_a2dp_source->disconnect();
            break;
        case 'c': {
	    bd_addr_t device_addr;
	    sscanf_bd_addr(device_addr_string, device_addr);
            printf("%c - Create AVRCP connection to addr %s.\n", cmd, bd_addr_to_str(device_addr));
            status = global_avrcp->connect(device_addr);
            break;
	}
        case 'C':
            printf("%c - AVRCP disconnect\n", cmd);
            status = global_avrcp->disconnect();
            break;
        case 'D':
            printf("Deleting all link keys\n");
            gap_delete_all_link_keys();
            break;
        case '\n':
        case '\r':
            break;

        case 't':
            printf(" - volume up\n");
            status = global_a2dp_source->volume_up();
            break;
        case 'T':
            printf(" - volume down\n");
            status = global_a2dp_source->volume_down();
            break;

        case 'p':
	    status = global_a2dp_source->pause_stream();
	    //global_player->pause();
            break;
        
        case 'P':
	    //global_player->resume();
	    status = global_a2dp_source->play_stream();
            break;
        
        default:
            show_usage();
            return;
    }
    if (status != ERROR_CODE_SUCCESS){
        printf("Could not perform command \'%c\', status 0x%02x\n", cmd, status);
    }
}
#endif

void a2dp_connect() {
    global_a2dp_source->connect(device_addr_string);
}

void a2dp_play(int16_t *buffer, int n_samples) {
    global_a2dp_source->play(buffer, n_samples);
}

int btstack_setup() {
    bluetooth_init();
    //global_player = new Player();
    new A2DPSource();
    bluetooth_start_a2dp_source();
    return 0;
}

int btstack_run() {
    //global_player = new Player();
}
