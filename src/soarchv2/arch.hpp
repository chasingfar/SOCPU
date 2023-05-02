
#ifndef SOCPU_SOARCHV2_ARCH_HPP
#define SOCPU_SOARCHV2_ARCH_HPP

#include "regs.hpp"
#include "../alu.hpp"
#include <functional>
#include <magic_enum.hpp>
#include <utility>

namespace SOCPU::SOARCHv2 {
	using namespace Regs;
	namespace ALU = ALU74181;
	using ALU::Carry;
	using ALU::ALUFN;

	struct STATE{
		using val_t=uint32_t;

		val_t index:7;
		Carry    CF:1;
		val_t    IF:1;

		std::strong_ordering operator<=>(const STATE&) const=default;

		template<typename T>
		static STATE from(const T o) {
			return {
				.index = o.index,
				.CF    = o.CF,
				.IF    = o.IF,
			};
		}

		template<typename T>
		[[nodiscard]] T set_to(T o) const {
			o.index = index;
			o.CF    = CF;
			o.IF    = IF;
			return o;
		}
	};

	struct ARG {
		static constexpr size_t size=19;
		using val_t=uint32_t;

		Carry carry:1;
		val_t index:7;
		Carry CF:1;
		val_t IF:1;
		val_t IRQ:1;
		val_t instr:8;
		val_t :13;//padding to 32bit

		[[nodiscard]] bool isINT() const {
			return IRQ == 0;
		}

		val_t val() const {
			return std::bit_cast<val_t>(*this);
		}

		static ARG make(val_t val) {
			return std::bit_cast<ARG>(val);
		}
	};

	struct CTL {
		static constexpr size_t size = 32;
		using val_t = uint32_t;
		static constexpr MReg default_reg = MReg::H;

		val_t index:7;
		Carry CF:1;

		val_t IF:1;
		val_t INTA:1;
		ALUFN alu_fn:5;
		Carry alu_c:1;

		MReg xs:4;
		MReg ys:4;
		MReg zs:4;

		enum struct Dir:val_t{
			M2M = 0b00,/*should not used*/
			M2R = 0b01,
			R2M = 0b10,
			R2R = 0b11,
		} dir:2;
		val_t unused:2;

		[[nodiscard]] CTL load(MReg16 from, MReg to) const{
			CTL ctl{*this};
			ctl.xs=to;
			ctl.ys=toL(from);
			ctl.zs=toH(from);
			ctl.dir=Dir::M2R;
			return ctl;
		}
		[[nodiscard]] CTL save(MReg from, MReg16 to) const{
			CTL ctl{*this};
			ctl.xs=from;
			ctl.ys=toL(to);
			ctl.zs=toH(to);
			ctl.dir=Dir::R2M;
			return ctl;
		}

		[[nodiscard]] CTL calc(ALU::alu_ctl_t fn,MReg lhs,MReg rhs,MReg dst) const{
			CTL ctl{*this};
			ctl.alu_fn=fn.first;
			ctl.alu_c=fn.second;
			ctl.xs=dst;
			ctl.ys=lhs;
			ctl.zs=rhs;
			ctl.dir=Dir::R2R;
			return ctl;
		}
		[[nodiscard]] CTL calc(ALU::alu_ctl_t fn,MReg lhs,MReg dst) const{
			return calc(fn,lhs,default_reg,dst);
		}
		[[nodiscard]] CTL calc(ALU::alu_ctl_t fn,MReg dst) const{
			return calc(fn,default_reg,default_reg,dst);
		}

		[[nodiscard]] CTL copy(MReg from,MReg to) const{
			return calc(ALU::passA(),from,to);
		}
		[[nodiscard]] CTL nop() const{
			return copy(default_reg,default_reg);
		}

		val_t val() {
			return std::bit_cast<val_t>(*this);
		}

		static CTL make(val_t val=-1) {
			return std::bit_cast<CTL>(val);
		}
		[[nodiscard]] std::string get_action() const {
			auto x=magic_enum::enum_name(xs);
			auto y=magic_enum::enum_name(ys);
			auto z=magic_enum::enum_name(zs);

			switch (dir) {
				case Dir::M2M:
					return "[M2M][Waring]";
				case Dir::M2R:
					return std::format("Reg[{}]=Mem[{}|{}]", x, y, z);
				case Dir::R2M:
					return std::format("Mem[{}|{}]=Reg[{}]", x, y, z);
				case Dir::R2R:
					return std::format("Reg[{}]={}", x, ALU::parse_fn(alu_c, alu_fn, y, z));
			}
		}
	};
}
#endif //SOCPU_SOARCHV2_ARCH_HPP
