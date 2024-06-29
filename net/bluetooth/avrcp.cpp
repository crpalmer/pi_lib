#include "pi.h"
#include "bluetooth/avrcp.h"
#include "avrcp-connection.h"

static class AVRCP *global_avrcp;

static void C_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    assert(global_avrcp);
    global_avrcp->packet_handler(packet_type, channel, packet, size);
}

static void C_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    assert(global_avrcp);
    global_avrcp->controller_packet_handler(packet_type, channel, packet, size);
}

static void C_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    assert(global_avrcp);
    global_avrcp->target_packet_handler(packet_type, channel, packet, size);
}

AVRCP::AVRCP() {
    assert(! global_avrcp);
    global_avrcp = this;

    avrcp_init();
    avrcp_controller_init();
    avrcp_target_init();

    avrcp_register_packet_handler(C_packet_handler);
    avrcp_controller_register_packet_handler(C_controller_packet_handler);
    avrcp_target_register_packet_handler(C_target_packet_handler);

    connection = new AVRCPConnection();
}

void AVRCP::packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;
    switch (packet[2]){
    case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED: connection->established(packet); break;
    case AVRCP_SUBEVENT_CONNECTION_RELEASED: connection->released(packet); break;
    default: break;
    }
}

void AVRCP::controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    // helper to print c strings
    uint8_t avrcp_subevent_value[256];
    uint8_t event_id;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;
    if (! connection->is_established()) return;

    memset(avrcp_subevent_value, 0, sizeof(avrcp_subevent_value));
    switch (packet[2]){
    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID:
	connection->add_notification(avrcp_subevent_get_capability_event_id_get_event_id(packet));
	break;

    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID_DONE:
	connection->dump_notifications();
	connection->enable_notification(AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
	connection->enable_notification(AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
	connection->enable_notification(AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
	connection->enable_notification(AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
	connection->enable_notification(AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_STATE:
	event_id = (avrcp_notification_event_id_t) avrcp_subevent_notification_state_get_event_id(packet);
	printf("AVRCP Controller: %s notification registered\n", avrcp_notification2str((avrcp_notification_event_id_t) event_id));
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_POS_CHANGED:
	printf("AVRCP Controller: Playback position changed, position %d ms\n", (unsigned int) avrcp_subevent_notification_playback_pos_changed_get_playback_position_ms(packet));
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_STATUS_CHANGED:
	printf("AVRCP Controller: Playback status changed %s\n", avrcp_play_status2str(avrcp_subevent_notification_playback_status_changed_get_play_status(packet)));
	playback_status = (avrcp_playback_status_t) avrcp_subevent_notification_playback_status_changed_get_play_status(packet);
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_NOW_PLAYING_CONTENT_CHANGED:
	printf("AVRCP Controller: Playing content changed\n");
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_TRACK_CHANGED:
	printf("AVRCP Controller: Track changed\n");
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_AVAILABLE_PLAYERS_CHANGED:
	printf("AVRCP Controller: Available Players Changed\n");
	break;

    case AVRCP_SUBEVENT_SHUFFLE_AND_REPEAT_MODE:{
	uint8_t shuffle_mode = avrcp_subevent_shuffle_and_repeat_mode_get_shuffle_mode(packet);
	uint8_t repeat_mode  = avrcp_subevent_shuffle_and_repeat_mode_get_repeat_mode(packet);
	printf("AVRCP Controller: %s, %s\n", avrcp_shuffle2str(shuffle_mode), avrcp_repeat2str(repeat_mode));
	break;
    }

    case AVRCP_SUBEVENT_NOW_PLAYING_TRACK_INFO:
	printf("AVRCP Controller: Track %d\n", avrcp_subevent_now_playing_track_info_get_track(packet));
	break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TOTAL_TRACKS_INFO:
	printf("AVRCP Controller: Total Tracks %d\n", avrcp_subevent_now_playing_total_tracks_info_get_total_tracks(packet));
	break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TITLE_INFO:
	if (avrcp_subevent_now_playing_title_info_get_value_len(packet) > 0){
	    memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_title_info_get_value(packet), avrcp_subevent_now_playing_title_info_get_value_len(packet));
	    printf("AVRCP Controller: Title %s\n", avrcp_subevent_value);
	}  
	break;

    case AVRCP_SUBEVENT_NOW_PLAYING_ARTIST_INFO:
	if (avrcp_subevent_now_playing_artist_info_get_value_len(packet) > 0){
	    memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_artist_info_get_value(packet), avrcp_subevent_now_playing_artist_info_get_value_len(packet));
	    printf("AVRCP Controller: Artist %s\n", avrcp_subevent_value);
	}  
	break;
    
    case AVRCP_SUBEVENT_NOW_PLAYING_ALBUM_INFO:
	if (avrcp_subevent_now_playing_album_info_get_value_len(packet) > 0){
	    memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_album_info_get_value(packet), avrcp_subevent_now_playing_album_info_get_value_len(packet));
	    printf("AVRCP Controller: Album %s\n", avrcp_subevent_value);
	}  
	break;
    
    case AVRCP_SUBEVENT_NOW_PLAYING_GENRE_INFO:
	if (avrcp_subevent_now_playing_genre_info_get_value_len(packet) > 0){
	    memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_genre_info_get_value(packet), avrcp_subevent_now_playing_genre_info_get_value_len(packet));
	    printf("AVRCP Controller: Genre %s\n", avrcp_subevent_value);
	}  
	break;

    case AVRCP_SUBEVENT_PLAY_STATUS:
	printf("AVRCP Controller: Song length %d ms, Song position %d ms, Play status %s\n", 
	    (int) avrcp_subevent_play_status_get_song_length(packet), 
	    (int) avrcp_subevent_play_status_get_song_position(packet),
	    avrcp_play_status2str(avrcp_subevent_play_status_get_play_status(packet)));
	break;
    
    case AVRCP_SUBEVENT_OPERATION_COMPLETE:
	printf("AVRCP Controller: %s complete\n", avrcp_operation2str(avrcp_subevent_operation_complete_get_operation_id(packet)));
	break;
    
    case AVRCP_SUBEVENT_OPERATION_START:
	printf("AVRCP Controller: %s start\n", avrcp_operation2str(avrcp_subevent_operation_start_get_operation_id(packet)));
	break;
   
    case AVRCP_SUBEVENT_NOTIFICATION_EVENT_TRACK_REACHED_END:
	printf("AVRCP Controller: Track reached end\n");
	break;

    case AVRCP_SUBEVENT_PLAYER_APPLICATION_VALUE_RESPONSE:
	printf("AVRCP Controller: Set Player App Value %s\n", avrcp_ctype2str(avrcp_subevent_player_application_value_response_get_command_type(packet)));
	break;

    default:
	break;
    }
}

void AVRCP::target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;
    
    uint8_t volume;
    char const * button_state;
    avrcp_operation_id_t operation_id;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_PLAY_STATUS_QUERY:
	// TODO
	// status = avrcp_target_play_status(media_tracker.avrcp_cid, play_info.song_length_ms, play_info.song_position_ms, play_info.status);
	printf("avrcp-target: Play status query not yet handled.\n");
	break;

    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
	volume = avrcp_subevent_notification_volume_changed_get_absolute_volume(packet);
	volume_percentage = volume * 100 / 127;
	printf("AVRCP Target    : Volume set to %d%% (%d)\n", volume_percentage, volume);
	on_volume_changed(volume);
	break;
    
    case AVRCP_SUBEVENT_OPERATION:
	operation_id = (avrcp_operation_id_t) avrcp_subevent_operation_get_operation_id(packet);
	button_state = avrcp_subevent_operation_get_button_pressed(packet) > 0 ? "PRESS" : "RELEASE";
	printf("AVRCP Target: operation %s (%s)\n", avrcp_operation2str(operation_id), button_state);
	switch (operation_id) {
	case AVRCP_OPERATION_ID_VOLUME_UP:   on_button_pressed(AVRCP_BUTTON_VOLUME_UP); break;
	case AVRCP_OPERATION_ID_VOLUME_DOWN: on_button_pressed(AVRCP_BUTTON_VOLUME_DOWN); break;
	case AVRCP_OPERATION_ID_PLAY:        on_button_pressed(AVRCP_BUTTON_PLAY); break;
	case AVRCP_OPERATION_ID_PAUSE:       on_button_pressed(AVRCP_BUTTON_PAUSE); break;
	case AVRCP_OPERATION_ID_STOP:        on_button_pressed(AVRCP_BUTTON_STOP); break;
	default: break;
	}
	break;

    default:
	printf("AVRCP Target    : Event 0x%02x is not parsed\n", packet[2]);
	break;
    }
}

uint8_t AVRCP::connect(bd_addr_t addr) {
    if (! connection->is_established()) {
	connection->connect(addr);
    }
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::disconnect() {
    if (connection->is_established()) return connection->disconnect();
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::volume_up() {
printf("volume UP for %d\n", connection->get_cid());
    if (connection->is_established()) return avrcp_controller_volume_up(connection->get_cid());
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::volume_down() {
printf("volume DOWN for %d\n", connection->get_cid());
    if (connection->is_established()) return avrcp_controller_volume_down(connection->get_cid());
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::set_volume(int volume) {
printf("set volume %d for %d\n", volume, connection->get_cid());
    if (connection->is_established()) return avrcp_controller_set_absolute_volume(connection->get_cid(), volume);
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::set_now_playing_info(const avrcp_track_t *track, uint16_t n_tracks) {
    if (connection->is_established()) {
	return avrcp_target_set_now_playing_info(connection->get_cid(), track, n_tracks);
    }
    return ERROR_CODE_SUCCESS;
}

uint8_t AVRCP::set_playback_status(avrcp_playback_status_t status) {
    if (connection->is_established()) {
        bool ret = avrcp_target_set_playback_status(connection->get_cid(), AVRCP_PLAYBACK_STATUS_PLAYING);
	if (ret == ERROR_CODE_SUCCESS) playback_status = status;
	return ret;
    }
    return ERROR_CODE_SUCCESS;
}
