#pragma once

#define P2P_CHAT_BUFFER_LEN 32
#define P2P_GAMESYNC_WAIT 900
#define P2P_GAMECB_WAIT 200
#define P2P_VERSION "v0r6 (" __DATE__ ")"

void p2p_chat_callback(char * nick, char * msg);
bool p2p_add_delay_callback();
void p2p_game_callback(char * game, int playerno, int maxplayers);
void p2p_end_game_callback();
void p2p_client_dropped_callback(char * nick, int playerno);
void __cdecl p2p_core_debug(char * arg_0, ...);

bool p2p_core_cleanup();
bool p2p_core_initialize(bool host, int port, char * appname, char * gamename, char * username);
bool p2p_core_connect(char * ip, int port);
void p2p_print_core_status();
void p2p_retransmit();
void p2p_drop_game();
void p2p_set_ready(bool bx);
void p2p_ping();
void p2p_send_chat(char * xxx);
bool p2p_disconnect();
void p2p_step();
bool p2p_is_connected();
void p2p_fodipp();
void p2p_fodipp_callback(char * host);

int p2p_modify_play_values(void *values, int size);
int p2p_core_get_port();
void p2p_hosted_game_callback(char * game);
void p2p_ping_callback(int PING);
