#include "renderer.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "addr_space.hpp"
#include "address.hpp"
#include "const.hpp"
#include "cpu.hpp"
#include "cursor.hpp"
#include "drawing.hpp"
#include "instruction.hpp"
#include "printer.hpp"
#include "ram.hpp"
#include "util.hpp"

using namespace std;

/*
 * Only public method. Also static. It creates new object every time 
 * it gets called.
 */
string Renderer::renderState(Printer printerIn, Ram ramIn, Cpu cpuIn, Cursor cursorIn) {
	Renderer instance(printerIn, ramIn, cpuIn, cursorIn);
	string out;
	for (string line : Util::splitString(drawing)) {
		out += instance.insertActualValues(line) + "\n";
	}
	out.erase(out.end() - 1);
	return out;
}

/*
 * Private constructor.
 */
Renderer::Renderer(Printer printerIn, Ram ramIn, Cpu cpuIn, Cursor cursorIn) {
	Renderer::printer = printerIn;
	Renderer::ram = ramIn;
	Renderer::cpu = cpuIn;
	Renderer::cursor = cursorIn;
	switchIndex.clear();
}

string Renderer::insertActualValues(string lineIn) {
	string lineOut;
	for (char cIn : lineIn) {
		char cOut;
		// Regex: [0-9a-z]
		bool charIsALightbulb = (cIn >= 'a' && cIn <= 'z') || 
				(cIn >= '0' && cIn <= '9');
		if (charIsALightbulb) {
			cOut = getLightbulb(cIn);
		} else {
			cOut = cIn;
		}
		lineOut += cOut;
	}
	return lineOut;
}

char Renderer::getLightbulb(char cIn) {
	int i = switchIndex[cIn]++;
	switch (cIn) {
		case 'a':
			return getCodeBit(i);
		case 'b':
			return getDataBit(i);
		case 'p':
			return Util::getChar(pcPointingToAddress(i));
		case 'd':
			{
				Address adr = Address(CODE, Util::getBoolNibb(i));
				return Util::getChar(instructionPointingToAddress(adr));
			}
		case 's':
			{
				Address adr = Address(DATA, Util::getBoolNibb(i));
				return Util::getChar(instructionPointingToAddress(adr));
			}
		case 'r':
			return  Util::getChar(cpu.getRegister().at(i));
		case 'i':
			return Util::getChar(instructionHasId(i));
		case 'o':
			return getFormattedOutput(i);
	}
	fprintf(stderr, "There was an error parsing a drawing file."
					" Problem with char %c. Will ignore it.", cIn);
	return ' ';
}

bool Renderer::pcPointingToAddress(int adr) {
	bool executionHasntStarted = cpu.getCycle() == 0;
	if (executionHasntStarted) {
		return false;
	}
	return Util::getInt(cpu.getPc()) == adr;
}

/*
 * Is instruction pointing to passed address in passed address space.
 */
bool Renderer::instructionPointingToAddress(Address adr) {
	if (machineNotActive()) {
		int cursorOnData = cursor.getAddressSpace() == DATA;
		if (cursorOnData) {
			return false;
		}
		//vector<bool> instruction = cursor.getWord();

		Instruction * inst = getCursorsInstruction();
		return inst->adr == adr;

		// AddrSpace instructionsAddrSpace = Cpu::getAddressSpaceOfInstruction(instruction);
		// if (instructionsAddrSpace != addrSpace) {
		// 	return false;
		// }
		// vector<bool> instructionsAddress = Cpu::getAddressOfInstruction(instruction, EMPTY_WORD, ram);
		// bool instructionIsPointingToCurrentAddress = Util::getInt(instructionsAddress) == adr;
		// return instructionIsPointingToCurrentAddress;
	}
	Instruction inst = cpu.getInstruction();
	return inst.adr == adr;
	// if (!cpu.hasAddress(addrSpace)) {
	// 	return false;
	// }
	// return cpu.getAddress() == Util::getBoolNibb(adr);
}

Instruction* Renderer::getCursorsInstruction() {
	if (!cursorsInstruction) {
		static vector<bool> phonyReg = vector<bool>(EMPTY_WORD);
		cursorsInstruction = &Instruction(cursor.getWord(), &phonyReg,	&ram);
	}
	return cursorsInstruction;
}

bool Renderer::instructionHasId(int id) {
	if (machineNotActive()) {
		// Instruction * inst = getCursorsInstruction();
		// return inst->

		int cursorOnData = cursor.getAddressSpace() == DATA;
		if (cursorOnData) {
			return false;
		}
		vector<bool> instruction = Util::getFirstNibble(cursor.getWord());
		int instructionCode = Util::getInt(instruction);
		return instructionCode == id;
	}
	return cpu.getInstructionCodeInt() == id;
}

char Renderer::getFormattedOutput(int i) {
	if (printer.getPrinterOutput().length() <= (unsigned) i) {
		return ' ';
	} else {
		return printer.getPrinterOutput().at(i);
	}
}

char getCharAt(int i, vector<vector<bool>>* matrix) {
	int j = i / WORD_SIZE;
	i = i % WORD_SIZE;
	return Util::getChar((*matrix).at(j).at(i));
}

char Renderer::getCodeBit(int i) {
	return getCharAt(i, &ram.state[CODE]);
}

char Renderer::getDataBit(int i) {
	return getCharAt(i, &ram.state[DATA]);
}

bool Renderer::machineNotActive() {
	bool executionHasntStarted = cpu.getCycle() == 0;
	bool executionEnded = Util::getInt(cpu.getPc()) == RAM_SIZE;
	return executionHasntStarted || executionEnded;
}