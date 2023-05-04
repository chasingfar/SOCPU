#include "ucode.hpp"

using namespace SOCPU::SOARCHv2;

uCode::gen_t uCode::halt() const{
	co_yield ctl.nop();
}
uCode::gen_t uCode::copy(MReg to,MReg from) const{
	co_yield ctl.copy(to,from);
}
uCode::gen_t uCode::copy(MReg16 to,MReg16 from) const{
	co_yield copy(toL(to), toL(from));
	co_yield copy(toH(to), toH(from));
}

uCode::gen_t uCode::inc(MReg to,MReg from) const{
	co_yield ctl.calc(ALU::inc(),to,from);
}
uCode::gen_t uCode::inc(MReg to, MReg from, Carry carry) const{
	co_yield ctl.calc(ALU::inc(carry),to,from);
}
uCode::gen_t uCode::inc(MReg reg) const{
	co_yield inc(reg,reg);
}
uCode::gen_t uCode::inc(MReg reg, Carry carry) const{
	co_yield inc(reg, reg, carry);
}
uCode::gen_t uCode::inc(MReg16 reg16) const{
	co_yield inc(toL(reg16));
	co_yield inc(toH(reg16),arg.carry);
}

uCode::gen_t uCode::dec(MReg to,MReg from) const{
	co_yield ctl.calc(ALU::dec(),to,from);
}
uCode::gen_t uCode::dec(MReg to, MReg from, Carry carry) const{
	co_yield ctl.calc(ALU::dec(carry),to,from);
}
uCode::gen_t uCode::dec(MReg reg) const{
	co_yield dec(reg,reg);
}
uCode::gen_t uCode::dec(MReg reg, Carry carry) const{
	co_yield dec(reg, reg, carry);
}
uCode::gen_t uCode::dec(MReg16 reg16) const{
	co_yield dec(toL(reg16));
	co_yield dec(toH(reg16),arg.carry);
}

uCode::gen_t uCode::load(MReg to,MReg16 from) const{
	co_yield ctl.load(to,from);
}
uCode::gen_t uCode::save(MReg16 to,MReg from) const{
	co_yield ctl.save(to,from);
}

uCode::gen_t uCode::add(MReg dst,MReg lhs,MReg rhs,Carry carry) const{
	co_yield ctl.calc(ALU::add(carry),dst,lhs,rhs);
}
uCode::gen_t uCode::add(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::add(),dst,lhs,rhs);
}
uCode::gen_t uCode::add(MReg16 dst,MReg16 lhs,MReg16 rhs) const{
	co_yield add(toL(dst),toL(lhs),toL(rhs));
	co_yield add(toH(dst),toH(lhs),toH(rhs),arg.carry);
}
uCode::gen_t uCode::sub(MReg dst,MReg lhs,MReg rhs,Carry carry) const{
	co_yield ctl.calc(ALU::sub(carry),dst,lhs,rhs);
}
uCode::gen_t uCode::sub(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::sub(),dst,lhs,rhs);
}
uCode::gen_t uCode::sub(MReg16 dst,MReg16 lhs,MReg16 rhs) const{
	co_yield sub(toL(dst),toL(lhs),toL(rhs));
	co_yield sub(toH(dst),toH(lhs),toH(rhs),arg.carry);
}
uCode::gen_t uCode::shift_left(MReg dst,MReg lhs,unsigned pad) const{
	co_yield ctl.calc(ALU::shiftLeft(pad),dst,lhs);
}
uCode::gen_t uCode::shift_left(MReg dst,MReg lhs,Carry carry) const{
	co_yield ctl.calc(ALU::shiftLeft(carry),dst,lhs);
}
uCode::gen_t uCode::shift_right(MReg dst,MReg lhs,unsigned pad) const{
	co_yield shift_left(dst,lhs,pad);
	auto carry=arg.carry;
	for(int j=0;j<7;j++){
		co_yield shift_left(dst,dst,carry);
	}
}
uCode::gen_t uCode::shift_right(MReg dst,MReg lhs,Carry CF) const{
	co_yield shift_left(dst,lhs,CF);
	auto carry=arg.carry;
	for(int j=0;j<7;j++){
		co_yield shift_left(dst,dst,carry);
	}
}
uCode::gen_t uCode::logic_and(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::AND(),dst,lhs,rhs);
}
uCode::gen_t uCode::logic_or(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::OR(),dst,lhs,rhs);
}
uCode::gen_t uCode::logic_not(MReg dst,MReg lhs) const{
	co_yield ctl.calc(ALU::NOT(),dst,lhs);
}
uCode::gen_t uCode::logic_xor(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::XOR(),dst,lhs,rhs);
}
uCode::gen_t uCode::set_zero(MReg dst) const{
	co_yield ctl.calc(ALU::zero(),dst);
}
uCode::gen_t uCode::set_zero(MReg16 reg16) const{
	co_yield set_zero(toL(reg16));
	co_yield set_zero(toH(reg16));
}
uCode::gen_t uCode::set_minus_one(MReg reg) const{
	co_yield ctl.calc(ALU::minusOne(),reg);
}
uCode::gen_t uCode::set_minus_one(MReg reg,Carry carry) const{
	co_yield ctl.calc(ALU::minusOne(carry),reg);
}
uCode::gen_t uCode::set_minus_one(MReg16 reg16) const{
	co_yield set_minus_one(toL(reg16));
	co_yield set_minus_one(toH(reg16));
}
uCode::gen_t uCode::test_zero(MReg dst,MReg lhs) const{
	co_yield ctl.calc(ALU::testAeqZero(),dst,lhs);
}
uCode::gen_t uCode::test_less(MReg dst,MReg lhs,MReg rhs) const{
	co_yield ctl.calc(ALU::testALessB(),dst,lhs,rhs);
}
uCode::gen_t uCode::save_carry() const{
	CTL tmp=ctl.nop();
	tmp.CF=arg.carry;
	co_yield tmp;
}

uCode::gen_t uCode::sign_extend(MReg low,MReg high) const{
	co_yield shift_left(high,low,0);
	co_yield set_minus_one(high,~arg.carry);
}
uCode::gen_t uCode::sign_extend(MReg16 reg16) const{
	co_yield sign_extend(toL(reg16),toH(reg16));
}

uCode::gen_t uCode::load_op() const{
	co_yield load(MReg::INST,MReg16::PC);
}
uCode::gen_t uCode::init_op() const{
	co_yield set_zero(MReg::INST);
}
uCode::gen_t uCode::next_op() const{
	co_yield inc(MReg16::PC);
	co_yield load_op();
}
uCode::gen_t uCode::stack_pop(MReg reg) const{
	co_yield inc(MReg16::SP);
	co_yield load(reg,MReg16::SP);
}
uCode::gen_t uCode::stack_push(MReg reg) const{
	co_yield save(MReg16::SP,reg);
	co_yield dec(MReg16::SP);
}
uCode::gen_t uCode::load_imm(MReg reg) const{
	co_yield inc(MReg16::PC);
	co_yield load(reg,MReg16::PC);
}
uCode::gen_t uCode::load_imm_extend(MReg16 reg16) const{
	co_yield inc(MReg16::PC);
	co_yield load(toL(reg16),MReg16::PC);
	co_yield sign_extend(reg16);
}
uCode::gen_t uCode::stack_pop(MReg16 reg16) const{
	co_yield stack_pop(toL(reg16));
	co_yield stack_pop(toH(reg16));
}
uCode::gen_t uCode::stack_push(MReg16 reg16) const{
	co_yield stack_push(toH(reg16));
	co_yield stack_push(toL(reg16));
}
uCode::gen_t uCode::load_imm(MReg16 reg16,bool from_int) const{
	co_yield load_imm(toL(reg16));
	co_yield load_imm(toH(reg16));
}
uCode::gen_t uCode::jump(MReg16 addr) const{
	co_yield copy(MReg16::PC,addr);
	co_yield load_op();
}
uCode::gen_t uCode::branch(MReg16 addr,Carry cond) const{
	if(arg.CF==cond){
		co_yield jump(addr);
	}
}
uCode::gen_t uCode::branch_zero(MReg16 addr,MReg lhs,MReg tmp) const{
	co_yield test_zero(tmp,lhs);
	co_yield save_carry();
	co_yield branch(addr,ALU::ifAeqZero);
}
uCode::gen_t uCode::branch_zero(MReg16 addr,MReg lhs) const{
	co_yield branch_zero(addr,lhs,lhs);
}
uCode::gen_t uCode::branch_less(MReg16 addr,MReg lhs,MReg rhs,MReg tmp) const{
	co_yield test_less(tmp,lhs,rhs);
	co_yield save_carry();
	co_yield branch(addr,ALU::ifALessB);
}