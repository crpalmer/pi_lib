#include "pi.h"
#include "consoles.h"
#include "bluetooth/bluetooth.h"
#include "avrcp.h"
#include "sbc-configuration.h"

// logarithmic volume reduction, samples are divided by 2^x
// #define VOLUME_REDUCTION 3

#define NUM_CHANNELS                2
#define AUDIO_TIMEOUT_MS            10 
#define SBC_STORAGE_SIZE 1030

typedef struct {

    uint32_t time_audio_data_sent; // ms
    uint32_t acc_num_missed_samples;
    uint32_t samples_ready;
    btstack_timer_source_t audio_timer;
    uint8_t  streaming;
    int      max_media_payload_size;
    uint32_t rtp_timestamp;

    uint8_t  sbc_storage[SBC_STORAGE_SIZE];
    uint16_t sbc_storage_count;
    uint8_t  sbc_ready_to_send;
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

static void produce_sine_audio(int16_t * pcm_buffer, int num_samples_to_write){
    int count;
    for (count = 0; count < num_samples_to_write ; count++){
	pcm_buffer[count * 2]     = sine_int16_44100[sine_phase];
	pcm_buffer[count * 2 + 1] = sine_int16_44100[sine_phase];
	sine_phase++;
	if (sine_phase >= num_samples_sine_int16_44100){
	    sine_phase -= num_samples_sine_int16_44100;
	}
    }
}

static void produce_audio(int16_t * pcm_buffer, int num_samples){
    produce_sine_audio(pcm_buffer, num_samples);
#ifdef VOLUME_REDUCTION
    int i;
    for (i=0;i<num_samples*2;i++){
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

class A2DPSource {
public:
    A2DPSource();

    void hack_can_send_now() {
        a2dp_source_stream_endpoint_request_can_send_now(a2dp_cid, local_seid);
    }

    int get_recommended_buffer_size();

    uint8_t connect(const char *address);
    uint8_t disconnect();
    uint8_t play_stream();
    uint8_t pause_stream();

    bool set_volume(int volume);
    bool volume_up(int inc = 10);
    bool volume_down(int inc = 10);

    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

private:
    void send_media_packet(void);

private:
    AVRCP *avrcp;
    uint8_t sdp_a2dp_source_service_buffer[150];
    uint8_t sdp_avrcp_target_service_buffer[200];
    uint8_t sdp_avrcp_controller_service_buffer[200];

    int volume = 50;

    SBCConfiguration *configuration;
    btstack_sbc_encoder_state_t sbc_encoder_state;

    uint8_t media_sbc_codec_capabilities[4] = {
         (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
         0xFF,//(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
         2, 53
    }; 
    uint8_t media_sbc_codec_configuration[4];

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

static a2dp_media_sending_context_t media_tracker;

void A2DPSource::send_media_packet(void) {
    int num_bytes_in_frame = btstack_sbc_encoder_sbc_buffer_length();
    int bytes_in_storage = media_tracker.sbc_storage_count;
    uint8_t num_sbc_frames = bytes_in_storage / num_bytes_in_frame;
    // Prepend SBC Header
    media_tracker.sbc_storage[0] = num_sbc_frames;  // (fragmentation << 7) | (starting_packet << 6) | (last_packet << 5) | num_frames;
    a2dp_source_stream_send_media_payload_rtp(a2dp_cid, local_seid, 0,
                                               media_tracker.rtp_timestamp,
                                               media_tracker.sbc_storage, bytes_in_storage + 1);

    // update rtp_timestamp
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    media_tracker.rtp_timestamp += num_sbc_frames * num_audio_samples_per_sbc_buffer;

    media_tracker.sbc_storage_count = 0;
    media_tracker.sbc_ready_to_send = 0;
}

static int a2dp_demo_fill_sbc_audio_buffer(a2dp_media_sending_context_t * context){
    // perform sbc encoding
    int total_num_bytes_read = 0;
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    while (context->samples_ready >= num_audio_samples_per_sbc_buffer
        && (context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length()){

        int16_t pcm_frame[256*NUM_CHANNELS];

        produce_audio(pcm_frame, num_audio_samples_per_sbc_buffer);
        btstack_sbc_encoder_process_data(pcm_frame);
        
        uint16_t sbc_frame_size = btstack_sbc_encoder_sbc_buffer_length(); 
        uint8_t * sbc_frame = btstack_sbc_encoder_sbc_buffer();
        
        total_num_bytes_read += num_audio_samples_per_sbc_buffer;
        // first byte in sbc storage contains sbc media header
        memcpy(&context->sbc_storage[1 + context->sbc_storage_count], sbc_frame, sbc_frame_size);
        context->sbc_storage_count += sbc_frame_size;
        context->samples_ready -= num_audio_samples_per_sbc_buffer;
    }

    if ((context->sbc_storage_count + btstack_sbc_encoder_sbc_buffer_length()) > context->max_media_payload_size){
        // schedule sending
        context->sbc_ready_to_send = 1;
	global_a2dp_source->hack_can_send_now();
    }

    return total_num_bytes_read;
}

static void a2dp_demo_audio_timeout_handler(btstack_timer_source_t * timer){
    a2dp_media_sending_context_t * context = (a2dp_media_sending_context_t *) btstack_run_loop_get_timer_context(timer);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS); 
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    uint32_t update_period_ms = AUDIO_TIMEOUT_MS;
    if (context->time_audio_data_sent > 0){
        update_period_ms = now - context->time_audio_data_sent;
    } 

    uint32_t num_samples = (update_period_ms * SAMPLE_RATE) / 1000;
    context->acc_num_missed_samples += (update_period_ms * SAMPLE_RATE) % 1000;
    
    while (context->acc_num_missed_samples >= 1000){
        num_samples++;
        context->acc_num_missed_samples -= 1000;
    }
    context->time_audio_data_sent = now;
    context->samples_ready += num_samples;

    if (context->sbc_ready_to_send) return;

    a2dp_demo_fill_sbc_audio_buffer(context);
}

static void a2dp_demo_timer_start(a2dp_media_sending_context_t * context){
    context->max_media_payload_size = global_a2dp_source->get_recommended_buffer_size();
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    context->streaming = 1;
    btstack_run_loop_remove_timer(&context->audio_timer);
    btstack_run_loop_set_timer_handler(&context->audio_timer, a2dp_demo_audio_timeout_handler);
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS); 
    btstack_run_loop_add_timer(&context->audio_timer);
}

static void a2dp_demo_timer_stop(a2dp_media_sending_context_t * context){
    context->time_audio_data_sent = 0;
    context->acc_num_missed_samples = 0;
    context->samples_ready = 0;
    context->streaming = 1;
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    btstack_run_loop_remove_timer(&context->audio_timer);
} 

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

A2DPSource::A2DPSource() {
    assert( ! global_a2dp_source);
    global_a2dp_source = this;

    configuration = new SBCConfiguration();

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

int A2DPSource::get_recommended_buffer_size() {
    return btstack_min(a2dp_max_media_payload_size(a2dp_cid, local_seid), SBC_STORAGE_SIZE);
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
    return a2dp_source_start_stream(a2dp_cid, local_seid);
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

	    configuration->receive(packet);
	    configuration->dump();
	    configuration->encoder_init(&sbc_encoder_state);
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

            a2dp_demo_timer_start(&media_tracker);
            printf("A2DP Source: Stream started, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
            break;

        case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
            local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
            cid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(packet);
	    global_a2dp_source->send_media_packet();
            break;        

        case A2DP_SUBEVENT_STREAM_SUSPENDED:
            local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
            cid = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);
            
            playback_status = AVRCP_PLAYBACK_STATUS_PAUSED;
	    global_avrcp->set_playback_status(AVRCP_PLAYBACK_STATUS_PAUSED);
            printf("A2DP Source: Stream paused, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
            
            a2dp_demo_timer_stop(&media_tracker);
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
            a2dp_demo_timer_stop(&media_tracker);
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
            break;
        
        case 'P':
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

int btstack_main() {
    bluetooth_init();
    new A2DPSource();
    bluetooth_start_a2dp_source();
    return 0;
}
