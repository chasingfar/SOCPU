
#ifndef SOCPU_SOARCHV2_UCODE_HPP
#define SOCPU_SOARCHV2_UCODE_HPP

#include "arch.hpp"
#include "../ucode_gen.hpp"

namespace SOCPU::SOARCHv2 {
	struct uCode:uCodeGen<uCode>{
		using gen_t=Util::generator<CTL>;

		ARG arg;
		CTL ctl=save_state();

		explicit uCode(ARG::val_t arg):arg(ARG::make(arg)){};
		explicit uCode(ARG arg):arg(arg){};
		[[nodiscard]] CTL save_state() const{
			return STATE::from(arg).set_to(CTL::make(-1));
		}

		template<typename Instr>
		[[nodiscard]] gen_t gen(Instr) const;

		[[nodiscard]] gen_t halt() const;
		[[nodiscard]] gen_t load(MReg to, MReg16 from) const;
		[[nodiscard]] gen_t save(MReg16 to, MReg from) const;
		[[nodiscard]] gen_t copy(MReg to, MReg from) const;
		[[nodiscard]] gen_t copy(MReg16 to, MReg16 from) const;

		[[nodiscard]] gen_t inc(MReg to, MReg from) const;
		[[nodiscard]] gen_t inc(MReg to, MReg from, Carry carry) const;
		[[nodiscard]] gen_t inc(MReg reg) const;
		[[nodiscard]] gen_t inc(MReg reg, Carry carry) const;
		[[nodiscard]] gen_t inc(MReg16 reg16) const;

		[[nodiscard]] gen_t dec(MReg to, MReg from) const;
		[[nodiscard]] gen_t dec(MReg to, MReg from, Carry carry) const;
		[[nodiscard]] gen_t dec(MReg reg) const;
		[[nodiscard]] gen_t dec(MReg reg, Carry carry) const;
		[[nodiscard]] gen_t dec(MReg16 reg16) const;

		[[nodiscard]] gen_t add(MReg dst, MReg lhs, MReg rhs, Carry carry) const;
		[[nodiscard]] gen_t add(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t add(MReg16 dst, MReg16 lhs, MReg16 rhs) const;
		[[nodiscard]] gen_t sub(MReg dst, MReg lhs, MReg rhs, Carry carry) const;
		[[nodiscard]] gen_t sub(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t sub(MReg16 dst, MReg16 lhs, MReg16 rhs) const;
		[[nodiscard]] gen_t shift_left(MReg dst, MReg lhs, unsigned pad) const;
		[[nodiscard]] gen_t shift_left(MReg dst, MReg lhs, Carry carry) const;
		[[nodiscard]] gen_t shift_right(MReg dst, MReg lhs, unsigned pad) const;
		[[nodiscard]] gen_t shift_right(MReg dst, MReg lhs, Carry CF) const;
		[[nodiscard]] gen_t logic_and(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t logic_or(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t logic_not(MReg dst, MReg lhs) const;
		[[nodiscard]] gen_t logic_xor(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t set_zero(MReg dst) const;
		[[nodiscard]] gen_t set_zero(MReg16 reg16) const;
		[[nodiscard]] gen_t set_minus_one(MReg reg) const;
		[[nodiscard]] gen_t set_minus_one(MReg reg, Carry carry) const;
		[[nodiscard]] gen_t set_minus_one(MReg16 reg16) const;
		[[nodiscard]] gen_t test_zero(MReg dst, MReg lhs) const;
		[[nodiscard]] gen_t test_less(MReg dst, MReg lhs, MReg rhs) const;
		[[nodiscard]] gen_t save_carry() const;

		[[nodiscard]] gen_t sign_extend(MReg low, MReg high) const;
		[[nodiscard]] gen_t sign_extend(MReg16 reg16) const;

		[[nodiscard]] gen_t load_op() const;
		[[nodiscard]] gen_t init_op() const;
		[[nodiscard]] gen_t next_op() const;
		[[nodiscard]] gen_t stack_pop(MReg reg) const;
		[[nodiscard]] gen_t stack_pop(MReg16 reg16) const;
		[[nodiscard]] gen_t stack_push(MReg reg) const;
		[[nodiscard]] gen_t stack_push(MReg16 reg16) const;
		[[nodiscard]] gen_t load_imm(MReg reg) const;
		[[nodiscard]] gen_t load_imm(MReg16 reg16, bool from_int = false) const;
		[[nodiscard]] gen_t load_imm_extend(MReg16 reg16) const;
		[[nodiscard]] gen_t jump(MReg16 addr) const;
		[[nodiscard]] gen_t branch(MReg16 addr, Carry cond = Carry::yes) const;
		[[nodiscard]] gen_t branch_zero(MReg16 addr, MReg lhs, MReg tmp) const;
		[[nodiscard]] gen_t branch_zero(MReg16 addr, MReg lhs) const;
		[[nodiscard]] gen_t branch_less(MReg16 addr, MReg lhs, MReg rhs, MReg tmp) const;
	};
}
#endif //SOCPU_SOARCHV2_UCODE_HPP
