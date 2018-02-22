# Project_Architecture
Mips Simulator in C++
       
1. Objective 
The objective is to create a simulation code for the multi-cycled implementation of the MIPS 
 
Architecture capable of handling following instructions. 
Arithmetic:                     add, sub 
Data transfer:                lw, sw 
Logical:                           and, or, nor 
Conditional Branch:     beq, bne, slt 
Jump:                              j 
 
 
2. Implementation 
Initial phase starts with the implementation of a 32 bit ALU . The ALU  uses a 32-bit ripple-carry
adder, and performs six operations, i.e., logic AND, logic OR ,  addition, subtraction, set on less 
than, logic NOR. In phase-2 all the components in the datapath are implemented as  functions 
based  and all micro operations are controlled by Control component. 
Control component is designed based on state diagram of the FSM-based control .  
In each clock, control component generates appropriate control signals needed by the 
components in the datapath and simulate micro operations. After designing all the components  
an  object file with object code and data is loaded into simulated memory(struct Memory[200[) 
along with this a register file containing 32 register is loaded (struct reg[32]). Now based on 
MIPS multi-cycle processor the five basic stages that a processor goes through are simulated. 




The following stages are implemented in a loop until opcode!=”111111”(dummy instruction 
stored in simulated memory). 

1. Instruction Fetch (IF) 
In this stage the instruction pointed by Program Counter(PC) is loaded from memory and stored 
in IR register(Intermediate Register) and PC<=PC+4. This is achieved by setting control signals at 
this clock, which sets IorD to 0 which sets 2X1 MUX to 0 allowing PC value to pass through to 
Memory where MemRead  signals enables to read  data from Memory[PC] and store it in IR 
register. PC is incremented by 4 by passing ALUSrcB(01) value to 4X1MUX which returns value 
of 4 and ALUSrcA(0)  is set to 0 by the control signal which allows to select PC from 
2X1MUX(PC,A,ALUSrcA) the value returned by the multiplex are fed to 32 bit ALU and addition 
is done on them and the value returned by 32bitALU is stored in PC.  
2. Instruction Decode/Register Fetch (ID) 
Control signals for this clock  are generated. Opcode is generated at this stage values IR[31-26] 
are opcode, values from IR[25-21] and IR[20-16] are fed to Register Component which reads the 
register file content at these addresses and stores them in A and B registers. For branch, 
instructions  ALUout<=PC+(sign-extend (IR(15:0))<<2) is achieved by passing getting PC value 
from 2X1MUX(PC,A,ALUSrcA) since ALUSrcA=0 and getting value of (sign-extend (IR(15:0))<<2) 
by passing the IR(15:0) through sign extend component which converts 16 bits to 32 bits and 
then through shift left which left shifts the sign extended bit by 2 and then passing these values 
through 4X1Mux(B,4,signextended, sign-extend (IR(15:0))<<2,ALUSrcB) and getting sign-extend 

(IR(15:0))<<2) values since ALUSrcB=”11”. Then passing this value of PC and sign-extend 
(IR(15:0))<<2) to 32 bit ALU and storing the result in ALUout. 
 
3. Execution, address computation, branch/jump Completion(EX) 
Control signals are generated for the opcode and clock and based on opcode following micro 
operations are executed. 
Opcode is R type  
ALUout =<A op B             
  (A from Mux2(PC,A,ALUsrcA)) ,  (B from Mux4(B,4,sign-extend,left-shift,ALUSrcB) 
op is done by 32 bit ALU based on ALUcontrol signal fed to it. 
op can be SUB,AND,OP,ADD,SLT based on function fields and ALUcontrol output to ALU. 
Opcode Memory Reference 
ALUout =<A +sign-extend(IR(15:0) for load and store operations 
Opcode Branches 
If (A==B) 
PC<=ALUout 
Else 
PC<=PC+4; 
 
 
 
Opcode Jump 
PC<=PC[31:28].IR[25:0]]+”00”; 
4. Memory access or R-type Completion(MEM) 
Control Signals are generated for opcode and clock based on opcode either R type or lw/sw 
micro operations are executed. 
R-type: Reg[IR[15:11]<=ALUOut 
Lw- MDR<=Memory[ALUOut] 
Sw –Memory[ALUOut]<=B 
5. Memory read completion (WB) 
Control signal is generated for clock and opcode and only Lw is possible at this stage: 
Lw:Reg[IR[20:16]]<=MDR 
 
 
 
 
 
 
3. Simulator Working 
Create a text file with name object.txt and save it in same directory. 
In object file initial values should be number of Instruction lines (eg. 8 in this project) and 
number of data lines(e.g 3 in this project) your instruction lines should be same as the number 
specified and data lines same as the number of data lines specified in the beginning of text file. 
Now execute the code it will display the initial Memory Content with address and Register file 
contents(reg. #s and contents). 
After the memory configuration it will display Content of PC, IR, MDR, A, B, ALUout after an 
interval of 1 second. 
The Execution will stop when opcode==”111111” after printing IF, ID since at MEM stage 
opcode is used for different types of micro operations. 
The final contents of the updated memory and register file will be  displayed after "111111"
