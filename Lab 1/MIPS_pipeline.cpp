#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
    IFStruct();
};

IFStruct::IFStruct(void) {
    PC = bitset<32> (0);
    nop = 0;
}

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
    IDStruct();
};

IDStruct::IDStruct(void) {
    nop = 1;
}

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;
    EXStruct();
};

EXStruct::EXStruct(void) {
    is_I_type = 0;
    rd_mem = 0;
    wrt_mem = 0;
    alu_op = 1;
    wrt_enable = 0;
    nop = 1;
}

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;
    MEMStruct();
};

MEMStruct::MEMStruct(void) {
    rd_mem = 0;
    wrt_mem = 0;
    wrt_enable = 0;
    nop = 1;
}

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
    WBStruct();
};

WBStruct::WBStruct(void) {
    wrt_enable = 0;
    nop = 1;
}

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 

int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state, newState;
    int cycle;
    cycle = 0;
    bool stall_flag = 1;	
             
    while (1) {

        /* --------------------- WB stage --------------------- */
        if (!state.WB.nop)
        {
            if (state.WB.wrt_enable)
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
            if (state.MEM.nop)
            {
                newState.WB.nop = state.MEM.nop;
            }
        }
        

        /* --------------------- MEM stage --------------------- */
        if (!state.MEM.nop)
        {
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

            if (state.MEM.rd_mem)           // lw
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
                newState.WB.nop = 0;
            }
            else if (state.MEM.wrt_mem)     // sw
            {
                // RAW Hazard: MEM-MEM
                if (state.WB.wrt_enable)
                {
                    if (state.WB.Wrt_reg_addr == state.MEM.Rt)
                    {
                        state.MEM.Store_data = state.WB.Wrt_data;
                    }
                }
                
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                // TODO: check WB.Wrt_data
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
                newState.WB.nop = 0;
            }
            else                            // addu & subu & beq
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
                newState.WB.nop = 0;
            }

            if (state.EX.nop)
            {
                newState.MEM.nop = state.EX.nop;
            }
        }


        /* --------------------- EX stage --------------------- */
        if (!state.EX.nop)
        {
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.Store_data = state.EX.Read_data2;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;

            // RAW Hazards: MEM-EX
            if (state.MEM.wrt_enable)
            {
                if (state.MEM.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = newState.WB.Wrt_data;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = newState.WB.Wrt_data;
                    newState.MEM.Store_data = state.EX.Read_data2;
                }
            }

            // RAW Hazards: MEM-EX
            if (state.WB.wrt_enable)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                    newState.MEM.Store_data = state.EX.Read_data2;
                }
            }

            if (!state.EX.is_I_type)
            {
                if (state.EX.alu_op)    // addu
                {
                    newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
                }
                else                    // subs
                {
                    newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
                }
            }
            else
            {
                bitset<32> Offset = bitset<32>(state.EX.Imm.to_ulong());
                if (state.EX.alu_op)
                {
                    if (state.EX.Imm[15] == 1)
                    {
                        for (int i = 31; i > 15; i--)
                        {
                            Offset[i] = 1;
                        }
                    }

                    if (state.EX.rd_mem)    // lw
                    {
                        newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + bitset<32>(Offset.to_ulong()).to_ulong());
                    }
                    else                    // sw
                    {
                        newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + bitset<32>(Offset.to_ulong()).to_ulong());
                    }  
                }
            }
            newState.MEM.nop = 0;

            if (state.ID.nop)
            {
                newState.EX.nop = state.ID.nop;
            }   
        }
          

        /* --------------------- ID stage --------------------- */
        if (!state.ID.nop)
        {
            newState.EX.Rs = bitset<5>(state.ID.Instr.to_string(), 6, 5);
            newState.EX.Rt = bitset<5>(state.ID.Instr.to_string(), 11, 5);
            newState.EX.Imm = bitset<16>(state.ID.Instr.to_string(), 16, 16);
            newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
            newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

            unsigned Op_code = (state.ID.Instr >> 26).to_ulong();
            switch (Op_code)
            {
            case 0x00:
                newState.EX.Wrt_reg_addr = bitset<5>(state.ID.Instr.to_string(), 16, 5);
                newState.EX.is_I_type = 0;
                newState.EX.wrt_enable = 1;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;
                switch (bitset<6>(state.ID.Instr.to_string(), 26, 6).to_ulong())
                {
                case 0x21:
                    newState.EX.alu_op = 1;
                    newState.EX.nop = 0;
                    break;
                
                case 0x23:
                    newState.EX.alu_op = 0;
                    newState.EX.nop = 0;
                    break;
                
                default:
                    break;
                }
                break;
            case 0x23:              // lw
                newState.EX.Wrt_reg_addr = newState.EX.Rt;
                newState.EX.alu_op = 1;
                newState.EX.is_I_type = 1;
                newState.EX.wrt_enable = 1;
                newState.EX.rd_mem = 1;
                newState.EX.wrt_mem = 0;
                newState.EX.nop = 0;
                break;

            case 0x2B:              // sw
                newState.EX.Wrt_reg_addr = newState.EX.Rt;
                newState.EX.alu_op = 1;
                newState.EX.is_I_type = 1;
                newState.EX.wrt_enable = 0;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 1;
                newState.EX.nop = 0;
                break;

            case 0x04:              // beq
                newState.EX.Wrt_reg_addr = newState.EX.Rt;
                newState.EX.alu_op = 0;
                newState.EX.is_I_type = 1;
                newState.EX.wrt_enable = 0;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;

                if (newState.EX.Read_data1 != newState.EX.Read_data2)
                {
                    bitset<32> BranchAddr;

                    // SignExtend
                    if (newState.EX.Imm[15] == 1)
                    {
                        BranchAddr = bitset<32>(newState.EX.Imm.to_ulong()) << 2;
                        for (int i = 31; i > 17; i--)
                        {
                            BranchAddr[i] = 1;
                        }
                    }
                    else
                    {
                        BranchAddr = bitset<32>(newState.EX.Imm.to_ulong()) << 2;
                    }

                    state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + BranchAddr.to_ulong());
                }
                newState.EX.nop = 0;
                break;

            default:
                break;
            }

            if (newState.MEM.rd_mem && !newState.EX.is_I_type)
            {
                if (stall_flag)
                {
                    if (newState.MEM.Wrt_reg_addr == newState.EX.Rs || newState.MEM.Wrt_reg_addr == newState.EX.Rt)
                    {
                        newState.EX.nop = 1;
                        state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
                        stall_flag = 0;
                    }
                }
                else
                {
                    stall_flag = 1;   
                } 
            }
        }
        

        /* --------------------- IF stage --------------------- */
        if (!state.IF.nop)
        {
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            if (newState.ID.Instr == bitset<32>(0xFFFFFFFF))
            {
                newState.IF.nop = 1;
                newState.ID.nop = 1;
            }
            else
            {
                newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
                newState.IF.nop = 0;
                newState.ID.nop = 0;
            } 
        }
        
             
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
        cycle += 1;

        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
                	
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}