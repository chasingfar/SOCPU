
#include "ucode.hpp"
#include <soasm/soisv1.hpp>

using namespace SOASM::SOISv1;

namespace SOCPU::SOARCHv2 {
	template<> uCode::gen_t uCode::gen(Unknown instr) const{
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Init instr) const{
		co_yield init_op();
		co_yield set_minus_one(MReg16::SP);//Reg16::SP=0xFFFF
		co_yield set_zero(MReg16::PC);//Reg16::PC=0x0000
		co_yield load_op();//load op from MEM[0]
	}
	template<> uCode::gen_t uCode::gen(LoadFar instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield add(toM(instr.from),MReg16::TMP,MReg16::TMP);
		co_yield load(MReg16::TMP,MReg::TMA);
		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(SaveFar instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield add(toM(instr.to),MReg16::TMP,MReg16::TMP);
		co_yield stack_pop(MReg::TMA);
		co_yield save(MReg::TMA,MReg16::TMP);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(LoadNear instr) const{
		co_yield load_imm_extend(MReg16::TMP);
		co_yield add(toM(instr.from),MReg16::TMP,MReg16::TMP);
		co_yield load(MReg16::TMP,MReg::TMA);
		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(SaveNear instr) const{
		co_yield load_imm_extend(MReg16::TMP);
		co_yield add(toM(instr.to),MReg16::TMP,MReg16::TMP);
		co_yield stack_pop(MReg::TMA);
		co_yield save(MReg::TMA,MReg16::TMP);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Load instr) const{
		co_yield load(toM(instr.from),MReg::TMA);
		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Save instr) const{
		co_yield stack_pop(MReg::TMA);
		co_yield save(MReg::TMA,toM(instr.to));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(SaveImm instr) const{
		co_yield load_imm(MReg::TMA);
		co_yield save(MReg::TMA,toM(instr.to));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Push instr) const{
		co_yield stack_push(toM(instr.from));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Pop instr) const{
		co_yield stack_pop(toM(instr.to));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Calc instr) const{

#define ARG_1 co_yield stack_pop(MReg::TML);
#define ARG_2 ARG_1 co_yield stack_pop(MReg::TMH);
#define CALC_1(fn,name)  case Calc::FN::fn: ARG_1 co_yield name(MReg::TML,MReg::TMA,0);break;
#define CALC_1C(fn,name) case Calc::FN::fn: ARG_1 co_yield name(MReg::TML,MReg::TMA,arg.CF);break;
#define CALC_2(fn,name)  case Calc::FN::fn: ARG_2 co_yield name(MReg::TMH,MReg::TML,MReg::TMA);break;
#define CALC_2C(fn,name) case Calc::FN::fn: ARG_2 co_yield name(MReg::TMH,MReg::TML,MReg::TMA,arg.CF);break;
		switch (instr.fn){
			CALC_1( SHL,shift_left)
			CALC_1( SHR,shift_right)
			CALC_1C(RCL,shift_left)
			CALC_1C(RCR,shift_right)
			CALC_2( ADD,add)
			CALC_2( SUB,sub)
			CALC_2C(ADC,add)
			CALC_2C(SUC,sub)
		}
#undef ARG_1
#undef ARG_2
#undef CALC_1
#undef CALC_1C
#undef CALC_2
#undef CALC_2C

		co_yield save_carry();
		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Logic instr) const{

#define ARG_1 co_yield stack_pop(MReg::TML);
#define ARG_2 ARG_1 co_yield stack_pop(MReg::TMH);
#define LOGIC_1(fn,name)  case Logic::FN::fn: ARG_1 co_yield name(MReg::TML,MReg::TMA);break;
#define LOGIC_2(fn,name)  case Logic::FN::fn: ARG_2 co_yield name(MReg::TMH,MReg::TML,MReg::TMA);break;
		switch (instr.fn){
			LOGIC_1(NOT,logic_not)
			LOGIC_2(AND,logic_and)
			LOGIC_2(OR ,logic_or )
			LOGIC_2(XOR,logic_xor)
		}
#undef ARG_1
#undef ARG_2
#undef LOGIC_1
#undef LOGIC_2

		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(BranchCF instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield branch(MReg16::TMP);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(BranchZero instr) const{
		co_yield stack_pop(MReg::TMA);
		co_yield load_imm(MReg16::TMP);
		co_yield branch_zero(MReg16::TMP,MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Jump instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield jump(MReg16::TMP);
	}
	template<> uCode::gen_t uCode::gen(ImmVal instr) const{
		co_yield load_imm(MReg::TMA);
		co_yield stack_push(MReg::TMA);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Call instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield inc(MReg16::PC);
		co_yield stack_push(MReg16::PC);
		co_yield jump(MReg16::TMP);
	}
	template<> uCode::gen_t uCode::gen(CallPtr instr) const{
		co_yield stack_pop(MReg16::TMP);
		co_yield inc(MReg16::PC);
		co_yield stack_push(MReg16::PC);
		co_yield jump(MReg16::TMP);
	}
	template<> uCode::gen_t uCode::gen(Return instr) const{
		co_yield stack_pop(MReg16::TMP);
		co_yield jump(MReg16::TMP);
	}
	template<> uCode::gen_t uCode::gen(Adjust instr) const{
		co_yield load_imm(MReg16::TMP);
		co_yield add(MReg16::SP,MReg16::TMP,MReg16::SP);
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Enter instr) const{
		co_yield stack_push(toM(instr.bp));
		co_yield copy(MReg16::SP,toM(instr.bp));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Leave instr) const{
		co_yield copy(toM(instr.bp),MReg16::SP);
		co_yield stack_pop(toM(instr.bp));
		co_yield next_op();
	}
	template<> uCode::gen_t uCode::gen(Halt instr) const{
		co_yield halt();
	}
	template<> uCode::gen_t uCode::gen(INTCall instr) const{
		if(!arg.isINT()){
			co_yield inc(MReg16::PC);
		}
		co_yield stack_push(MReg16::PC);
		co_yield load_imm(MReg16::PC,arg.isINT());
		co_yield jump(MReg16::TMP);
	}

} // SOCPU::SOARCHv2