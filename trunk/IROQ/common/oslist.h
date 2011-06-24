/******************************************************************************
***N***N**Y***Y**X***X*********************************************************
***NN**N***Y*Y****X*X**********************************************************
***N*N*N****Y******X***********************************************************
***N**NN****Y*****X*X************************ Make Date  : Jun 29, 2007 *******
***N***N****Y****X***X*********************** Last Update: Jun 29, 2007 *******
******************************************************************************/
#ifndef N_STL_OSLIST
#define N_STL_OSLIST

#include <string>
#ifndef __GNUC__
#pragma intrinsic (memcpy)
#endif

template <class _Type, int _Size>
class oslist {
public:
	_Type items[_Size];	
	int length;	

public:

	oslist(){
		length = 0;
	}

	//add 	adds an element to the list
	void add(_Type element){
		items[length++] = element;
	}

	//remove 	removes an element from the list
	void removei(int i){
		if (i >= 0 && i < length) {
			int l = length-1;
			if (l!=i) {
				memcpy(&items[i], &items[i+1], (l-i) * sizeof(_Type));
			}
			length = l;
		}
	}

	void remove(_Type t){
		for (int i = 0; i < length; i++) {
			if (items[i] == t) {
				removei(i);
				i--;
			}
		}
	}

	void set_size(int l){
		length = l;
	}


	//set 	sets an element value in the list
	void set(_Type v, int i){
		items[i] = v;
	}

	//get 	gets an element value in the list
	_Type get(int i){
		return items[i];
	}
	
	_Type operator[] (const int i) {
		return items[i];
	}

	//clear 	removes all elements from the list	
	void clear(){
		length = 0;
	}

	//size 	returns the number of items in the list
	int size(){
		return length;
	}
};
#endif
