 //simulator for MIPS Multi-cycled CPU Simulation Project.

#include<iostream>
#include<fstream>//input from file
#include<string>//string operations
#include<sstream>
#include<iomanip>//for set(w)
#include<windows.h>

using namespace std;

///13 control signals are declared global.
int PCWriteCond;
int PCWrite;
int IorD;
int MemRead;
int MemWrite;
int MemtoReg;
int IRWrite;
string PCSource;
string ALUOp;
string ALUSrcB;
int ALUSrcA;
int RegWrite;
int RegDst;

int clock;//used for generating clock signal
int Set=0;//Used for SLT(operation)

int binarytodecimal(string);//converts binary to decimal
void display();//display intermediate registers e.g PC,MDR
void displaymem();//display memory content and register content (along with register names)
int AND(int,int);//AND operation
int OR(int,int);//OR operation
int mux_2X1(int,int,int);//MUX 2x1
int mux_4X1(int,int,int,int,string);//MUX 4X1
int negation(int x);//negation of a bit

int _1_bitAlu(int,int,int,int,int,int,string,int&);//1 bit ALU
int _1_bitAlu_OF(int,int,int,int,int,int,string,int&,int&);//1 bit ALU  for overflow
int bit_full_adder(int,int,int,int&);//1 bit full Adder
int* _32_bitALU(int*,int*,int,int,string,int&,int&);//32 bitALU

string memory(string,string,int,int);//Representation of Memory
void instructionRegister(string,string&,string&,string&,string& ,int);//Representation of Instruction Register
void  registers(string ,string ,string ,string ,string& ,string& ,int );//Representation of Registers
string sign_extend(string);//convert 16 bit to 32 bit
string shift_left2(string);//left shift by 2
string shift_left_26(string);//left shift and  convert 26 bit to 28 bit
string ALUcontrol(string,string);//Controls operations of ALU
void loader();//loads object code and data into simulated memory along with a 32 bit dummy final instruction
void control(string);//control operation simulation

string mux_1(string,string,int);//selects memory address for instruction(PC) or data(ALUout) based on IorD.
string mux_2(string,string,int);//selects write-register number from rt(for load) and rd(for R type) based on RegDst
string mux_3(string,string,int);//selects write-data from ALUour(R type) and MDR(for data),based on MemtoReg
int* mux_4(string a,string b,int c);//selects ALU-operand-1 from PC and A,based on ALUSrcA.
int* mux_5(string,string,string,string,string);//selects ALU-operand-2 from B,#4,sign-extended offset(for load/store.
string mux_6(string,string,string,string);//selects the next PC value from ALU(PC+4),ALUout(branch target) and Jump based on PCSource

string PC="00000000000000000000000000000000";//Initial value of PC
string IR;
string MDR;
string A;
string B;
string  ALUout;
 int of;
 int gp=100;//the data starts from this address
int dataload=gp;//used by loader()

/** Data Structure for Memory Simulation*/
struct mem
{
string memData;//memory content
}Memory[200];

int d=0;

/**Register Simulation */
struct r
{
string  reg_name;//register name e.g $to,$t1
string content;//content
 }reg[32];

fstream f1;

/**Converts array to corresponding string **/
string arraytostring(int* x,int n)
{
ostringstream oss;
for(int i=0;i<n;i++)
{
    oss<<x[i];

}
return oss.str();

}

/**Converts decimal number to 32 bit binary*/
string decimaltobinary(int n)
{

int s[32];
for(int i=31;i>=0;i--)
{   if(n%2==1)
    s[i]=1;
    else
        s[i]=0;

        n=n/2;
}
return arraytostring(s,32);


}



/** main function where every operations are executed based on opcode and clock**/
int main()
{
    int n=0;
    int opalu;
    int zero;
    loader();//loads object code and data into simulated memory
    displaymem();//display initial configuration of memory and register file

    string sign_ext;
    string instr_0_5;//function field used by ALUcontrol
    string instr_0_25;//used by mux_6 for jump target
    string opcode="000000";

    while(opcode!="111111")//opcode is not equal to 63(dummy final instruction
    {
        clock=n%5;//generate clock signal
        cout<<"--------------------------------------------------------------------"<<endl;
        if(clock==0)//IF STAGE
        {
        cout<<endl;
        cout<<"IF STAGE"<<endl;
        cout<<endl;
        control(opcode);//generates control signal based on clock
        string addr=mux_1(PC,ALUout,IorD);
        IR=memory(addr,B,MemRead,MemWrite);//IR<=Memory[PC]
        string add4="00000000000000000000000000000100";//#4
        int* pc;//since return type is a pointer
        pc=_32_bitALU(mux_4(PC,A,ALUSrcA),mux_5(B,add4,"0","0",ALUSrcB),0,0,"10",opalu,zero);//PC<=PC+4
        PC=arraytostring(pc,32);
        display();//display content of Intermediate Registers
        n++;
        }

        if(clock==1)//ID STAGE
        {
            string rs,rt,rd;
            cout<<endl;
            cout<<"ID STAGE"<<endl;
            cout<<endl;
            instructionRegister(IR,opcode,rs,rt,rd,IRWrite);//opcode,rs,rt,rd are decoded

            control(opcode);//based on opcode and clock clock signals are generated

            string write_register=mux_2(rt,rd.substr(0,5),RegDst);
            string write_data1=mux_3(ALUout,MDR,MemtoReg);

             registers(rs,rt,write_register,write_data1,A,B,RegWrite);//A<=reg[IR[25:21]] B<=reg[IR[20:16]]
             sign_ext=sign_extend(rd);//sig_ext used immediate(lw/sw)
             instr_0_5=rd.substr(10,6);
             instr_0_25=IR.substr(6,26);
             string add4="00000000000000000000000000000100";//#4

            int* ab=mux_4(PC,A,ALUSrcA);//based on ALUSrcA chooses PC or A
            int* ab1=mux_5(B,add4,sign_extend(rd),shift_left2(sign_extend(rd)),ALUSrcB);//based on ALUSrcB chooses among 4
            string cont=ALUcontrol(instr_0_5,ALUOp);//control signal for ALU
            int a1=cont[0]-'0';//ALUOp1
            int b1=cont[1]-'0';//ALUop0

            string op=cont.substr(2,2);//op for ADD
            int of;
            int* pc;
            pc=_32_bitALU(ab,ab1,a1,b1,op,of,zero);//ALUOut<=PC+(sign-extend(IR[15:0])<<2)
            ALUout=arraytostring(pc,32);//store in ALUout
            display();

            n++;

        }

        if(clock==2)
        {
        cout<<endl;
        cout<<"EX STAGE"<<endl;
        cout<<endl;

            if(opcode=="000000")//R type
             {
                cout<<"R Type"<<endl;
               control(opcode);//set control signal for clock and R type
               int* a=mux_4(PC,A,ALUSrcA);//get A

               string add4="00000000000000000000000000000100";
               int* b=mux_5(B,add4,sign_ext,PC,ALUSrcB);//get B
               cout<<"Instr 0_5:"<<instr_0_5<<endl;//function field
               string cont=ALUcontrol(instr_0_5,ALUOp);//control signal for ALU
               int a1=cont[0]-'0';//ALUop1
               int b1=cont[1]-'0';//ALUop2

               string op=cont.substr(2,2);//op for SUB,ADD,ASLT,OR
               int* alu=_32_bitALU(a,b,a1,b1,op,of,zero);//ALUOut<=AopB
               ALUout=arraytostring(alu,32);//Store in ALU
               display();

             }

            else if(opcode=="100011" || opcode=="101011")//Memory
             {
                 control(opcode);//set control signal for clock for Memory Reference Instruction
                cout<<"LW && SW Type"<<endl;

               int* a=mux_4(PC,A,ALUSrcA);//get A
               string add4="00000000000000000000000000000100";
               int* b=mux_5(B,add4,sign_ext,PC,ALUSrcB);//get sign-extend[IR[15:0]]
               string cont=ALUcontrol(instr_0_5,ALUOp);//control signal for ALU
               int a1=cont[0]-'0';//ALUop1
               int b1=cont[1]-'0';//ALUop0
               string op=cont.substr(2,2);//op for Add

                int of;

                int* ALU=_32_bitALU(a,b,a1,b1,op,of,zero);//ALUOut<=A+sign-extend[IR[15:0]];
                ALUout=arraytostring(ALU,32);//store in ALU
                display();

                }
            else if(opcode=="000100" || opcode=="000101")//branches
             {
                control(opcode);//set control signal for clock for Branches
                cout<<"Branch Type"<<endl;
               int* a=mux_4(PC,A,ALUSrcA);//get A
               string add4="00000000000000000000000000000100";
               int* b=mux_5(B,add4,sign_ext,PC,ALUSrcB);//get B
               string cont=ALUcontrol(instr_0_5,ALUOp);//control signal for ALU
               int a1=cont[0]-'0';//ALUop1
               int b1=cont[1]-'0';//ALUop0
               string op=cont.substr(2,2);

               int of;
               int* ALU=_32_bitALU(a,b,a1,b1,op,of,zero);//return zero=1 if (A==B)

               string s_28=shift_left2(instr_0_25);//converts 26 bit instruction[25-0] to 28 bit
               string pc_31=PC.substr(0,4);//PC[31-28]
               string pcExtended=pc_31+s_28;//PC[31-28].(IR25:0),2'B00)]
               string aluAfterOp=arraytostring(ALU,32);


              if(zero==1)
              {
               PC=mux_6(aluAfterOp,ALUout,pcExtended,PCSource);//PC<=ALUOut since PCSource=01
                display();
               }
              else
              {
               int* pc1;
               pc1=_32_bitALU(mux_4(PC,A,0),mux_5(B,add4,"0","0","01"),0,0,ALUOp,opalu,zero);//PC+4
               PC=arraytostring(pc1,32);//PC<=PC+4
               display();
               }

             }


            else if(opcode=="000010")//jump
             {
              control(opcode);//generate control signal for Jump and clock
              cout<<"Jump type"<<endl;

              string s_28=shift_left_26(instr_0_25);//converts 26 bit instruction[25-0] to 28 bit

              string pc_31=PC.substr(0,4);//PC[31-28]
              string pcExtended=pc_31+s_28;//PC[31-28].(IR25:0),2'B00)]

              PC=mux_6("","",pcExtended,PCSource);//PC=<PC[31-28].(IR25:0),2'B00)] since ALUSource=10

             display();



             }
            n++;

        }

        if(clock==3)
        {
           if(opcode=="000000")//R type
             {
             control(opcode);//generate control signals for R type and clock
             cout<<"MEM"<<endl;
             string wr=mux_2(IR.substr(11,5),IR.substr(16,5),RegDst);//set write register from mux_2
               registers("","",wr,ALUout,A,B,RegWrite);//Reg[IR[15:11]]<=ALUOut
               display();

             }

          else  if(opcode=="100011" )//lw
             {
              control(opcode);//generate control signal for lw type
             cout<<"LW in Mem"<<endl;
            MDR=Memory[binarytodecimal(ALUout)].memData+Memory[binarytodecimal(ALUout)+1].memData+Memory[binarytodecimal(ALUout)+2].memData+Memory[binarytodecimal(ALUout)+3].memData;
            //MDR<=Memory[ALUout]
            display();


             }
           else  if(opcode=="101011")//sw
             {
              control(opcode);//generate control signal for sw and clock
             cout<<"SW in Mem"<<endl;

             string ab=mux_1(PC,ALUout,IorD);//ALUOut since IorD=1
             cout<<"ALUout at SW:"<<ab<<" //"<<binarytodecimal(ab)<<endl;
             string ax=memory(ab,B,MemRead,MemWrite);//Memory[ALUout]<=B

              display();

             }

            n++;
        }
        if((clock==4))
        {
        control(opcode);//generate control signals for lw and clock
       if(opcode=="100011")
       {cout<<"WB in WB"<<endl;
        string write_data=mux_3(ALUout,MDR,MemtoReg);//get MDR since MemtoReg=1

        string wr=mux_2(IR.substr(11,5),IR.substr(16,5),RegDst);//get IR[20:16] since RegDst=0
        registers("0","0",wr,write_data,A,B,RegWrite);//Reg[IR[20:16]<=MDR

       display();//display intermediate registers

        }
        n++;
        }
   cout<<"--------------------------------------------------------------------"<<endl;
        }
cout<<endl;
cout<<"Since opcode is detected at MEM it exit after ID:"<<endl;
cout<<endl;
displaymem();//display memory and register file after update


return 0;
}

/**function return 32 bit binary from a 16 bit Instruction fed to it    **/
string sign_extend(string q)
{
string a=q;
string b;

  if(a[0]=='0')
{
    for(int i=0;i<16;i++)
    b+='0';
}
else
{
    for(int i=0;i<16;i++)
    b+='1';

}
for(int j=16;j<32;j++)
{
    b+=a[j-16];
}
return b;//return 32 bit binary

}


/** Left shift binary string by 2 bits **/
string shift_left2(string q)
{
   string a=q;
   int l=a.length();
    a[l-1]='0';
    a[l-2]='0';
   for(int i=l-3;i>=0;i--)
    a[i]=q[i+2];

return a;//return left shifted binary


}

/**Left shift binary string by 2 bits and add 2 '00' bits at the end to make it 28 bits**/
string shift_left_26(string q)
{
   string a=q;
   int l=a.length();
    a[l-1]='0';
    a[l-2]='0';

   for(int i=l-3;i>=0;i--)
    a[i]=q[i+2];

   string k="00";
   a=k+a;

    return a;//return left shifted binary of 28 bits

}

/** Generate control signal based on the FSM Diagram based on clock and opcode**/

void control(string _op)
{   string optype;
    if(_op=="000000")
    {
        optype="R";
    }
    else if(_op=="100011")
    {
        optype="LW";
    }
    else if(_op=="101011")
    {
        optype="SW";
    }
    else if(_op=="000100" || _op=="000101")
    {
        optype="BEQ";
    }
    else if(_op=="000010")
    {
        optype="J";
    }


    switch(clock)//based on clock
    {
    case 0:
        MemRead=1;
        ALUSrcA=0;
        IorD=0;
        IRWrite=1;
        ALUSrcB="01";
        ALUOp="00";
        PCWrite=1;
        PCSource="00";
        RegWrite=0;//
        MemWrite=0;//
        PCWrite=1;//
        PCWriteCond=0;
        break;


    case 1:
        MemRead=0;//
        PCWrite=0;//
        IRWrite=0;//
        ALUSrcA=0;
        ALUSrcB="11";
        ALUOp="00";
        break;

    case 2:
        if((optype=="LW") or(optype=="SW"))//based on opcode LW/SW
           {

               ALUSrcA=1;
               ALUSrcB="10";
               ALUOp="00";
           }

           else if(optype=="R")//based on opcode R
            {
                ALUSrcA=1;
                ALUSrcB="00";
                ALUOp="10";

            }
             else if(optype=="BEQ")//based on opcode BEQ
            {
                ALUSrcA=1;
                ALUSrcB="00";
                ALUOp="01";
                PCWriteCond=1;
                PCSource="01";

            }
             else if(optype=="J")//based on opcode Jump
            {
                PCWrite=1;
                PCSource="10";

            }

    break;

    case 3:


        if(optype=="LW")
        {

            MemRead=1;
            IorD=1;
            MemWrite=0;
        }


        else if(optype=="SW")
        {

            MemWrite=1;
            IorD=1;
            MemRead=0;
        }

        else if(optype=="R")
        {
            RegDst=1;
            RegWrite=1;
            MemtoReg=0;

        }
    break;


    case 4:

        RegDst=0;
        RegWrite=1;
        MemtoReg=1;

    break;


    }


}

/**loads object code and data into simulated memory**/
void loader()
{
f1.open("object.txt");
if(!f1.is_open())
{
    cout<<"Unable to open file";

}
else
{   int data;//number of data lines 3 in this case
    int inst;//number of instruction 8 in this case
    f1>>inst>>data;

    /**Stores instruction in struct Memory from 0-32**/
    for(int i=0;i<inst;i++)
    {
        string s;
        f1>>s;
        string temp=s.substr(0,8);//memoryWrite
        Memory[MemWrite].memData=temp;
        MemWrite++;
        string temp1=s.substr(8,8);//memoryWrite
        Memory[MemWrite].memData=temp1;
        MemWrite++;
        string temp2=s.substr(16,8);//memoryWrite
        Memory[MemWrite].memData=temp2;
        MemWrite++;
        string temp3=s.substr(24,8);//memoryWrite
        Memory[MemWrite].memData=temp3;
        MemWrite++;
    }
   /**stores 32 bit dummy op**/
   Memory[MemWrite].memData="11111100";//dummy no_op
   Memory[MemWrite+1].memData="00000000";
   Memory[MemWrite+2].memData="00000000";
   Memory[MemWrite+3].memData="00000000";


   /**Stores data at location starting at 100**/
    for(int j=0;j<data;j++)
    {
     string d;
     f1>>d;

     string temp=d.substr(0,8);
     Memory[dataload].memData=temp;
     dataload++;
      string temp1=d.substr(8,8);
     Memory[dataload].memData=temp1;
     dataload++;
      string temp2=d.substr(16,8);
     Memory[dataload].memData=temp2;
     dataload++;
      string temp3=d.substr(24,8);
     Memory[dataload].memData=temp3;
     dataload++;

    }

}
/**Register names at corresponding register position*/
reg[0].reg_name="$zero";
reg[1].reg_name="$at";
reg[2].reg_name="$v0";
reg[3].reg_name="$v1";
reg[4].reg_name="$a0";
reg[5].reg_name="$a1";
reg[6].reg_name="$a2";
reg[7].reg_name="$a3";
reg[8].reg_name="$t0";
reg[9].reg_name="$t1";
reg[10].reg_name="$t2";
reg[11].reg_name="$t3";
reg[12].reg_name="$t4";
reg[13].reg_name="$t5";
reg[14].reg_name="$t6";
reg[15].reg_name="$t7";
reg[16].reg_name="$s0";
reg[17].reg_name="$s1";
reg[18].reg_name="$s2";
reg[19].reg_name="$s3";
reg[20].reg_name="$s4";
reg[21].reg_name="$s5";
reg[22].reg_name="$s6";
reg[23].reg_name="$s7";
reg[24].reg_name="$t8";
reg[25].reg_name="$t9";
reg[26].reg_name="$k0";
reg[27].reg_name="$k1";
reg[28].reg_name="$gp";
reg[29].reg_name="$sp";
reg[30].reg_name="$fp";
reg[31].reg_name="$ra";
for(int i=0;i<32;i++)
{
    reg[i].content="00000000000000000000000000000000";
    if(i==28)
    {
      reg[28].content="00000000000000000000000001100100";//loads gp at this point since in simulated memory data starts at gp=100
    }
}

}


/**Convert binary string to decimal**/
int binarytodecimal(string n)
{
    string num=n;
    int dec=0;

    int base=1;

    int l=n.length();

    for(int i=l-1;i>=0;i--)
    {
        if(num[i]=='0')
        {
            dec+=0*base;
            base=base*2;
        }
        else
        {

            dec+=1*base;
            base=base*2;
        }

    }

    return dec;//return decimal

}

//selects memory address for instruction(PC) or data(ALUout) based on IorD.
string mux_1(string a,string b,int c)//a=pc and b=aluout
{

    if(c==0)
        return a;
    else if(c==1)
        return b;
}

//selects write-register number from rt(for load) and rd(for R type) based on RegDst
string mux_2(string a,string b,int c)
{

    if(c==0)
        return a;
    else if(c==1)
        return b;
}

//selects write-data from ALUour(R type) and MDR(for data),based on MemtoReg
string mux_3(string a,string b,int c)
{

    if(c==0)
        return a;
    else if(c==1)
        return b;
}

//selects ALU-operand-1 from PC and A,based on ALUSrcA
int* mux_4(string a,string b,int c)
{

    if(c==0)
    {
        int* x=new int[a.length()];
        for(int i=0;i<a.length();i++)
        {
         x[i]=a[i]-'0';

        }return x;
    }
    else if(c==1)
    {
    int* y=new int[b.length()];
     for(int i=0;i<b.length();i++)
        {
        y[i]=b[i]-'0';
        }
        return y;
}
}

//selects ALU-operand-2 from B,#4,sign-extended offset(for load/store).
int* mux_5(string a,string b,string c,string d,string control)
{

    if(control=="00")
    {
        int* x=new int[a.length()];
        for(int i=0;i<a.length();i++)
        {x[i]=a[i]-'0';

        }return x;
    }
    else if(control=="01")
    {

    int* y=new int[b.length()];
     for(int i=0;i<b.length();i++)
        {y[i]=b[i]-'0';

    }
        return y;
    }
    else if(control=="10")
    {

    int* w=new int[c.length()];
     for(int i=0;i<c.length();i++)
        {w[i]=c[i]-'0';

    }
        return w;
    }

     else if(control=="11")
    {

    int* t=new int[d.length()];
     for(int i=0;i<d.length();i++)
        {t[i]=d[i]-'0';

    }
        return t;
    }
}


//selects the next PC value from ALU(PC+4),ALUout(branch target) and Jump based on PCSource
string mux_6(string a,string b,string c,string d)
{if(d=="00")
    {
       return a;
    }
    else if(d=="01")
    {
       return b;
    }
    else if(d=="10")
    {
     return c;

    }


}

/**simulate AND gate**/
int AND(int x,int y)
{
 return x*y;

}

/**simulate OR gate**/
int OR(int x,int y)
{
 if(x==1 && y==1)
   return 1;

   else
    return x+y;

}


/**simulate 1-bit Full Adder gate**/
int bit_full_adder(int a,int b,int cini,int& coout)
{

    coout=a*b+cini*(a^b);
    int sum=(a^b)^cini;
     return sum;

}

/** Mux 2X1 simulator **/
int mux_2X1(int x,int y,int control)
{
    if(control==0)
        return x;

    else if(control==1)
        return y;

}

/** MUX 4X1 simulator **/
int mux_4X1(int x1,int x2,int x3,int x4,string control)
{
    int x;
    if(control=="00")
        x=0;
    else if(control=="01")
        x=1;
    else if(control=="10")
        x=2;
    else if(control=="11")
        x=3;
    switch(x)
    {
    case 0: return x1;//and
            break;
    case 1: return x2;//or
            break;
    case 2: return x3;//1 bit full adder
            break;
    case 3: return x4;//less
            break;

    default:cout<<"Error input in MUX 4X1"<<endl;
    }

}

/**negation of x **/
int negation(int x)
{
    if(x==0)
        return 1;
    else if(x==1)
        return 0;

}


/**_1_bitAlu simulation takes 1-bit a,b ainv,binv,less,op returns cout used for bits 0-30**/
int _1_bitAlu(int a,int b,int Cin,int Ainv,int Binv,int Less,string op,int& Cout)
{

a=mux_2X1(a,negation(a),Ainv);//0 or 1 based on Ainv
b=mux_2X1(b,negation(b),Binv);//0 or 1 based on Binv
int z=AND(a,b);
int q=OR(a,b);
int t=bit_full_adder(a,b,Cin,Cout);
return mux_4X1(z,q,t,Less,op);//based on op returns 1-bit value


}


/**_1_bitAlu simulation takes 1-bit a,b ainv,binv,less,op returns cout overflow bit(OF) used for bit 31**/
int _1_bitAlu_OF(int a,int b,int Cin,int Ainv,int Binv,int Less,string op,int& Cout,int& OF)
{

a=mux_2X1(a,negation(a),Ainv);
b=mux_2X1(b,negation(b),Binv);
int z=AND(a,b);
int q=OR(a,b);
int c=Cin;
int t=bit_full_adder(a,b,Cin,Cout);
if((c^Cout)==1)
{
    OF=1;
}
else if((c^Cout)==0)
    OF=0;

Set=t;//used for slt operation declared global

return mux_4X1(z,q,t,Less,op);

}




/**_32_bitAlu simulation takes 32-bit a,b , values ainv,binv,op returns zero and of(overflow) returns 32 bit output**/
int* _32_bitALU(int* A,int* B,int ainv,int binv,string op,int& of,int& zero)
{

 static int Cin=0;
 int Cout;
 int* result= new int[32];
 int* resultSlt= new int[32];
 int Less=0;


 for(int i=31;i>=1;i--)
 {   if(i==31 && op=="10" && binv==1)//for subtract
     {
      result[i]=_1_bitAlu(A[i],B[i],1,ainv,binv,Less,op,Cout);//cin is set to 1 for subtract case to work
      Cin=Cout;
      continue;
     }
     else if(i==31)//for other operations
     {
      result[i]=_1_bitAlu(A[i],B[i],Cin,ainv,binv,Set,op,Cout);
      Cin=Cout;

     }

     result[i]=_1_bitAlu(A[i],B[i],Cin,ainv,binv,Less,op,Cout);
     Cin=Cout;
 }
  result[0]=_1_bitAlu_OF(A[0],B[0],Cin,ainv,binv,Less,op,Cout,of);

  if(op=="11")//for slt operation
  result[31]=_1_bitAlu(A[31],B[31],Cin,ainv,binv,Set,op,Cout);//set Set bit 1 if b is greater than a

  int l=result[0];
  for(int i=1;i<=31;i++)
  {
    l=OR(l,result[i]);//or operation to find zero bit
   }
if(l==1)
zero=0;//return 0 if result is not zero
else
zero=1;//return 1 if result is zero

int CoutSlt,CinSlt=0,ofSlt;

/**Specially done for SLT operation it subtract A,B if A-B=0 then set bit for 0 bit has to be set as 0**/
for(int i=31;i>=0;i--)
{
if(i==31)
{
    resultSlt[i]=_1_bitAlu(A[i],B[i],1,0,1,0,"10",CoutSlt);
    CinSlt=CoutSlt;
    continue;
}
resultSlt[i]=_1_bitAlu(A[i],B[i],CinSlt,0,1,0,"10",CoutSlt);
CinSlt=CoutSlt;
if(i==0)
{
resultSlt[0]=_1_bitAlu_OF(A[0],B[0],CinSlt,0,1,0,"10",CoutSlt,ofSlt);
}
}

if((binarytodecimal(arraytostring(resultSlt,32)))==0)//if A-B=0
{
    if(op=="11")//op is for SLT
    {
        result[31]=_1_bitAlu(A[31],B[31],Cin,ainv,binv,0,op,Cout);//set 0 for less at 0 bit
    }
}


  return result;//return 32 bit result

}


/**Simulation of Instruction Register takes IR as input returns opcode,rs,rt,rd,based on control signal IRWrite**/
void instructionRegister(string n,string& i3,string& i2,string& i1,string& i0,int w)
{

    int len=n.length();
    if(w==1){
    i3=n.substr(0,6);//returns opcode
    i2=n.substr(6,5);//returns rs
    i1=n.substr(11,5);//returns rt
    i0=n.substr(16,16);//returns rd
    }

}

/** Simulation of Memory Module takes PC,ALUout as input and based on MemRead and MemWrite returns value and
    also writes to memory file based on ALUout.
**/
string memory(string a,string b,int x,int y)
{
    if(x==1)
    {   int ad=binarytodecimal(a);
        return Memory[ad].memData+Memory[ad+1].memData+Memory[ad+2].memData+Memory[ad+3].memData;
    }
    else if(y==1)
    {   int ad=binarytodecimal(a);
        Memory[ad].memData=b.substr(0,8);
        Memory[ad+1].memData=b.substr(8,8);
        Memory[ad+2].memData=b.substr(16,8);
        Memory[ad+3].memData=b.substr(24,8);

    }

}

/**
   Simulates Register takes input rs,rt and (write register,write data based on control signals RegWrite)
   set values to intermediate register A,B.
**/
void  registers(string a,string b,string write_reg,string write_data,string& x,string& y,int q)
{
    int readreg1=binarytodecimal(a);//rs
    int readreg2=binarytodecimal(b);//rt

    if(q!=1)
{

    A=reg[readreg1].content;//A<=Reg[IR[25:21]]
    B=reg[readreg2].content;//B<=Reg[IR[20:16]]

}

   if(q==1)
     {

         int readreg3=binarytodecimal(write_reg);
         reg[readreg3].content=write_data;


    }

}

/** ALUcontrol module based on ALUOp1,ALUop0 and function field F5-F4-F3-F2-F1-F0 generates operations **/
string ALUcontrol(string a,string b)
{
   string a1=a.substr(2,4);

    if(b=="00")
        return "0010";
    else if(b=="01")
        return "0110";
    else if(b=="10" && a1=="0000")
        return "0010";
    else if((b=="10" || b=="11") && a1=="0010")
        return "0110";
    else if(b=="10" && a1=="0100")
        return "0000";
    else if(b=="10" && a1=="0101")
        return "0001";
    else if((b=="10" || b=="11" )&& a1=="1010")
        return "0111";


}


//Display Content of Intermediate Registers in 32 bit format along with decimal values
void display()
{   Sleep(1000);
    cout<<setw(8)<<"PC:"<<PC<<setw(3)<<"//"<<binarytodecimal(PC)<<endl;
    cout<<setw(8)<<"IR:"<<IR<<setw(3)<<endl;
    cout<<setw(8)<<"MDR:"<<MDR<<setw(3)<<"//"<<binarytodecimal(MDR)<<endl;
    cout<<setw(8)<<"A:"<<A<<setw(3)<<"//"<<binarytodecimal(A)<<endl;
    cout<<setw(8)<<"B:"<<B<<setw(3)<<"//"<<binarytodecimal(B)<<endl;
    cout<<setw(8)<<"ALUout:"<<setw(6)<<ALUout<<"//"<<binarytodecimal(ALUout)<<endl;




}

//Display Content of Memory with Address along with the register file content(reg.# and contents).
void displaymem()
{
    cout<<"Memory Content with Address:"<<endl;
    for(int i=0;i<200;i++)
    cout<<setw(4)<<i<<"  "<<Memory[i].memData<<endl;
    cout<<endl;
    cout<<"Register File Content"<<endl;
    cout<<endl;
    Sleep(1000);
    for(int i=0;i<32;i++)
    cout<<setw(6)<<reg[i].reg_name<<" "<<reg[i].content<<endl;
    cout<<endl;


}
