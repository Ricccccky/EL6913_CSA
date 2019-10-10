#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR 5
#define NOR 7
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

class RF
{
public:
	bitset<32> ReadData1, ReadData2;
	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
	{
		// implement the funciton by you.

		// Read
		if (RdReg1.to_ulong() < 32)
		{
			ReadData1 = Registers[RdReg1.to_ulong()];
		}
		if (RdReg2.to_ulong() < 32)
		{
			ReadData2 = Registers[RdReg2.to_ulong()];
		}

		// Write
		if (WrtEnable.test(0))
		{
			Registers[WrtReg.to_ulong()] = WrtData;
		}
	}

	void OutputRF() // write RF results to file
	{
		ofstream rfout;
		rfout.open("RFresult.txt", std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "A state of RF:" << endl;
			for (int j = 0; j < 32; j++)
			{
				rfout << Registers[j] << endl;
			}
		}
		else
			cout << "Unable to open file";
		rfout.close();
	}

private:
	vector<bitset<32>> Registers;
};

class ALU
{
public:
	bitset<32> ALUresult;
	bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
	{
		// implement the ALU operations by you.

		switch (ALUOP.to_ulong())
		{
		case ADDU:
			ALUresult = bitset<32>(oprand1.to_ulong() + oprand2.to_ulong());
			break;
		case SUBU:
			ALUresult = bitset<32>(oprand1.to_ulong() - oprand2.to_ulong());
			break;
		case AND:
			ALUresult = oprand1 & oprand2;
			break;
		case OR:
			ALUresult = oprand1 | oprand2;
			break;
		case NOR:
			ALUresult = ~(oprand1 | oprand2);
			break;
		default:
			break;
		}
		return ALUresult;
	}
};

class INSMem
{
public:
	bitset<32> Instruction;
	INSMem() // read instruction memory
	{
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open("imem.txt");
		if (imem.is_open())
		{
			while (getline(imem, line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else
			cout << "Unable to open file";
		imem.close();
	}

	bitset<32> ReadMemory(bitset<32> ReadAddress)
	{
		// implement by you. (Read the byte at the ReadAddress and the following three byte).
		unsigned RealAddress = ReadAddress.to_ulong();
		string InstructionString = "";
		for (int i = 0; i < 4; i++)
		{
			InstructionString += IMem[RealAddress + i].to_string();
		}
		Instruction = bitset<32>(InstructionString);

		return Instruction;
	}

private:
	vector<bitset<8>> IMem;
};

class DataMem
{
public:
	bitset<32> readdata;
	DataMem() // read data memory
	{
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else
			cout << "Unable to open file";
		dmem.close();
	}
	bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
	{
		// implement by you.

		if (writemem.test(0))
		{
			unsigned RealAddress = Address.to_ulong();
			for (int i = 0; i < 4; i++)
			{
				DMem[RealAddress + i] = bitset<8>((WriteData >> (32 - 8 * (i + 1))).to_ulong());
			}
		}

		if (readmem.test(0))
		{
			unsigned RealAddress = Address.to_ulong();
			string DataString = "";
			for (int i = 0; i < 4; i++)
			{
				DataString += DMem[RealAddress + i].to_string();
			}
			readdata = bitset<32>(DataString);
		}

		return readdata;
	}

	void OutputDataMem() // write dmem results to file
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j < 1000; j++)
			{
				dmemout << DMem[j] << endl;
			}
		}
		else
			cout << "Unable to open file";
		dmemout.close();
	}

private:
	vector<bitset<8>> DMem;
};

int main()
{
	RF myRF;
	ALU myALU;
	INSMem myInsMem;
	DataMem myDataMem;

	bitset<32> pc(0);

	while (1)
	{
		// Fetch
		bitset<32> insturciton = myInsMem.ReadMemory(pc);
		bitset<5> Rs, Rt, Rd;
		bitset<32> Rs_value, Rt_value;
		bitset<6> funct;
		bitset<16> imm;
		bitset<26> address;

		// If current insturciton is "11111111111111111111111111111111", then break;
		if (insturciton == bitset<32>(0xFFFFFFFF))
		{
			break;
		}

		// decode(Read RF)
		unsigned Op_code = (insturciton >> 26).to_ulong();
		switch (Op_code)
		{
		case 0x00:
			Rs = bitset<5>(insturciton.to_string(), 6, 5);
			Rt = bitset<5>(insturciton.to_string(), 11, 5);
			Rd = bitset<5>(insturciton.to_string(), 16, 5);
			funct = bitset<6>(insturciton.to_string(), 26, 6);

			myRF.ReadWrite(Rs, Rt, 0, 0, 0);
			Rs_value = myRF.ReadData1;
			Rt_value = myRF.ReadData2;

			switch (funct.to_ulong())
			{
			case 0x21:
				myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(ADDU, Rs_value, Rt_value), 1);
				break;
			case 0x23:
				myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(SUBU, Rs_value, Rt_value), 1);
				break;
			case 0x24:
				myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(AND, Rs_value, Rt_value), 1);
				break;
			case 0x25:
				myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(OR, Rs_value, Rt_value), 1);
				break;
			case 0x26:
				myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(NOR, Rs_value, Rt_value), 1);
				break;
			default:
				break;
			}
			pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;
		case 0x09:
			Rs = bitset<5>(insturciton.to_string(), 6, 5);
			Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);

			myRF.ReadWrite(Rs, 0, 0, 0, 0);
			Rs_value = myRF.ReadData1;
			myRF.ReadWrite(0, 0, Rt, myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), 1);
			pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;
		case 0x04:
			Rs = bitset<5>(insturciton.to_string(), 6, 5);
			Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);

			myRF.ReadWrite(Rs, Rt, 0, 0, 0);
			Rs_value = myRF.ReadData1;
			Rt_value = myRF.ReadData2;
			if (Rs_value == Rt_value)
			{
				pc = myALU.ALUOperation(ADDU, pc, bitset<32>(imm.to_ulong()) << 2);
			}
			pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;
		case 0x23:
			Rs = bitset<5>(insturciton.to_string(), 6, 5);
			Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);

			myRF.ReadWrite(Rs, 0, 0, 0, 0);
			Rs_value = myRF.ReadData1;
			myRF.ReadWrite(0, 0, Rt, myDataMem.MemoryAccess(myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), 0, 1, 0), 1);
			pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;
		case 0x2B:
			Rs = bitset<5>(insturciton.to_string(), 6, 5);
			Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);

			myRF.ReadWrite(Rs, Rt, 0, 0, 0);
			Rs_value = myRF.ReadData1;
			Rt_value = myRF.ReadData2;
			myDataMem.MemoryAccess(myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), Rt_value, 0, 1);
			pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;
		case 0x02:
			address = bitset<26>(insturciton.to_string(), 6, 26);
			pc = myALU.ALUOperation(OR, bitset<32>(address.to_ulong()) << 2, myALU.ALUOperation(AND, myALU.ALUOperation(ADDU, pc, bitset<32>(4)), bitset<32>(0xF0000000)));
			break;
		case 0x3F:
			break;
		default:
			break;
		}

		// pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));

		// Execute

		// Read/Write Mem

		// Write back to RF

		myRF.OutputRF(); // dump RF;
	}
	myDataMem.OutputDataMem(); // dump data mem

	return 0;
}
