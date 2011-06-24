/******************************************************************************
***N***N**Y***Y**X***X*********************************************************
***NN**N***Y*Y****X*X**********************************************************
***N*N*N****Y******X***********************************************************
***N**NN****Y*****X*X************************ Make Date  : Jul 01, 2007 *******
***N***N****Y****X***X*********************** Last Update: Jul 01, 2007 *******
******************************************************************************/
#pragma once
#include "../common/k_socket.h"
#include "p2p_instruction.h"

#define ICACHESIZE 32

#pragma pack(push, 1)

typedef struct {
    unsigned char serial;
    unsigned char length;
} p2p_instruction_head;


#pragma pack(pop)

typedef struct {
    p2p_instruction_head head;
    char body[256];
} p2p_instruction_ptr; 

/*
class in_cache_q: {



};
//*/

extern int PACKETLOSSCOUNT;
extern int PACKETMISOTDERCOUNT;

void __cdecl kprintf(char * arg_0, ...);
void OutputHexx(const void * inb, int len, bool usespace);

class p2p_message : public k_socket {
    unsigned char      last_sent_instruction;
    unsigned char      last_processed_instruction;
	unsigned char      last_cached_instruction;
	oslist<p2p_instruction_ptr, ICACHESIZE> out_cache;
	oslist<p2p_instruction_ptr, ICACHESIZE> in_cache;
public:
	int                 default_ipm;
	int                 dsc;
    p2p_message(){
        k_socket();
        last_sent_instruction = 0;
        last_processed_instruction = 0;
		last_cached_instruction = -1;
        default_ipm = 3;
		dsc = 0;
    }

    void send_instruction(p2p_instruction * arg_0){
        p2p_message::send((char*)arg_0, arg_0->size());
		////kprintf("Sent:");
		//arg_0->to_string();
    }

	void send_tinst(int type, int flags){
		p2p_instruction kx(type, flags);
        p2p_message::send((char*)&kx, kx.size());
		////kprintf("Sent:");
		//kx.to_string();
    }

    bool send(char * buf, int len){
        unsigned char vx = last_sent_instruction++;
		p2p_instruction* ibuf = (p2p_instruction*)buf;

        if (out_cache.size() == ICACHESIZE-1) {
			out_cache.removei(0);
        }

		p2p_instruction_ptr kip;
		kip.head.serial = vx;
		kip.head.length = ibuf->write_to_message(kip.body);
		out_cache.add(kip);

        send_message(default_ipm);
        return true;
    }

    bool send_message(int limit){
        char buf[0x1000];
        int len = 1;
        char * buff = buf;
        char max_t = min(out_cache.size(), limit);
        *buff++ = max_t;
		////kprintf(__FILE__ "%i", out_cache.size());
        if(max_t > 0) {
            for (int i = 0; i < max_t; i++) {
                int cache_index = out_cache.size() - i -1;
                *(p2p_instruction_head*)buff = out_cache[cache_index].head;
                buff += sizeof(p2p_instruction_head);
                int l;
                memcpy(buff, out_cache[cache_index].body, l = out_cache[cache_index].head.length);
                buff += l;
                len += l + sizeof(p2p_instruction_head);
            }
        }
		if (dsc>0)
			dsc--;
		else
			k_socket::send(buf, len);
        return true;
    }

    bool receive_instruction(p2p_instruction * arg_0, bool leave_in_queue, sockaddr_in* arg_8) {
        char var_8000[1024];
        int var_8004 = 1024;
        if (check_recv(var_8000, &var_8004, leave_in_queue, arg_8)) {
			////kprintf("Received:");
            arg_0->read_from_message(var_8000, var_8004);
			//arg_0->to_string();
            return true;
        }
		return false;
    }

    bool check_recv (char* buf, int * len, bool leave_in_queue, sockaddr_in* addrp){
		////kprintf(__FILE__ ":%i", __LINE__);
        if (has_data_waiting) {
			////kprintf(__FILE__ ":%i", __LINE__);
            char buff      [2024];
            int  bufflen = 2024;
            if (k_socket::check_recv(buff, &bufflen, false, addrp)) {
                unsigned char instruction_count = *buff;

                char* ptr = buff + 1;
                if (instruction_count > 0 && instruction_count < 15) {
					
					////kprintf(__FILE__ ":%i", __LINE__);
					
					//OutputHexx(buff, bufflen, 0);

					unsigned char latest_serial = *ptr;

					int si = in_cache.size();
					////kprintf("si=%i", si);

					unsigned char tx = latest_serial-last_cached_instruction;

                    if (tx > 0 && tx < 15) {

    					in_cache.set_size(si+tx);
    					//kprintf("ss=%i %s", in_cache.size(), buff);
    
                        for (int u=0; u<instruction_count; u++) {
    						//kprintf(__FILE__ ":%i", __LINE__);
    
    						unsigned char serial = ((p2p_instruction_head*)ptr)->serial;
                            unsigned char length = ((p2p_instruction_head*)ptr)->length;
    
    						ptr += sizeof(p2p_instruction_head);
    
                            if (serial == last_cached_instruction)
                                break;
    
    						unsigned char cix = serial - last_cached_instruction;
    
    						PACKETLOSSCOUNT += cix - 1;
    
    						int ind = si + (cix) - 1;
    						
    						//kprintf("ind=%i", ind);
    
    						in_cache.items[ind].head.serial = serial;
    						in_cache.items[ind].head.length = length;
                            memcpy(in_cache.items[ind].body, ptr, length);
    
                            ptr += length;
                        }
    
    					last_cached_instruction = latest_serial;
                    } else PACKETMISOTDERCOUNT++;
                }
            } //else //kprintf("ERR%i", WSAGetLastError());
        }

		////kprintf(__FILE__ ":%i", __LINE__);
        if (in_cache.size() > 0) {
			
			////kprintf(__FILE__ ":%i", __LINE__);

			*len = in_cache[0].head.length;
			memcpy(buf, in_cache[0].body, *len);
			//OutputHexx(buf, *len, true);

			////kprintf(__FILE__ ":%i", __LINE__);

			if(!leave_in_queue) {

				////kprintf(__FILE__ ":%i", __LINE__);

				last_processed_instruction = in_cache[0].head.serial;
				in_cache.removei(0);

			}

			return true;
        }
		//kprintf(__FILE__ ":%i return false", __LINE__);
        return false;
    }

    bool has_data(){
        if (in_cache.length == 0)
            return has_data_waiting;
        else
            return true;
    }

    void resend_message(int limit){
        send_message(limit);
    }
};
