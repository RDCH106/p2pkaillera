#include "p2p_core.h"

#include "p2p_message.h"
#include "../common/k_framecache.h"

#include "../delay.h"

typedef struct {
	int crframeno;
	bool local;
	char msg[251];
}p2p_chatstruct;

oslist<p2p_chatstruct, P2P_CHAT_BUFFER_LEN> p2p_chat_cache;

struct P2PCORESTAT {
	p2p_message * connection;
	bool HOST;
	int PORT;
	char IP[128];
	char APP[128];
	//char GAME[128];
	char GAME[150];
	bool USERREADY;
	bool PEERREADY;
	TCHAR USERNAME[32];
	TCHAR PEERNAME[32];
	int ping;
	int pingc;
	bool CONNECTED;
	bool USERLOADED;
	bool PEERLOADED;
	int status;//0 = NC; 1 = C; 2 = GL; 3 = NS; 4 = PL
	int throughput;
	int frameno;
	int crframeno;
	int DATALEN;
	k_framecache USERDATA;
	k_framecache PEERDATA;
} P2PCORE;

void __cdecl kprintf(char * arg_0, ...);

bool p2p_core_initialized = false;
//===========================================================
//===========================================================
//===========================================================

//comment next line to enable tracing
#define TRACE
HFILE traceou;
void TRACE_INIT() {
	#ifndef TRACE
	OFSTRUCT of;
	traceou = OpenFile("trace.txt", &of, OF_WRITE|OF_CREATE);
	#endif
}
void TRACE_TERM() {
#ifndef TRACE
	_lclose(traceou);
#endif
}
void TRACEX(char * file, int LINE) {
	char buf[300];
	wsprintf(buf, "%s -- %i \r\n", file, LINE);
	_lwrite(traceou, buf, strlen(buf)+1);
}
#ifndef TRACE
#define TRACE TRACEX(__FILE__,__LINE__);
#endif



//===========================================================
//===========================================================
//===========================================================
DWORD p2p_initial_time;
#define ADJUST_RATIO_T 1/30
void p2p_InitializeTime(){
	p2p_initial_time = GetTickCount();
}

int p2p_GetTime(){
	return GetTickCount() - p2p_initial_time;
}
void p2p_AdjustTime(int TIME){
	p2p_initial_time += ((p2p_initial_time - TIME) * ADJUST_RATIO_T);
}
//====================================
#define P2P_FODIPP_SERVER "60.242.194.154"
struct {
	bool FODIPP;
	in_addr ip;
}P2PFODIPP;
void p2p_fodipp(){

	if (!P2PCORE.CONNECTED) {
		P2PFODIPP.FODIPP = true;
//		P2PFODIPP.ip.in_addr = inet_addr(P2P_FODIPP_SERVER);
		//p2p_fodipp_callback(host);

	}

	

}


//=====================================================
int p2p_PING_TIME;

bool p2p_is_connected(){
	return P2PCORE.status != 0 && P2PCORE.ping != -1 && P2PCORE.connection;
}


bool p2p_core_cleanup(){
	TRACE_TERM();
	kprintf(__FILE__ ":%i", __LINE__);
	if (p2p_core_initialized){

		kprintf(__FILE__ ":%i", __LINE__);

		if (P2PCORE.connection)
			delete P2PCORE.connection;
		P2PCORE.connection = 0;
		
		kprintf(__FILE__ ":%i", __LINE__);
		
		while (p2p_chat_cache.length>0){
			//delete p2p_chat_cache[0];
			p2p_chat_cache.removei(0);
		}
		

		
		p2p_core_initialized = false;
		
	} return true;

}


bool p2p_disconnect(){
	if (P2PCORE.CONNECTED) {
		if (P2PCORE.status > 1) {
			p2p_core_debug("Cant quit while in game");
			return false;
		} else {
			P2PCORE.connection->send_tinst(EXIT, 0);
			P2PCORE.status = 0;
			P2PCORE.CONNECTED = false;

			return true;
		}
	} else {
		P2PCORE.status = 0;
		return true;
	}
}

bool p2p_core_initialize(bool host, int port, char * appname, char * gamename, char * username){

	TRACE_INIT();

	p2p_InitializeTime();

	if (p2p_core_initialized) p2p_core_cleanup();

	P2PCORE.HOST = host;
	P2PCORE.PORT = port;
	strncpy(P2PCORE.APP, appname, 128); //rs
	strncpy(P2PCORE.USERNAME, username, 32);
	strncpy(P2PCORE.GAME, gamename, 128);

	P2PCORE.connection = new p2p_message;
	if (!P2PCORE.connection->initialize(P2PCORE.PORT)){
		return false;
	}
	P2PCORE.PORT = P2PCORE.connection->get_port();

	TRACE

	P2PCORE.status = 1;
	P2PCORE.ping = -1;
	P2PCORE.throughput = 3;
	P2PCORE.USERREADY = false;
	P2PCORE.PEERREADY = false;
	P2PCORE.connection->dsc = 0;
	p2p_chat_cache.clear();

	p2p_core_initialized = true;

	TRACE
	
	return true;
}

int p2p_core_get_port(){
	return P2PCORE.PORT;
}

bool p2p_core_connect(char * ip, int port){
	strncpy(P2PCORE.IP, ip, 128);
	if (!P2PCORE.HOST && P2PCORE.connection->set_address(ip, port)){
		p2p_instruction contreq(LOGN, LOGN_REQ);
		contreq.store_sstring(P2PCORE.USERNAME);
		contreq.store_string(P2PCORE.APP);
		P2PCORE.connection->send_instruction(&contreq);
		return true;
	}
	return false;
}


void p2p_print_core_status(){
	p2p_core_debug( "P2PCORE{ \r\n"
		"\tHOST: %s \r\n"
		"\tPORT: %i \r\n"
		"\tIP: %s \r\n"
		"\tAPP: %s \r\n"
		"\tGAME: %s \r\n"
		"\tUSERREADY: %s \r\n"
		"\tPEERREADY: %s \r\n"
		"\tUSERNAME: %s \r\n"
		"\tPEERNAME: %s \r\n"
		"\tping: %i \r\n"
		"\tpingc: %i \r\n"
		"\tCONNECTED: %s \r\n"
		"\tUSERLOADED: %s \r\n"
		"\tPEERLOADED: %s \r\n"
		"\tstatus: %i \r\n"
		"\tthroughput: %i \r\n"
		"\tframeno: %i \r\n"
		"\tDATALEN: %i \r\n"
		"\tINCACHE: %i \r\n"
		"\tOUTCACHE: %i \r\n"
		"\tCOMPILE: " __DATE__ " - " __TIME__ "\r\n}"
		, P2PCORE.HOST? "true":"false"
		, P2PCORE.PORT
		, P2PCORE.HOST? "irrelevent" : P2PCORE.IP
		, P2PCORE.APP
		, P2PCORE.GAME
		, P2PCORE.USERREADY? "true":"false"
		, P2PCORE.PEERREADY? "true":"false"
		, P2PCORE.USERNAME
		, P2PCORE.PEERNAME
		, P2PCORE.ping
		, P2PCORE.pingc
		, P2PCORE.CONNECTED? "true":"false"
		, P2PCORE.USERLOADED? "true":"false"
		, P2PCORE.PEERLOADED? "true":"false"
		, P2PCORE.status
		, P2PCORE.throughput
		, P2PCORE.frameno
		, P2PCORE.DATALEN
		, P2PCORE.USERDATA.pos
		, P2PCORE.PEERDATA.pos);
}

void p2p_retransmit(){
	P2PCORE.connection->send_message(P2PCORE.throughput+2);
}

void p2p_drop_game(){
	if (P2PCORE.connection) {
		P2PCORE.connection->send_tinst(DROP, 0);
		P2PCORE.status = 1;
		P2PCORE.USERREADY = false;
		P2PCORE.PEERREADY = false;
		p2p_retransmit();

		p2p_client_dropped_callback(P2PCORE.PEERNAME, P2PCORE.HOST? 2: 1);
		p2p_client_dropped_callback(P2PCORE.USERNAME, P2PCORE.HOST? 1: 2);
		
		p2p_end_game_callback();


	}
}

void p2p_set_ready(bool bx){

	if (P2PCORE.USERREADY != bx) {
		P2PCORE.USERREADY = bx;
		if (P2PCORE.connection && P2PCORE.CONNECTED){
			P2PCORE.connection->send_tinst(PREADY, bx? PREADY_READY:PREADY_NREADY);
		}
		p2p_core_debug("You are marked as %s", bx? "ready":"not ready");
	}
}


void p2p_ping(){
	p2p_instruction kx;
	kx.inst.type = PING;
	kx.inst.flags = PING_PING;
	kx.store_sstring((char*)&P2PCORE);
	P2PCORE.connection->send_instruction(&kx);
	p2p_PING_TIME = p2p_GetTime();
}

void p2p_send_chat(char * xxx){

	p2p_chatstruct * ps = &p2p_chat_cache.items[p2p_chat_cache.length];
	ps->crframeno = P2PCORE.frameno + P2PCORE.throughput + 2;
	
	if (p2p_chat_cache.length == 0)
		P2PCORE.crframeno = ps->crframeno;

	strncpy(ps->msg, xxx, min(251, strlen(xxx)+1));
	ps->local = true;
	p2p_chat_cache.length++;
	
	p2p_instruction kx(CHAT, 0);
	kx.store_int(P2PCORE.crframeno);
	kx.store_vstring(xxx);
	P2PCORE.connection->send_instruction(&kx);

}








inline void p2p_handle_chat_instruction(p2p_instruction * ki){
	p2p_chatstruct * pc = &p2p_chat_cache.items[p2p_chat_cache.length];

	pc->crframeno = ki->load_int();

	pc->local = false;

	ki->load_vstring(pc->msg);

	p2p_chat_cache.length++;

	if (p2p_chat_cache.length>0)
		P2PCORE.crframeno = p2p_chat_cache.items[0].crframeno;
}

bool p2p_WaitForPeerToLoadOrDie(){
	//kprintf("WaitForPeerToLoad:" __FILE__ ":%i", __LINE__);
	
	sockaddr_in saddr;
	
	p2p_core_debug("Muffin loaded, waiting for Donut.");
	
	P2PCORE.USERLOADED = true;
	P2PCORE.PEERLOADED = false;
	
	P2PCORE.connection->send_tinst(LOAD, LOAD_LOADED);
	
	//kprintf(__FILE__ ":%i", __LINE__);
	
	while (!P2PCORE.PEERLOADED && P2PCORE.status == 1){
		if (P2PCORE.connection->has_data() || (k_socket::check_sockets(1,0) && P2PCORE.connection->has_data())){
			p2p_instruction ki;
			//kprintf(__FILE__ ":%i", __LINE__);
			if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)) {
				switch(ki.inst.type){
				case LOAD:
					p2p_core_debug("Donut loaded.");
					P2PCORE.PEERLOADED = true;
					P2PCORE.status = 2;
					
					P2PCORE.connection->send_tinst(LOAD, LOAD_LOADED);
					
					break;
				case CHAT:
					p2p_handle_chat_instruction(&ki);
					break;
				}
			}
		}
	}
	return P2PCORE.status == 2;
}


bool p2p_CalculatePingOrDie(){
#define p2p_PING_TIMES 4
	sockaddr_in saddr;
	//outp("calculating alcohol content in raspberry muffin...");
	if (P2PCORE.HOST) 
		Sleep(250);
	P2PCORE.pingc = 0;
	if (P2PCORE.HOST) {
		p2p_instruction kix(PING, PING_PING);
		kix.store_bytes(&P2PCORE, 64);
		
		P2PCORE.connection->send_tinst(PING, PING_PING);
		p2p_PING_TIME = p2p_GetTime();
	}
	while (P2PCORE.pingc < p2p_PING_TIMES){
		if (P2PCORE.connection->has_data() || (k_socket::check_sockets(1,0) && P2PCORE.connection->has_data())) {
			p2p_instruction ki;
			//kprintf(__FILE__ ":%i", __LINE__);
			if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)) {
				if (ki.inst.type==PING) {
					if (ki.inst.flags==PING_PING){
						//kprintf(__FILE__ ":%i", __LINE__);
						if (P2PCORE.HOST && ++P2PCORE.pingc>=p2p_PING_TIMES) {
							break;
						}
						P2PCORE.connection->send_instruction(&ki);
					} else {
						P2PCORE.ping = ki.load_int();
						P2PCORE.pingc = 99;
					}
				} else if (ki.inst.type == CHAT){
					p2p_handle_chat_instruction(&ki);
				}
			}
		}
	}
	if (P2PCORE.HOST) {
		P2PCORE.ping = (p2p_GetTime() - p2p_PING_TIME)/p2p_PING_TIMES;// * 7/5;
		p2p_instruction kx(PING, PING_ECHO);
		kx.store_int(P2PCORE.ping);
		P2PCORE.connection->send_instruction(&kx);
	}
	return true;
}


bool p2p_SynChronizeClocksOrDie(){
	sockaddr_in saddr;
	
	//p2p_core_debug("calculate no of atoms in a blueberry muffin...");
	
	if (P2PCORE.HOST) {
		
		//kprintf("cSyncServer");
		p2p_InitializeTime();

		Sleep(250);
		
		int ttime = P2PCORE.ping / 2 + 5;
		int predicted = p2p_GetTime() + ttime;
		
		p2p_instruction kxx_pdd(TSYNC,TSYNC_FORCE);
		kxx_pdd.store_int(predicted);
		P2PCORE.connection->send_instruction(&kxx_pdd);
		
		Sleep(250);
		
		p2p_instruction kxx_psl(TSYNC, TSYNC_CHECK);
		predicted = p2p_GetTime() + ttime;
		kxx_psl.store_int(predicted);
		P2PCORE.connection->send_instruction(&kxx_psl);
		
		int readjustments = 0;
		
		while (readjustments < 4){
			if (P2PCORE.connection->has_data() || (k_socket::check_sockets(1,0) && P2PCORE.connection->has_data())) {
				p2p_instruction ki;
				if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)) {
					//kprintf(__FILE__ ":%i", __LINE__);
					if (ki.inst.type == TSYNC){
						if (ki.inst.flags == TSYNC_ADJUST){
							//p2p_core_debug("TSYNC_ADJUST: %i, %i", predicted, 0);
							int dx = ki.load_int();
							p2p_core_debug("TSYNC_ADJUST: %i, %i", predicted, dx);

							if (dx > 0 && dx < ttime * ADJUST_RATIO_T)
								ttime += dx * ADJUST_RATIO_T;
							
							if (dx == 0)
								readjustments = 4;
							
							if (++readjustments < 4){
								
								Sleep(ttime + 10);
								predicted = p2p_GetTime() + ttime;
								p2p_instruction kx_rl4;
								kx_rl4.inst.type = TSYNC;
								kx_rl4.inst.flags = TSYNC_CHECK;
								kx_rl4.store_int(predicted);
								P2PCORE.connection->send_instruction(&kx_rl4);
								//p2p_core_debug("Sending TSYNC_CHECK %i", predicted);
							}
						}
					} else if (ki.inst.type == CHAT) {
						p2p_handle_chat_instruction(&ki);
					}
				}
			}
		}
		
		P2PCORE.throughput = 1 + (ttime * 60/1000) + (p2p_add_delay_callback()? 1:0);
		char buff[5];
		GetWindowText(p2p_ui_con_delay, buff, 5);
		int delay = atoi(buff);
		if(delay > 0)
		{
			/**/P2PCORE.throughput = delay;/**/
		}
		
		p2p_instruction kxxx(TTIME, 0);
		kxxx.store_int(P2PCORE.throughput);
		P2PCORE.connection->send_instruction(&kxxx);
	} else {
		//kprintf("cSyncClient");
		int readjustments = 0;
		
		while (readjustments < 4){
			if (P2PCORE.connection->has_data() || (k_socket::check_sockets(1,0) && P2PCORE.connection->has_data())) {
				p2p_instruction ki;
				if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)) {
					//kprintf(__FILE__ ":%i", __LINE__);
					if (ki.inst.type == TSYNC){
						if (ki.inst.flags == TSYNC_FORCE){
							p2p_InitializeTime();
							unsigned int tx = ki.load_int();
							p2p_initial_time += (p2p_GetTime() - tx);
							
							//p2p_core_debug("TSYNC_FORCE: %i, %i", tx, p2p_GetTime());
							
						} else if (ki.inst.flags == TSYNC_CHECK){
							int tx = ki.load_int();
							int dx = p2p_GetTime() - tx;
							p2p_initial_time += dx * ADJUST_RATIO_T;
							
							//p2p_core_debug("TSYNC_CHECK: %i, %i", tx, p2p_GetTime());
							
							tx += dx * ADJUST_RATIO_T;
							
							p2p_instruction kxx(TSYNC, TSYNC_ADJUST);
							kxx.store_int(dx);
							P2PCORE.connection->send_instruction(&kxx);
						}
					} else if (ki.inst.type == TTIME) {
						int tx = ki.load_int();
						P2PCORE.throughput = tx;
						readjustments = 20;
						break;
					} else if (ki.inst.type == CHAT) {
						p2p_handle_chat_instruction(&ki);
					}
				}
			}
		}
	}
	
	//kprintf(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
	P2PCORE.status = 3;
	
	//* /
	return true;
}



void p2p_step(){
	k_socket::check_sockets(0,200);
	while (P2PCORE.connection && P2PCORE.connection->has_data()){
		//p2p_core_debug(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
		p2p_instruction ki;
		sockaddr_in saddr;
		if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)){
			//p2p_core_debug(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
			//ki.to_string();
			switch(ki.inst.type){
			case LOGN:
				{
					if (P2PCORE.HOST && !P2PCORE.CONNECTED){
						if (P2PCORE.ping == -1 && ki.inst.flags == LOGN_REQ) {
							
							P2PCORE.connection->set_addr(&saddr);
							ki.load_sstring(P2PCORE.PEERNAME);
							
							char peerapp[128];
							ki.load_string(peerapp);
							
							p2p_core_debug("Connection Request from %s (%s).. Waiting for reconfirmation...", P2PCORE.PEERNAME, peerapp);
							
							if (strcmp(peerapp, P2PCORE.APP)!=0)
								p2p_send_chat("Emulator/version difference alert! Game may desync!");

							p2p_instruction kx;
							kx.inst.type = LOGN;
							kx.inst.flags = LOGN_RPOS;
							kx.store_sstring(P2PCORE.USERNAME);
							kx.store_string(P2PCORE.GAME);
							P2PCORE.connection->send_instruction(&kx);
							P2PCORE.ping = 0;
						}
						if (P2PCORE.ping == 0 && ki.inst.flags == LOGN_RPOS) {
							P2PCORE.CONNECTED = true;
							//p2p_core_debug("Peer reconfirmed P2PCORE.connection");
							
							p2p_instruction kx;
							kx.inst.type = PING;
							kx.inst.flags = PING_PING;
							kx.store_string((char*)&P2PCORE);
							P2PCORE.connection->send_instruction(&kx);
							p2p_PING_TIME = p2p_GetTime();
							
							p2p_send_chat("Using version: " P2P_VERSION " - Things may behave in an unexpected manner if different versions are used");
							MessageBeep(MB_OK);
							if (P2PCORE.USERREADY) {
								p2p_instruction kxx;
								kxx.inst.type = PREADY;
								kxx.inst.flags = PREADY_READY;
								P2PCORE.connection->send_instruction(&kxx);
							}

						}
						
						if (P2PCORE.ping == 0 && ki.inst.flags == LOGN_RNEG) {
							P2PCORE.ping = -1;
							p2p_core_debug("Peer dropped P2PCORE.connection (probbly doensot have the game)");
						}
					} else {
						if (!P2PCORE.HOST && !P2PCORE.CONNECTED){
							if (P2PCORE.ping == -1 && ki.inst.flags == LOGN_RPOS) {
								ki.load_sstring(P2PCORE.PEERNAME);
								ki.load_string(P2PCORE.GAME);
								p2p_core_debug("Peer replied: %s (%s)", P2PCORE.PEERNAME, P2PCORE.GAME);
								
								P2PCORE.CONNECTED = true;

								
								p2p_hosted_game_callback(P2PCORE.GAME);
								/*
								char xxx[800];
								OutputHex(xxx, &ki, sizeof(ki), 0, true);
								outp(xxx);
								//*/
								p2p_instruction kx;
								kx.inst.type = LOGN;
								kx.inst.flags = LOGN_RPOS;
								P2PCORE.connection->send_instruction(&kx);
								P2PCORE.ping = 0;
								
								p2p_instruction kxx;
								kxx.inst.type = PING;
								kxx.inst.flags = PING_PING;
								kxx.store_string((char*)&P2PCORE);
								P2PCORE.connection->send_instruction(&kxx);
								p2p_PING_TIME = p2p_GetTime();
								
								p2p_send_chat("Using version: " P2P_VERSION);
								MessageBeep(MB_OK);
								
							}
							if (P2PCORE.ping == -1 && ki.inst.flags == LOGN_RNEG) {
								p2p_core_debug("Peer dropped connection (Probably different emu version)");
								P2PCORE.ping = -1;
							}
						}
					}
				}
				break;
				
			case CHAT:
				{
					p2p_handle_chat_instruction(&ki);
				}
				break;
			case PING:
				{
					if (ki.inst.flags == PING_PING) {
						ki.inst.flags = PING_ECHO;
						ki.pos = ki.len;
						P2PCORE.connection->send_instruction(&ki);
					} else {
						P2PCORE.ping = p2p_GetTime() - p2p_PING_TIME;
						p2p_ping_callback(P2PCORE.ping);
					}
				}
				break;
			case PREADY:
				P2PCORE.PEERREADY = (ki.inst.flags == PREADY_READY);
				p2p_core_debug("%s is %s", P2PCORE.PEERNAME,P2PCORE.PEERREADY? "ready":"not ready");
				if (P2PCORE.USERREADY && P2PCORE.PEERREADY) {
					p2p_send_chat("both players are ready, starting game");
					p2p_instruction kx;
					kx.inst.type = LOAD;
					kx.inst.flags = LOAD_LOAD;
					P2PCORE.connection->send_instruction(&kx);
					Sleep(P2P_GAMECB_WAIT);
					p2p_game_callback(P2PCORE.GAME, P2PCORE.HOST? 1:2, 2);
				}
				break;
			case LOAD:
				Sleep(P2P_GAMECB_WAIT);
				p2p_game_callback(P2PCORE.GAME, P2PCORE.HOST? 1:2, 2);
				break;
			case EXIT:
				p2p_core_debug("Peer left");
				P2PCORE.status = P2PCORE.HOST?1:0;
				P2PCORE.CONNECTED = false;
				
				if (P2PCORE.status) {
					p2p_core_debug("Re Initializing server...");
					delete P2PCORE.connection;
					P2PCORE.connection = 0;
					
					if (P2PCORE.connection != 0)
						delete P2PCORE.connection;
					P2PCORE.connection = new p2p_message;
					
					if (!P2PCORE.connection->initialize(P2PCORE.PORT)){
						p2p_core_debug("Error initializing socket at specified port");
					} else p2p_core_debug("Done");
					
					P2PCORE.status = 1;
					P2PCORE.ping = -1;
					P2PCORE.USERREADY = false;
					P2PCORE.PEERREADY = false;
					
				}
				break;
		}
	}
	}
	if (p2p_chat_cache.length > 0){
		do {
			p2p_chatstruct * cma = &p2p_chat_cache.items[0];
			p2p_chat_callback(cma->local? P2PCORE.USERNAME:P2PCORE.PEERNAME, cma->msg);
			p2p_chat_cache.removei(0);
		} while (p2p_chat_cache.length > 0);
	}
}

inline bool ProcessGameStuff(){
	sockaddr_in saddr;
	while (P2PCORE.connection->has_data()){
		p2p_instruction ki;
		if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)){
			if (ki.inst.type == DATA) {
				char data[32];
				ki.load_bytes(data, P2PCORE.DATALEN);
				P2PCORE.PEERDATA.put_data(data, P2PCORE.DATALEN);
			} else if (ki.inst.type == CHAT) {
				p2p_handle_chat_instruction(&ki);
			} else if (ki.inst.type == DROP) {
				p2p_core_debug("peer dropped");
				
				P2PCORE.status = 1;
				P2PCORE.USERREADY = false;
				P2PCORE.PEERREADY = false;
				
				p2p_client_dropped_callback(P2PCORE.PEERNAME, P2PCORE.HOST? 2: 1);
				p2p_client_dropped_callback(P2PCORE.USERNAME, P2PCORE.HOST? 1: 2);

				p2p_end_game_callback();

				return true;
			}
		}
		//data chat and drop
	}
	return false;
}

int p2p_modify_play_values(void *values, int size){
	TRACE
	if (P2PCORE.status > 1) {
		TRACE
		P2PCORE.USERDATA.put_data(values, P2PCORE.DATALEN);
		TRACE
		p2p_instruction kix(DATA,0);
		kix.store_bytes(values, P2PCORE.DATALEN);
		P2PCORE.connection->send_instruction(&kix);
		TRACE
		if (P2PCORE.frameno++ >= P2PCORE.throughput) {
			TRACE
			int TI = p2p_GetTime();
			TRACE
			while (P2PCORE.PEERDATA.pos < P2PCORE.DATALEN && P2PCORE.status > 1){
				if (P2PCORE.connection->has_data() || (k_socket::check_sockets(0,10) && P2PCORE.connection->has_data())){
					if (ProcessGameStuff())
						return -1;
				} else if (p2p_GetTime()-TI > P2PCORE.throughput * 17) p2p_retransmit();
			}
			TRACE
			if (P2PCORE.frameno > P2PCORE.crframeno && p2p_chat_cache.length > 0){
				p2p_chatstruct * cma = &p2p_chat_cache.items[0];
				p2p_chat_callback(cma->local? P2PCORE.USERNAME:P2PCORE.PEERNAME, cma->msg);
				p2p_chat_cache.removei(0);
				
				if (p2p_chat_cache.length > 0) {
					P2PCORE.crframeno = p2p_chat_cache.items[0].crframeno;
				} else 
					P2PCORE.crframeno = 5 * P2PCORE.frameno;
			}
			TRACE
			if (P2PCORE.PEERDATA.pos >= P2PCORE.DATALEN) {
				if (P2PCORE.HOST) {
					P2PCORE.USERDATA.get_data((char*)values, P2PCORE.DATALEN);
					P2PCORE.PEERDATA.get_data((((char*)values)+P2PCORE.DATALEN), P2PCORE.DATALEN);
				} else {
					P2PCORE.PEERDATA.get_data((char*)values, P2PCORE.DATALEN);
					P2PCORE.USERDATA.get_data((((char*)values)+P2PCORE.DATALEN), P2PCORE.DATALEN);
				}
				return P2PCORE.DATALEN * 2;
			}
			TRACE
			return -1;
			//other user's input
		} else {
			TRACE
			return 0;
		}
	} else {
		TRACE
		//kprintf(__FILE__ ":%i", __LINE__);
		TRACE
		sockaddr_in saddr;
		//initialize cache
		P2PCORE.USERDATA.reset();
		P2PCORE.PEERDATA.reset();
		TRACE
		p2p_chat_cache.clear();

		P2PCORE.DATALEN = size;
		
		P2PCORE.status = 1;
		TRACE
		if (p2p_WaitForPeerToLoadOrDie()){
			TRACE
			p2p_core_debug("===Everyone Loaded=====================================================");
		}
		TRACE
		if (P2PCORE.status == 2 && p2p_CalculatePingOrDie()){
			p2p_core_debug("===Calculated Ping=%ims================================================", P2PCORE.ping);
		}
		TRACE
		if (P2PCORE.status == 2 && p2p_SynChronizeClocksOrDie()) {
			p2p_core_debug("===Calculated delay %i frame(s)====================\r\n", P2PCORE.throughput);
		}
		TRACE
		
		//kprintf(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
		p2p_core_debug("gamesync in less than %i second(s)", (P2P_GAMESYNC_WAIT/1000)+1);
		
		P2PCORE.frameno = 0;
		P2PCORE.crframeno = 0;
		int start_time = 0;
		//kprintf(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
		
		if (P2PCORE.HOST){
			start_time = p2p_GetTime()+P2P_GAMESYNC_WAIT;
			p2p_instruction ki(START, 0);
			ki.store_int(start_time);
			P2PCORE.connection->send_instruction(&ki);
		} else {
			while (start_time == 0){
				if (P2PCORE.connection->has_data() || (k_socket::check_sockets(1,0) && P2PCORE.connection->has_data())){
					p2p_instruction ki;
					if (P2PCORE.connection->receive_instruction(&ki, false, &saddr)) {
						if (ki.inst.type == START){
							start_time = ki.load_int();
							//kprintf("Start time %i", start_time);
						}
					}
				}
			}
		}
		TRACE
		//kprintf(__FILE__ ":%i, %i", __LINE__, p2p_GetTime());
		
		int tl = start_time - p2p_GetTime();
		//kprintf("time left %i ms", tl);
		TRACE
		k_socket::check_sockets(tl / 1000, tl % 1000);
		TRACE
		return p2p_modify_play_values(values, size);
		
	}
}

