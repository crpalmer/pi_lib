#include "pi.h"
#include "consoles.h"

#include "btstack.h"
#include "bluetooth/a2dp-sink.h"
#include "avrcp.h"
#include "sbc-decoder.h"

static A2DPSink *global_sink = NULL;

void C_a2dp_sink_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t event_size) {
    assert(global_sink);
    global_sink->packet_handler(packet_type, channel, packet, event_size);
}

static void C_handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet, uint16_t size) {
    assert(global_sink);
    global_sink->media_packet_handler(seid, packet, size);
}

typedef enum {
    STREAM_STATE_CLOSED,
    STREAM_STATE_OPEN,
    STREAM_STATE_PLAYING,
    STREAM_STATE_PAUSED,
} stream_state_t;

class A2DPSinkConnection {
protected:
    friend class A2DPSink;

    bd_addr_t addr;
    uint16_t  a2dp_cid;
    uint8_t   a2dp_local_seid;
    stream_state_t stream_state;
};

A2DPSink::A2DPSink() {
    assert(! global_sink);
    global_sink = this;
    a2dp_sink_init();
    connection = new A2DPSinkConnection();

    a2dp_sink_register_packet_handler(C_a2dp_sink_packet_handler);
    a2dp_sink_register_media_handler(C_handle_l2cap_media_data_packet);

    avrcp = new AVRCP();
    decoder = new SBCDecoder(this);

    // - Create and register A2DP Sink service record
    memset(service_buffer, 0, sizeof(service_buffer));
    a2dp_sink_create_sdp_record(service_buffer, sdp_create_service_record_handle(), AVDTP_SINK_FEATURE_MASK_HEADPHONE, NULL, NULL);
    sdp_register_service(service_buffer);

    // - Create AVRCP Controller service record and register it with SDP. We send Category 1 commands to the media player, e.g. play/pause
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features = 1 << AVRCP_CONTROLLER_SUPPORTED_FEATURE_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, sdp_create_service_record_handle(), controller_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    //   -  We receive Category 2 commands from the media player, e.g. volume up/down
    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t target_supported_features = 1 << AVRCP_TARGET_SUPPORTED_FEATURE_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, sdp_create_service_record_handle(), target_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

}

void A2DPSink::packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) return;

    switch (packet[2]){
    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_OTHER_CONFIGURATION:
	printf("A2DP  Sink      : Received non SBC codec - not implemented\n");
	break;
    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION:{
	printf("A2DP  Sink      : Received SBC codec configuration\n");
	decoder->receive_configuration(packet);
	decoder->dump_state();
	break;
    }

    case A2DP_SUBEVENT_STREAM_ESTABLISHED:
	status = a2dp_subevent_stream_established_get_status(packet);
	if (status != ERROR_CODE_SUCCESS){
	    printf("A2DP  Sink      : Streaming connection failed, status 0x%02x\n", status);
	    break;
	}

	a2dp_subevent_stream_established_get_bd_addr(packet, connection->addr);
	connection->a2dp_cid = a2dp_subevent_stream_established_get_a2dp_cid(packet);
	connection->a2dp_local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
	connection->stream_state = STREAM_STATE_OPEN;

	printf("A2DP  Sink      : Streaming connection is established, address %s, cid 0x%02x, local seid %d\n",
	       bd_addr_to_str(connection->addr), connection->a2dp_cid, connection->a2dp_local_seid);
	break;
    
    case A2DP_SUBEVENT_STREAM_STARTED:
	printf("A2DP  Sink      : Stream started\n");
	connection->stream_state = STREAM_STATE_PLAYING;
	decoder->stream_started();
	// audio stream is started when buffer reaches minimal level
	break;
    
    case A2DP_SUBEVENT_STREAM_SUSPENDED:
	printf("A2DP  Sink      : Stream paused\n");
	connection->stream_state = STREAM_STATE_PAUSED;
	decoder->pause();
	break;
    
    case A2DP_SUBEVENT_STREAM_RELEASED:
	printf("A2DP  Sink      : Stream released\n");
	connection->stream_state = STREAM_STATE_CLOSED;
	decoder->media_close();
	break;
    
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
	printf("A2DP  Sink      : Signaling connection released\n");
	connection->a2dp_cid = 0;
	decoder->media_close();
	break;
    
    default:
printf("a2dp: %02x\n", packet[2]);
	break;
    }
}

void A2DPSink::media_packet_handler(uint8_t seid, uint8_t *packet, uint16_t size) {
    decoder->packet_handler(seid, packet, size);
}
