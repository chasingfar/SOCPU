
#ifndef SOCPU_SOARCHV2_REGS_HPP
#define SOCPU_SOARCHV2_REGS_HPP
#include <utility>
#include <soasm/soisv1.hpp>
namespace SOCPU::SOARCHv2::Regs{
	using namespace SOASM::SOISv1::Regs;
	enum struct MReg:uint8_t{
		INST,TMA,
		TML,TMH,
		SPL,SPH,
		PCL,PCH,
		A  ,B  ,
		C  ,D  ,
		E  ,F  ,
		L  ,H  ,
	};
	enum struct MReg16:uint8_t{
		IMM,
		TMP,
		SP ,
		PC ,
		BA ,
		DC ,
		FE ,
		HL ,
	};

	template<typename From> struct to_machine{};
	template<> struct to_machine<Reg>{
		using type=MReg;
		static constexpr MReg offset=MReg::A;
	};
	template<> struct to_machine<Reg16>{
		using type=MReg16;
		static constexpr MReg16 offset=MReg16::BA;
	};
	
	template<typename From> struct to_narrow{};
	template<> struct to_narrow<MReg16>{ using type=MReg;};
	template<> struct to_narrow< Reg16>{ using type= Reg;};
	
	template<typename From>
	auto toM(From reg){
		using M=to_machine<From>;
		return static_cast<M::type>(std::to_underlying(reg)+std::to_underlying(M::offset));
	}
	inline auto toL(MReg16 reg16){
		return static_cast<MReg>((std::to_underlying(reg16)<<1)+0);
	}
	inline auto toH(MReg16 reg16){
		return static_cast<MReg>((std::to_underlying(reg16)<<1)+1);
	}
}
#endif //SOCPU_SOARCHV2_REGS_HPP
