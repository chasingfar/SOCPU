#include "ucode.hpp"

using namespace SOCPU::SOARCHv2;

uCode::gen_t uCode::halt() const{
	co_yield ctl.nop();
}
uCode::gen_t uCode::copy(MReg from,MReg to) const{
	co_yield ctl.copy(from,to);
}
uCode::gen_t uCode::copy(MReg16 from,MReg16 to) const{
	co_yield copy(toL(from), toL(to));
	co_yield copy(toH(from), toH(to));
}

uCode::gen_t uCode::inc(MReg from,MReg to) const{
	co_yield ctl.calc(ALU::inc(),from,to);
}
uCode::gen_t uCode::inc(MReg from, MReg to, Carry carry) const{
	co_yield ctl.calc(ALU::inc(carry),from,to);
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

uCode::gen_t uCode::dec(MReg from,MReg to) const{
	co_yield ctl.calc(ALU::dec(),from,to);
}
uCode::gen_t uCode::dec(MReg from, MReg to, Carry carry) const{
	co_yield ctl.calc(ALU::dec(carry),from,to);
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

uCode::gen_t uCode::load(MReg16 from,MReg to) const{
	co_yield ctl.load(from,to);
}
uCode::gen_t uCode::save(MReg from,MReg16 to) const{
	co_yield ctl.save(from,to);
}

uCode::gen_t uCode::add(MReg lhs,MReg rhs,MReg dst,Carry carry) const{
	co_yield ctl.calc(ALU::add(carry),lhs,rhs,dst);
}
uCode::gen_t uCode::add(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::add(),lhs,rhs,dst);
}
uCode::gen_t uCode::add(MReg16 lhs,MReg16 rhs,MReg16 dst) const{
	co_yield add(toL(lhs),toL(rhs),toL(dst));
	co_yield add(toH(lhs),toH(rhs),toH(dst),arg.carry);
}
uCode::gen_t uCode::sub(MReg lhs,MReg rhs,MReg dst,Carry carry) const{
	co_yield ctl.calc(ALU::sub(carry),lhs,rhs,dst);
}
uCode::gen_t uCode::sub(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::sub(),lhs,rhs,dst);
}
uCode::gen_t uCode::sub(MReg16 lhs,MReg16 rhs,MReg16 dst) const{
	co_yield sub(toL(lhs),toL(rhs),toL(dst));
	co_yield sub(toH(lhs),toH(rhs),toH(dst),arg.carry);
}
uCode::gen_t uCode::shift_left(MReg lhs,MReg dst,unsigned pad) const{
	co_yield ctl.calc(ALU::shiftLeft(pad),lhs,dst);
}
uCode::gen_t uCode::shift_left(MReg lhs,MReg dst,Carry carry) const{
	co_yield ctl.calc(ALU::shiftLeft(carry),lhs,dst);
}
uCode::gen_t uCode::shift_right(MReg lhs,MReg dst,unsigned pad) const{
	co_yield shift_left(lhs,dst,pad);
	auto carry=arg.carry;
	for(int j=0;j<7;j++){
		co_yield shift_left(dst,dst,carry);
	}
}
uCode::gen_t uCode::shift_right(MReg lhs,MReg dst,Carry CF) const{
	co_yield shift_left(lhs,dst,CF);
	auto carry=arg.carry;
	for(int j=0;j<7;j++){
		co_yield shift_left(dst,dst,carry);
	}
}
uCode::gen_t uCode::logic_and(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::AND(),lhs,rhs,dst);
}
uCode::gen_t uCode::logic_or(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::OR(),lhs,rhs,dst);
}
uCode::gen_t uCode::logic_not(MReg lhs,MReg dst) const{
	co_yield ctl.calc(ALU::NOT(),lhs,dst);
}
uCode::gen_t uCode::logic_xor(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::XOR(),lhs,rhs,dst);
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
uCode::gen_t uCode::test_zero(MReg lhs,MReg dst) const{
	co_yield ctl.calc(ALU::testAeqZero(),lhs,dst);
}
uCode::gen_t uCode::test_less(MReg lhs,MReg rhs,MReg dst) const{
	co_yield ctl.calc(ALU::testALessB(),lhs,rhs,dst);
}
uCode::gen_t uCode::save_carry() const{
	CTL tmp=ctl.nop();
	tmp.CF=arg.carry;
	co_yield tmp;
}

uCode::gen_t uCode::sign_extend(MReg low,MReg high) const{
	co_yield shift_left(low,high,0);
	co_yield set_minus_one(high,~arg.carry);
}
uCode::gen_t uCode::sign_extend(MReg16 reg16) const{
	co_yield sign_extend(toL(reg16),toH(reg16));
}

uCode::gen_t uCode::load_op() const{
	co_yield load(MReg16::PC, MReg::INST);
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
	co_yield load(MReg16::SP,reg);
}
uCode::gen_t uCode::stack_push(MReg reg) const{
	co_yield save(reg,MReg16::SP);
	co_yield dec(MReg16::SP);
}
uCode::gen_t uCode::load_imm(MReg reg) const{
	co_yield inc(MReg16::PC);
	co_yield load(MReg16::PC,reg);
}
uCode::gen_t uCode::load_imm_extend(MReg16 reg16) const{
	co_yield inc(MReg16::PC);
	co_yield load(MReg16::PC,toL(reg16));
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
	co_yield copy(addr,MReg16::PC);
	co_yield load_op();
}
uCode::gen_t uCode::branch(MReg16 addr,Carry cond) const{
	if(arg.CF==cond){
		co_yield jump(addr);
	}
}
uCode::gen_t uCode::branch_zero(MReg16 addr,MReg lhs,MReg dst) const{
	co_yield test_zero(lhs,dst);
	co_yield save_carry();
	co_yield branch(addr,ALU::ifAeqZero);
}
uCode::gen_t uCode::branch_zero(MReg16 addr,MReg lhs) const{
	co_yield branch_zero(addr,lhs,lhs);
}
uCode::gen_t uCode::branch_less(MReg16 addr,MReg lhs,MReg rhs,MReg dst) const{
	co_yield test_less(lhs,rhs,dst);
	co_yield save_carry();
	co_yield branch(addr,ALU::ifALessB);
}