#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <vector>

#include "const.hpp"

using namespace std;

class Cpu {
	public:
		// INTERFACE
		bool step();
		vector<bool> getRegister();
		vector<bool> getPc();
		vector<bool> getInstructionCode();
		vector<bool> getValue();
		bool hasCodeAddress();
		vector<bool> getCodeAddress();
		bool hasDataAddress();
		vector<bool> getDataAddress();
		int getCycle();
	private:
		// STATE
		vector<bool> reg = vector<bool>(WORD_SIZE);
		vector<bool> pc = vector<bool>(ADDR_SIZE);
		int cycle = 0;
		// INSTRUCTION SET
		void read(vector<bool> adr);
		void write(vector<bool> adr);
		void add(vector<bool> adr);
		void sub(vector<bool> adr);
		void jump(vector<bool> adr);
		void ifMax(vector<bool> adr);
		void ifMin(vector<bool> adr);
		void logic(vector<bool> value);
		void readPointer(vector<bool> adr);
		void writePointer(vector<bool> adr);
		void incDec(vector<bool> adr);
		void print(vector<bool> adr);
		void ifNotMax(vector<bool> adr);
		void ifNotMin(vector<bool> adr);
		// UTIL
		void increasePc();
		void addOrSubtract(vector<bool> adr, bool add);
		void shift(int delta);
		void andOrOr(bool isAnd);
		void bitwiseNot();
		void equals();
		void jumpReg();
		void readReg();
		void incOrDec(vector<bool> adr, bool isInc);
		bool getRegBit(int index);
};

#endif
