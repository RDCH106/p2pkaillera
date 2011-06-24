// Updated by Ownasaurus in June 2010

#include "kaillera_core.h"
#include "k_message.h"
#include "../common/k_framecache.h"
#include "Wincon.h"

// Owna's fix to reduce the server timeout period from 15 seconds to 2 seconds
#define KAILLERA_CONNECTION_RESP_MAX_DELAY /*15000*/ 2000

#define KAILLERA_LOGIN_RESP_MAX_DELAY 10000 // not used at all?
#define KAILLERA_GAME_LOSTCON_TIMEOUT 10000 // not used at all?
#define KAILLERA_TIMEOUT_RESET 60000
#define KAILLERA_TIMEOUT_NETSYNC 120000
#define KAILLERA_TIMEOUT_NETSYNC_RETR_INTERVAL 5000

extern int PACKETLOSSCOUNT;

typedef struct {
	k_message * connection;
	int frameno;
	int dframeno;
	int throughput;
	int pps;
	int REQDATALEN;
	int DATALEN;
	k_framecache USERDATA;
	int playerno;
	int PORT;
	char IP[128];
	char APP[128];
	//char GAME[128]; REQ
	char GAME[150];
	char USERNAME[32];
	char conset;
	int USERSTAT; //0 = dc; 1 = connected; 2 = logged in; 3 = in game
	int PLAYERSTAT; //0 = idle; 1 = netsync; 2 = playing
	bool owner;
	bool game_id_requested;
	unsigned int game_id;
	bool user_id_requested;
	bool leave_game_requested;
	unsigned short user_id;
	unsigned int tmoutrsttime;
} KAILLERAC_;

KAILLERAC_ KAILLERAC;

//void __cdecl kprintf(char * arg_0, ...);

bool kaillera_core_initialized = false;
bool spoofing = false;
int spoofPing = -1;

void p2p_InitializeTime();
int p2p_GetTime();
void p2p_AdjustTime(int TIME);

// Created for simplicity
// Crafts the packet: 0123 and sends it once to the server
// Please note that the server will send 4 pings and so
// the client will have to pong 4 times.
void SendPong()
{
	k_instruction pong;
	pong.type = USERPONG;
	int x = 0;
	while(x<4)
		pong.store_int(x++);
	KAILLERAC.connection->send_instruction(&pong);
}

bool kaillera_is_connected(){
	return KAILLERAC.USERSTAT > 0;
}
int kaillera_get_frames_count(){
	return KAILLERAC.frameno;
}
int kaillera_get_delay(){
	return KAILLERAC.dframeno;
}

bool kaillera_core_cleanup(){
	kprintf(__FILE__ ":%i", __LINE__);
	if (kaillera_core_initialized){
		
		if (KAILLERAC.connection)
			delete KAILLERAC.connection;
		
		KAILLERAC.connection = 0;
		
		
		
		
		kaillera_core_initialized = false;
		
	} return true;
	
}


bool kaillera_disconnect(char * quitmsg){
	if (KAILLERAC.USERSTAT >= 1) {
		if (KAILLERAC.USERSTAT > 1) {
			int len = strlen(quitmsg);
			k_instruction ls;
			ls.type = USERLEAV;
			ls.store_short(-1);
			ls.store_string(quitmsg);
			KAILLERAC.connection->send_instruction(&ls);
			KAILLERAC.USERSTAT = 0;
			KAILLERAC.PLAYERSTAT = -1;
			return true;
		}
	} else {
		KAILLERAC.USERSTAT = 0;
		KAILLERAC.PLAYERSTAT = -1;
		return true;
	}
	return true;
}


bool kaillera_core_initialize(int port, char * appname, char * username, char connection_setting){
	p2p_InitializeTime();
	
	if (kaillera_core_initialized) kaillera_core_cleanup();
	
	KAILLERAC.PORT = port;
	KAILLERAC.conset = connection_setting;
	
	strncpy(KAILLERAC.APP, appname, 128);
	strncpy(KAILLERAC.USERNAME, username, 32);
	
	KAILLERAC.connection = new k_message;
	if (!KAILLERAC.connection->initialize(KAILLERAC.PORT)){
		return false;
	}
	KAILLERAC.PORT = KAILLERAC.connection->get_port();
	
	kaillera_core_initialized = true;
	
	return true;
}

int kaillera_core_get_port(){
	return KAILLERAC.PORT;
}

bool kaillera_core_connect(char * ip, int port){
	
	kaillera_core_debug("Connecting to %s:%i", ip, port);
	
	strncpy(KAILLERAC.IP, ip, 128);
	
	k_socket ksock;
	ksock.initialize(0, 2048);
	
	if (ksock.set_address(ip, port)){
		ksock.send("HELLO0.83", 10); // Client's hello message
		
		DWORD tout = p2p_GetTime();
		
		while ((!k_socket::check_sockets(0, 100) || !ksock.has_data()) && p2p_GetTime() - tout < KAILLERA_CONNECTION_RESP_MAX_DELAY);
		
		if (ksock.has_data()) {

			char srsp[256];
			int srspl = 256;
			sockaddr_in addr;
			
			kaillera_core_debug("server replied");
			
			if (ksock.check_recv(srsp, &srspl, false, &addr)) {
				if (strncmp("HELLOD00D", srsp, 9) == 0) { // Server's hello reply
					
					kaillera_core_debug("logging in");
					
					addr.sin_port = htons(atoi(srsp+9));
					KAILLERAC.connection->set_addr(&addr);
					KAILLERAC.USERSTAT = 1;
					KAILLERAC.PLAYERSTAT = -1;
					
					k_instruction ki;
					ki.type = USERLOGN;
					ki.store_string(KAILLERAC.APP);
					ki.store_char(KAILLERAC.conset);
					/* Old code to append "--FakePing" to the username
					if(spoofing)
					{
						//char tempbuff[32];
						//strcpy(tempbuff,KAILLERAC.USERNAME);
						//snprintf(KAILLERAC.USERNAME,32,"%s--FakePing",tempbuff);
					}*/
					ki.set_username(KAILLERAC.USERNAME);
					
					KAILLERAC.connection->send_instruction(&ki);

					// Credits to Fireblaster for showing me the code that taught me
					// I needed to pong 4 times.
					// If you take a minute to think about it, this ping spoofing code
					// is simple and makes perfect sense.
					if(spoofing)
					{
						Sleep(spoofPing);
						SendPong();
						Sleep(spoofPing);
						SendPong();
						Sleep(spoofPing);
						SendPong();
						Sleep(spoofPing);
						SendPong();
					}

					return true;
				}
			} else {
				if (strcmp("TOO", srsp)==0) { // Too many users
					kaillera_error_callback("Server is full");
				} else {
					kaillera_core_debug("server protocol mismatch or other error");
					//protocol different or unrecognized protocol
				}
			}

		}
		
		return false;
	}
	return false;
}

void kaillera_ProcessGeneralInstruction(k_instruction * ki) {
	switch (ki->type) {
	case USERJOIN: // user joins the server
		{
			unsigned short id = ki->load_short();
			int ping = ki->load_int();
			int conn = ki->load_char();
			kaillera_user_join_callback(ki->user, ping, id, conn);
			break;
		}
	case USERLEAV:
		{
			unsigned short id = ki->load_short();
			char quitmsg[128];
			ki->load_str(quitmsg, 128);
			kaillera_user_leave_callback(ki->user, quitmsg, id);
			break;
		}
	case GAMEMAKE:
		{
			//kaillera_core_debug("GAMEMAKE");

			char gname[128];
			ki->load_str(gname, 128);
			char emulator[128];
			ki->load_str(emulator, 128);
			unsigned int id = ki->load_int();
			
			kaillera_game_create_callback(gname, id, emulator, ki->user);
			
			if (KAILLERAC.game_id_requested) {
				KAILLERAC.game_id = id;
				if (strcmp(gname, KAILLERAC.GAME)==0 && strcmp(emulator, KAILLERAC.APP)==0 && strcmp(ki->user, KAILLERAC.USERNAME)==0) {
					KAILLERAC.game_id_requested = false;
					KAILLERAC.user_id_requested = true;
					
					KAILLERAC.USERSTAT = 3;
					KAILLERAC.PLAYERSTAT = 0;
					
					kaillera_user_game_create_callback();
				}
			}
			break;
		}
	case INSTRUCTION_GAMESTAT:
		{
			unsigned int id = ki->load_int();
			char status = ki->load_char();
			int players = ki->load_char();
			int maxplayers = ki->load_char();
			kaillera_game_status_change_callback(id, status, players, maxplayers);
			break;
		}
	case INSTRUCTION_GAMESHUT:
		{
			unsigned int id = ki->load_int();
			kaillera_game_close_callback(id);
			if (id==KAILLERAC.game_id){
				kaillera_user_game_closed_callback();
				kaillera_end_game_callback();
				KAILLERAC.USERSTAT = 2;
				KAILLERAC.PLAYERSTAT = -1;
			}
			break;
		}
	case GAMEBEGN:
		{
			//kaillera_core_debug("GAMEBEGN");
			//KAILLERAC.USERSTAT = 3;
			KAILLERAC.PLAYERSTAT = 1;
			KAILLERAC.throughput = ki->load_short();
			KAILLERAC.dframeno = (KAILLERAC.throughput + 1) * KAILLERAC.conset - 1;
			KAILLERAC.playerno = ki->load_char();					
			int players = ki->load_char();
			kaillera_game_callback(0, KAILLERAC.playerno, players);
			kaillera_core_debug("Server says: delay is %i frames", KAILLERAC.dframeno);
			break;
		}
	case GAMRSLST:
		{
			//kaillera_core_debug("GAMRSLST");

			if (!KAILLERAC.owner)
				kaillera_user_game_joined_callback();

			KAILLERAC.USERSTAT = 3;
			KAILLERAC.PLAYERSTAT = 0;
			int numplayers = ki->load_int();
			for (int x=0; x < numplayers; x++) {
				char name[32];
				ki->load_str(name, 32);
				int ping = ki->load_int();
				unsigned short id = ki->load_short();
				int conn = ki->load_char();
				kaillera_player_add_callback(name, ping, id, conn);
			}
			break;
		}
	case GAMRJOIN:
		{
			//kaillera_core_debug("GAMRJOIN");
			ki->load_int();
			char username[32];
			ki->load_str(username, 32);					
			int ping = ki->load_int();					
			unsigned short uid = ki->load_short();
			char connset = ki->load_char();

			kaillera_player_joined_callback(username, ping, uid, connset);

			if (KAILLERAC.user_id_requested) {
				KAILLERAC.user_id = uid;
				if (KAILLERAC.conset == connset && strcmp(username, KAILLERAC.USERNAME)==0) {
					KAILLERAC.user_id_requested = false;
					KAILLERAC.USERSTAT = 3;
					KAILLERAC.PLAYERSTAT = 0;
				}
			}
			break;
		}
	case GAMRLEAV:
		{
			unsigned short id;
			kaillera_player_left_callback(ki->user, id = ki->load_short());
			if (id==KAILLERAC.user_id && !KAILLERAC.leave_game_requested) {
				kaillera_user_kicked_callback();
				KAILLERAC.USERSTAT = 2;
				KAILLERAC.PLAYERSTAT = -1;
			}
			break;
		}
	case GAMRDROP:
		{
			int gdpl = ki->load_char();
			if (KAILLERAC.playerno == gdpl) {
				KAILLERAC.USERSTAT = 3;
				KAILLERAC.PLAYERSTAT = 0;
				kaillera_end_game_callback();
			}
			kaillera_player_dropped_callback(ki->user, gdpl);
			break;
		}
	case PARTCHAT:
		{
		kaillera_chat_callback(ki->user, ki->buffer);
		}
		break;
	case GAMECHAT:
		{
			kaillera_game_chat_callback(ki->user, ki->buffer);
		}
		break;
	case MOTDLINE:
		kaillera_motd_callback(ki->user, ki->buffer);
		break;				
	case LONGSUCC:
		{
			KAILLERAC.USERSTAT = KAILLERAC.USERSTAT == 1? 2:KAILLERAC.USERSTAT;
			int gameZ;
			int userZ;
			userZ = ki->load_int();
			gameZ = ki->load_int();
			int x;
			//users
			for(x=0;x<userZ; x++) {
				char name[32];
				ki->load_str(name, 32);
				int ping = ki->load_int();
				int status = ki->load_char();
				unsigned short id = ki->load_short();
				int conn = ki->load_char();
				kaillera_user_add_callback(name, ping, status, id, conn);
			}
			for(x=0;x<gameZ; x++) {
				char gname[128];
				ki->load_str(gname, 128);
				unsigned int id = ki->load_int();
				char emulator[128];
				ki->load_str(emulator, 128);
				char owner[32];
				ki->load_str(owner, 32);
				char users[20];
				ki->load_str(users, 20);
				int status = ki->load_char();
				kaillera_game_add_callback(gname, id, emulator, owner, users, status);
			}
			KAILLERAC.tmoutrsttime = p2p_GetTime();
			break;
		}
	case SERVPING:
		{
			if(!spoofing) // norma pong response only if not spoofing
				SendPong();
			break;
		}
	case LOGNSTAT:
		{
			ki->load_short();
			char lsmsg[128];
			ki->load_str(lsmsg, 128);
			kaillera_login_stat_callback(lsmsg);
			break;
		}
	default:
		return;
	}
}

/////////////////////////////////////////////////

void kaillera_step(){
	//kprintf(__FILE__ ":%i", __LINE__);
	k_socket::check_sockets(0,200);
	while (KAILLERAC.connection && KAILLERAC.connection->has_data()){
		//kprintf(__FILE__ ":%i", __LINE__);
		k_instruction ki;
		sockaddr_in saddr;
		if (KAILLERAC.connection->receive_instruction(&ki, false, &saddr)){
			kaillera_ProcessGeneralInstruction(&ki);
		}
	}
	if (KAILLERAC.USERSTAT > 1 && p2p_GetTime() - KAILLERAC.tmoutrsttime > KAILLERA_TIMEOUT_RESET) {
		KAILLERAC.tmoutrsttime = p2p_GetTime();
		k_instruction trst;
		trst.type = TMOUTRST;
		KAILLERAC.connection->send_instruction(&trst);
		//kaillera_core_debug("TMOUTRST");
	}
}

void kaillera_chat_send(char * text) {
	if (KAILLERAC.USERSTAT > 1) {
		k_instruction sgc;
		sgc.type = PARTCHAT;
		sgc.store_string(text);
		KAILLERAC.connection->send_instruction(&sgc);
	}

}

void kaillera_game_chat_send(char * text) {
	if (KAILLERAC.USERSTAT > 1) {
		k_instruction sgc;
		sgc.type = GAMECHAT;
		sgc.store_string(text);
		KAILLERAC.connection->send_instruction(&sgc);
	}
}


void kaillera_kick_user (unsigned short id) {
	if (KAILLERAC.USERSTAT > 1) {
		k_instruction sgc;
		sgc.type = GAMRKICK;
		sgc.store_short(id);
		KAILLERAC.connection->send_instruction(&sgc);
	}
}

void kaillera_join_game(unsigned int id){
	KAILLERAC.game_id_requested = false;
	KAILLERAC.game_id = id;
	KAILLERAC.leave_game_requested = false;
	
	KAILLERAC.owner = false;

	k_instruction jog;
	jog.type = GAMRJOIN;
	jog.store_int(id);
	jog.store_char(0);
	jog.store_int(0);
	jog.store_short(-1);
	jog.store_char(KAILLERAC.conset);
	
	KAILLERAC.connection->send_instruction(&jog);;
}
void kaillera_create_game(char * name) {
	KAILLERAC.GAME[150] = '\0'; //rs
	strncpy(KAILLERAC.GAME, name, 149); //rs
	KAILLERAC.game_id_requested = true;
	k_instruction cg;
	cg.type = GAMEMAKE;
	cg.store_string(name);
	cg.store_char(0);
	cg.store_int(-1);

	KAILLERAC.owner = true;

	KAILLERAC.connection->send_instruction(&cg);;
}

void kaillera_leave_game (){
	KAILLERAC.leave_game_requested = true;
	k_instruction lg;
	lg.type = INSTRUCTION_GAMRLEAV;
	lg.store_short(-1);
	KAILLERAC.connection->send_instruction(&lg);
	kaillera_user_game_closed_callback();
}

void kaillera_start_game() {
	KAILLERAC.leave_game_requested = false;
	k_instruction kx;
	kx.type = INSTRUCTION_GAMEBEGN;
	kx.store_int(-1);
	KAILLERAC.connection->send_instruction(&kx);
}
void kaillera_game_drop(){
	KAILLERAC.leave_game_requested = false;
	k_instruction kx;
	kx.type=GAMRDROP;
	kx.store_char(0);
	KAILLERAC.connection->send_instruction(&kx);
	KAILLERAC.PLAYERSTAT = 0;
	kaillera_end_game_callback();
}

void kaillera_end_game(){
	k_instruction kx;
	kx.type = GAMRDROP;
	kx.store_char(0);
	KAILLERAC.connection->send_instruction(&kx);
	kaillera_end_game_callback();
}


oslist<char*, 256> kaillera_incoming_data_cache;
oslist<char*, 256> kaillera_gv_queue;

int kaillera_modify_play_values (void * values, int size) {
	//kaillera_core_debug(" KMPV f=%i d=%i", KAILLERAC.frameno, KAILLERAC.dframeno );
	//kaillera_core_debug("XX %i     %i", KAILLERAC.connection->in_cache.length, KAILLERAC.connection->last_cached_instruction);
	
	if (KAILLERAC.USERSTAT > 2 && KAILLERAC.PLAYERSTAT > 0) {
		if (KAILLERAC.PLAYERSTAT == 2) {
			
			KAILLERAC.USERDATA.put_data(values, size);

			//kaillera_core_debug(" PUT DATA  %i/%i", KAILLERAC.USERDATA.pos, KAILLERAC.REQDATALEN);

			if (KAILLERAC.USERDATA.pos >= KAILLERAC.REQDATALEN){
				
				//outgoing caching goes here

				k_instruction kx;
				kx.type = GAMEDATA;
				kx.store_short(KAILLERAC.REQDATALEN);
				kx.store_bytes(KAILLERAC.USERDATA.buffer, KAILLERAC.REQDATALEN);
				KAILLERAC.connection->send_instruction(&kx);

				//kaillera_core_debug("send on %i, l = %i D: %08X", KAILLERAC.frameno, KAILLERAC.USERDATA.pos, *((DWORD*)KAILLERAC.USERDATA.buffer));

				KAILLERAC.USERDATA.reset();
			}

			int pix = 0;
			int ttx = 0;

			do {
				if (KAILLERAC.connection->has_data() || (k_socket::check_sockets(0,pix) && KAILLERAC.connection->has_data())){
					k_instruction ki;
					sockaddr_in saddr;
					
					if (KAILLERAC.connection->receive_instruction(&ki, false, &saddr)){
						if (ki.type==GAMEDATA) {

							short len = ki.load_short();
							char * kd;

							if (kaillera_incoming_data_cache.length == 256){
								//MessageBox(0,0,0,0);

								kd = kaillera_incoming_data_cache[0];
								kaillera_incoming_data_cache.removei(0);

								if (*((short*)kd) != len)
									realloc(kd, len + 2);
								////////////////
								//free(kaillera_incoming_data_cache[0]);
								//kaillera_incoming_data_cache.removei(0);
								//kd = (char*)malloc(len + 2);								
								//MessageBox(0,0,0,0);
							} else {
								kd = (char*)malloc(len+2);
							}

							*((short*)kd) = len;

							char * kdd = kd + 2;
							ki.load_bytes(kdd, len);

							

							//kaillera_core_debug("GAMEDATA %08X  l = %i", *((DWORD*)kdd), *((short*)kd));
							kaillera_incoming_data_cache.add(kd);
							int ilen = len / KAILLERAC.conset;

							//kaillera_core_debug("CACHESI = %i, PLC = %i, INCL = %i", kaillera_incoming_data_cache.length, PACKETLOSSCOUNT, KAILLERAC.connection->in_cache.length);

							for (int x = 0; x < len; x+= ilen) {
								char * kds = (char *)malloc(ilen + 2);
								*((short*)kds) = ilen;
								//kaillera_core_debug("GVADD %i, x=%i D: %i", x, ilen, *((DWORD*)(kds+2)));
								memcpy(kds+2, kdd + x, ilen);
								kaillera_gv_queue.add(kds);
							}

						} else if (ki.type==GAMCDATA) {

							//kaillera_core_debug("GAMCDATA");

							int index = ki.load_char();

							char * kd = kaillera_incoming_data_cache[index];

							short len = *((short*)kd);

							char * kdd = kd + 2;
							int ilen = len / KAILLERAC.conset;
							for (int x = 0; x < len; x+= ilen) {
								char * kds = (char *)malloc(ilen + 2);
								*((short*)kds) = ilen;
								//kaillera_core_debug("GVADD %i, x=%i D: %i", x, ilen, *((DWORD*)(kds+2)));
								memcpy(kds+2, kdd + x, ilen);
								kaillera_gv_queue.add(kds);
							}
						} else {
							kaillera_ProcessGeneralInstruction(&ki);
						}
					}
				}

				if (kaillera_gv_queue.length <= 0) {
					if (KAILLERAC.frameno < KAILLERAC.dframeno) {
						KAILLERAC.frameno++;
						return 0;
					}
					if (ttx == 0) {
						pix++;
						ttx = p2p_GetTime();
					} else {
						DWORD tttx = p2p_GetTime();
						if (tttx - ttx > 1000) {
							
							//kaillera_core_debug("PIX=%i", pix);

							ttx = tttx;
							KAILLERAC.connection->resend_message(5);
							pix++;
							///*
							if (pix == 100){
								kaillera_core_debug("Lost Connection");
								kaillera_game_drop();
								return -1;
							}//*/
						}
					}
					//kaillera_core_debug(__FILE__ ": %i kaillera_gv_queue.length <= 0, pix %i ttc %i", __LINE__, pix, ttx);

				}
			} while ((kaillera_gv_queue.length <= 0) && KAILLERAC.PLAYERSTAT > 1);

			if (KAILLERAC.PLAYERSTAT > 1 && kaillera_gv_queue.length > 0) {
				char * kd = kaillera_gv_queue[0];
				kaillera_gv_queue.removei(0);
				int l = *((short*)kd);
				memcpy(values, kd+2, l);
				free(kd);
				KAILLERAC.frameno++;
				return l;
			} else {
				return -1;
			}
		} else {
			KAILLERAC.DATALEN = size;
			KAILLERAC.REQDATALEN = size * KAILLERAC.conset;
			KAILLERAC.USERDATA.reset();
			KAILLERAC.frameno = 0;
			kaillera_incoming_data_cache.clear();
			kaillera_gv_queue.clear();
			while (kaillera_incoming_data_cache.length>0) {
				free(kaillera_incoming_data_cache[0]);
				kaillera_incoming_data_cache.removei(0);
			}
			k_instruction kx;
			kx.type = GAMRSRDY;
			DWORD ti = p2p_GetTime();
			DWORD tit = KAILLERA_TIMEOUT_NETSYNC_RETR_INTERVAL + ti;
			KAILLERAC.connection->send_instruction(&kx);
			while (KAILLERAC.PLAYERSTAT == 1) {
				if (k_socket::check_sockets(0,100) && KAILLERAC.connection->has_data()){
					k_instruction ki;
					sockaddr_in saddr;
					if (KAILLERAC.connection->receive_instruction(&ki, false, &saddr)){
						if (ki.type== GAMRSRDY) {
							KAILLERAC.PLAYERSTAT = 2;
							kaillera_core_debug("All players are ready");
							break;
						} else {
							kaillera_ProcessGeneralInstruction(&ki);
						}
					}
				}
				int tx;
				if ((tx = p2p_GetTime() - ti) > KAILLERA_TIMEOUT_NETSYNC) {
					KAILLERAC.PLAYERSTAT = 0;
					break;
				} else {
					kaillera_game_netsync_wait_callback(KAILLERA_TIMEOUT_NETSYNC - tx);
					if (tit <= ti) {
						tit = KAILLERA_TIMEOUT_NETSYNC_RETR_INTERVAL + ti;
						KAILLERAC.connection->resend_message(5);
					}
				}
			}
			return kaillera_modify_play_values(values, size);
		}
	}
	return -1;
}



int kaillera_ping_server(char * host, int port, int limit) {
	k_socket psk;
	psk.initialize(0);
	psk.set_address(host, port);
	k_socket::check_sockets(0,0);
	DWORD ti = GetTickCount();
	psk.send("PING", 5);
	
	while (!psk.has_data() && GetTickCount() - ti < limit) {
		k_socket::check_sockets(0,10);
	}
	
	return GetTickCount() - ti;
}
