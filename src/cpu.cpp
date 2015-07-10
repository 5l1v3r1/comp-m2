#include "cpu.hpp"

#include <cmath>
#include <string>
#include <vector>
#include <set>

#include "comp.hpp"
#include "util.hpp"

using namespace std;


/////////////////////////
/////// INTERFACE ///////
/////////////////////////

int Cpu::getCycle() {
	return cycle;
}

vector<bool> Cpu::getRegister() {
	return reg;
}

vector<bool> Cpu::getPc() {
	return pc;
}

vector<bool> Cpu::getInstructionCode() {
	return Util::getFirstNibble(ram.get(CODE, pc));
}

vector<bool> Cpu::getValue() {
	return Util::getSecondNibble(ram.get(CODE, pc));
}

bool Cpu::hasDataAddress() {
	vector<bool> instructionCodeBool = getInstructionCode();
	int instCode = Util::getInt(instructionCodeBool);
	set<int> instWithoutDataAddress = { 4, 5, 6, 7, 10, 13, 14, 15 };
	return instWithoutDataAddress.count(instCode) == 0;
}

vector<bool> Cpu::getDataAddress() {
	vector<bool> instructionCodeBool = getInstructionCode();
	int instCode = Util::getInt(instructionCodeBool);

	// If instruction has only 3 bits for address.
	set<int> instWith3bitAddress = { 2, 3, 11 };
	if (instWith3bitAddress.count(instCode) == 1) {
		vector<bool> value = getValue();
		return { false, value.at(1), value.at(2), value.at(3) };
	} else { 
		return Util::getSecondNibble(ram.get(CODE, pc));
	}
}

bool Cpu::hasCodeAddress() {
	vector<bool> instructionCodeBool = getInstructionCode();
	int instCode = Util::getInt(instructionCodeBool);
	set<int> instWithCodeAddress = { 4, 5, 6, 13, 14 };
	return instWithCodeAddress.count(instCode) == 1;
}

vector<bool> Cpu::getCodeAddress() {
	return Util::getSecondNibble(ram.get(CODE, pc));
}

/*
 * Returns 'false' when reaches last address.
 */
bool Cpu::step() {
	cycle++;
	bool reachedLastAddress = Util::getInt(pc) >= RAM_SIZE;
	if (reachedLastAddress) {
		return false;
	}

	vector<bool> instructionCodeBool = getInstructionCode();
	int instCode = Util::getInt(instructionCodeBool);
	vector<bool> value = getValue();

	switch (instCode) {
		case 0:
			read(value);
			break;
		case 1:
			write(value);
			break;
		case 2:
			add(value); 
			break;
		case 3:
			sub(value);
			break;
		case 4:
			jump(value);
			break;
		case 5:
			ifMax(value);
			break;
		case 6:
			ifMin(value);
			break;
		case 7:
			logic(value);
			break;
		case 8:
			readPointer(value);
			break;
		case 9:
			writePointer(value);
			break;
		case 10:
			incDec(value);
			break;
		case 11:
			print(value);
			break;
		case 13:
			ifNotMax(value);
			break;
		case 14:
			ifNotMin(value);
			break;
		default:
			read(value);
	}

	return true;
}

///////////////////////////////
/////// INSTRUCTION SET ///////
///////////////////////////////

/*
 * Copies value at the passed address to the register.
 */
void Cpu::read(vector<bool> adr) {
	reg = ram.get(DATA, adr);
	increasePc();
}

/*
 * Copies value of the register to the passed address.
 */
void Cpu::write(vector<bool> adr) {
	ram.set(DATA, adr, reg);
	increasePc();
}

/*
 * Adds value at the passed address to the register, and
 * stores result in the register.
 */
void Cpu::add(vector<bool> adr) {
	addOrSubtract(adr, true);
	increasePc();
}

/*
 * Subtracts value at the passed address from the register, and
 * stores result in the register.
 */
void Cpu::sub(vector<bool> adr) {
	addOrSubtract(adr, false);
	increasePc();
}

/*
 * Jumps to the passed address.
 */
void Cpu::jump(vector<bool> adr) {
	pc = adr;
}

/*
 * Jumps to passed address if value of the register is 'max'.
 */
void Cpu::ifMax(vector<bool> adr) {
	if (Util::getInt(reg) >= pow(2, WORD_SIZE)-1) {
		pc = adr;
	} else {
		increasePc();
	}
}

/*
 * Jumps to passed address if value of the register is 'min'.
 */
void Cpu::ifMin(vector<bool> adr) {
	if (Util::getInt(reg) <= 0) {
		pc = adr;
	} else {
		increasePc();
	}
}

// << >> & | ^ == JUMP_REG READ_REG
void Cpu::logic(vector<bool> value) {
	switch (Util::getInt(value)) {
		case 0:
			shift(1);
			break;
		case 1:
			shift(-1);
			break;
		case 2:
			andOrOr(true); 
			break;
		case 3:
			andOrOr(false);
			break;
		case 4:
			bitwiseNot();
			break;
		case 5:
			equals();
			break;
		case 6:
			jumpReg();
			break;
		case 7:
			readReg();
			break;
		default:
			fprintf(stderr, "Passed value to method Cpu::logic"
					" out of range: %d. Caneling this "
					"instruction.", Util::getInt(value));
	}
}

/*
 * Reads from the address that is stored at passed address.
 */
void Cpu::readPointer(vector<bool> adr) {
	vector<bool> pointer = ram.get(DATA, adr);
	reg = ram.get(DATA, pointer);
	increasePc();
}

/*
 * Writes 'reg' to the address that is stored at passed address.
 */
void Cpu::writePointer(vector<bool> adr) {
	vector<bool> pointer = ram.get(DATA, adr);
	ram.set(DATA, pointer, reg);
	increasePc();
}

/*
 * Increases or decreases value at the passed address, and copies
 * it to the register.
 */
void Cpu::incDec(vector<bool> value) {
	bool isInc = !value.at(0);
	vector<bool> adr = { false, value.at(1), value.at(2), value.at(3) };
	incOrDec(adr, isInc);
	increasePc();
}

/*
 * Copies value at the passed address to the last address and thus
 * sends it to the printer.
 */
void Cpu::print(vector<bool> adr) {
	vector<bool> val = ram.get(DATA, adr);
	ram.set(DATA, Util::getLastAddress(), val);
	increasePc();
}

/*
 * Jumps to passed address if value of the register is not 'max'.
 */
void Cpu::ifNotMax(vector<bool> adr) {
	if (Util::getInt(reg) >= pow(2, WORD_SIZE)-1) {
		increasePc();
	} else {
		pc = adr;
	}
}

/*
 * Jumps to passed address if value of the register is not 'min'.
 */
void Cpu::ifNotMin(vector<bool> adr) {
	if (Util::getInt(reg) <= 0) {
		increasePc();
	} else {
		pc = adr;
	}
}

///////////////////
////// UTIL ///////
///////////////////

void Cpu::increasePc() {
	pc = Util::getBoolNibb(Util::getInt(pc) + 1);
}

/*
 * Adds or subtracts value at passed address from register,
 * and stores result in the register.
 */
void Cpu::addOrSubtract(vector<bool> adr, bool add) {
	int regValue = Util::getInt(reg);
	int ramValue = Util::getInt(ram.get(DATA, adr));
	if (add) {
		reg = Util::getBoolByte(regValue + ramValue);
	} else {
		reg = Util::getBoolByte(regValue - ramValue);
	}
}

/*
 * Shifts bits in the register for 'delta' spots.
 */
void Cpu::shift(int delta) {
	vector<bool> tmp = vector<bool>(WORD_SIZE);
	for(int i = 0; i < WORD_SIZE; i++) {
		tmp[i] = getRegBit(i + delta);
	}
	reg = tmp;
	increasePc();
}

/*
 * Executes 'and' operation between register value, and
 * value at first address and writes the result to register.
 */
void Cpu::andOrOr(bool isAnd) {
	vector<bool> ramValue = ram.get(DATA, Util::getFirstAddress());
	reg = Util::bitwiseAndOrOr(reg, ramValue, isAnd);
	increasePc();
}

/*
 * Executes 'not' operation on the value of the register.
 */
void Cpu::bitwiseNot() {
 	reg = Util::bitwiseNot(reg);
	increasePc();
}

/*
 * Checks if value of the register and value at the first address
 * are the same, and if so sets value of the register to 'max',
 * otherwise to the 'min'.
 */
void Cpu::equals() {
	vector<bool> ramValue = ram.get(DATA, Util::getFirstAddress());
	if (ramValue == reg) {
		reg = vector<bool>(WORD_SIZE, true); 
	} else {
		reg = vector<bool>(WORD_SIZE, false);
	}
	increasePc();
}

/*
 * Jumps to the address stored in register.
 */
void Cpu::jumpReg() {
	pc = Util::getSecondNibble(reg);
}

/*
 * Copies value at the address that is stored in register
 * to the register.
 */
void Cpu::readReg() {
	vector<bool> adr =Util::getSecondNibble(reg);
	reg = ram.get(DATA, adr);
	increasePc();
}

/*
 * Increases or decreases value at the passed address, and copies
 * it to register.
 */
void Cpu::incOrDec(vector<bool> adr, bool isInc) {
	vector<bool> value = ram.get(DATA, adr);
	int intValue;
	if (isInc) {
		intValue = Util::getInt(value) + 1;
	} else {
		intValue = Util::getInt(value) - 1;
	}
	ram.set(DATA, adr, Util::getBoolByte(intValue));
	reg = Util::getBoolByte(intValue);
}

/*
 * Returns register bit at passed position, or 'false' if position 
 * is out of bounds.
 */
bool Cpu::getRegBit(int index) {
	bool indexOutOfBounds = index < 0 || index >= WORD_SIZE;
	if (indexOutOfBounds) {
		return false;
	}
	return reg.at(index);
}
