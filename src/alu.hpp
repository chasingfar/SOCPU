
#ifndef SOCPU_ALU_HPP
#define SOCPU_ALU_HPP
#include <utility>
#include <iostream>
#include <string_view>
#include <format>
namespace ALU74181{
	enum struct Carry:uint8_t{
		yes = 0,
		no = 1,
	};
	inline Carry operator~(Carry carry){
		return carry==Carry::yes?Carry::no:Carry::yes;
	}
	#define FN_TABLE \
		X(Arith_A                ,0b00000u,"{0}"                    ) \
		X(Arith_AorB             ,0b00001u,"{0}|{1}"                ) \
		X(Arith_AornotB          ,0b00010u,"{0}|(~{1})"             ) \
		X(Arith_minus1           ,0b00011u,"-1"                     ) \
		X(Arith_AplusAandnotB    ,0b00100u,"{0}+({0}&(~{1}))"       ) \
		X(Arith_AorBplusAandnotB ,0b00101u,"({0}|{1})+({0}&(~{1}))" ) \
		X(Arith_AminusBminus1    ,0b00110u,"{0}-{1}-1"              ) \
		X(Arith_AandnotBminus1   ,0b00111u,"({0}&(~{1}))-1"         ) \
		X(Arith_AplusAandB       ,0b01000u,"{0}+({0}&{1})"          ) \
		X(Arith_AplusB           ,0b01001u,"{0}+{1}"                ) \
		X(Arith_AornotBplusAandB ,0b01010u,"({0}|(~{1}))+({0}&{1})" ) \
		X(Arith_AandBminus1      ,0b01011u,"({0}&{1})-1"            ) \
		X(Arith_AplusA           ,0b01100u,"{0}+{0}"                ) \
		X(Arith_AorBplusA        ,0b01101u,"({0}|{1})+{0}"          ) \
		X(Arith_AornotBplusA     ,0b01110u,"({0}|(~{1}))+{0}"       ) \
		X(Arith_Aminus1          ,0b01111u,"{0}-1"                  ) \
		X(Logic_notA             ,0b10000u,"~{0}"                   ) \
		X(Logic_AnorB            ,0b10001u,"~({0}|{1})"             ) \
		X(Logic_notAandB         ,0b10010u,"(~{0})&{1}"             ) \
		X(Logic_fill0            ,0b10011u,"0"                      ) \
		X(Logic_AnandB           ,0b10100u,"~({0}&{1})"             ) \
		X(Logic_notB             ,0b10101u,"~{1}"                   ) \
		X(Logic_AxorB            ,0b10110u,"{0}^{1}"                ) \
		X(Logic_AandnotB         ,0b10111u,"{0}&(~{1})"             ) \
		X(Logic_notAorB          ,0b11000u,"~{0}|{1}"               ) \
		X(Logic_notAxorB         ,0b11001u,"(~{0})^{1}"             ) \
		X(Logic_B                ,0b11010u,"{1}"                    ) \
		X(Logic_AandB            ,0b11011u,"{0}&{1}"                ) \
		X(Logic_fill1            ,0b11100u,"-1"                     ) \
		X(Logic_AornotB          ,0b11101u,"{0}|(~{1})"             ) \
		X(Logic_AorB             ,0b11110u,"{0}|{1}"                ) \
		X(Logic_A                ,0b11111u,"{0}"                    ) \
		
	enum struct ALUFN:uint8_t{
		#define X(a, b, c) a = b,
			FN_TABLE
		#undef X
	};
	static constexpr std::string_view fn_pattern[]={
		#define X(a, b, c) c,
			FN_TABLE
		#undef X
	};
	inline static bool is_arithmetic(ALUFN fn){
		return (std::to_underlying(fn)&0b10000)==0;
	}

	template<size_t size,typename U>
	static std::pair<Carry,U> calc(Carry Cn,ALUFN fn,U _A,U _B){
		std::bitset<4> S{std::to_underlying(fn)&0b01111u};
		std::bitset<size> F,A{_A},B{_B};
		std::bitset<size+1> C;
		bool M=is_arithmetic(fn);
		C[0]=std::to_underlying(Cn);
		for(size_t i=0;i<size;++i){
			bool P_=!A[i] && (!S[1] || B[i]) && (!S[0] || !B[i]);
			bool G_=(!S[3] || !A[i] || !B[i]) && (!S[2] || !A[i] || B[i]);
			C[i+1]=(C[i]&&G_)||P_;
			F[i]=(G_&&!P_)^!(C[i]&&M);
		}
		return {static_cast<Carry>((uint8_t)C[size]), static_cast<U>(F.to_ullong())};
	}

	template<typename U>
	static std::string parse_fn(Carry Cn,ALUFN fn,U A,U B){
		auto str=std::vformat(fn_pattern[std::to_underlying(fn)],std::make_format_args(A,B));
		if(is_arithmetic(fn)&&Cn==Carry::yes){
			auto s=str.rfind("-1");
			if(s!=std::string::npos){
				str.erase(s,2);
				if(str.empty()){
					str="0";
				}
			}else{
				str+="+1";
			}
		}
		return str;
	}


	using alu_ctl_t=std::pair<ALUFN,Carry>;
	static alu_ctl_t passA() {
		return {ALUFN::Logic_A,Carry::no};
	}
	static alu_ctl_t passB() {
		return {ALUFN::Logic_B,Carry::no};
	}
	static alu_ctl_t zero() {
		return {ALUFN::Logic_fill0,Carry::no};
	}
	static alu_ctl_t minusOne() {
		return {ALUFN::Logic_fill1,Carry::no};
	}
	static alu_ctl_t minusOne(Carry carry) {
		return {ALUFN::Arith_minus1,carry};
	}
	static alu_ctl_t testAeqZero() {
		return {ALUFN::Arith_Aminus1, Carry::no};//CF=Carry::no if A==0
	}
	inline static Carry ifAeqZero=Carry::no;
	static alu_ctl_t testALessB() {
		return {ALUFN::Arith_AminusBminus1, Carry::yes};//CF=Carry::no if A<B
	}
	inline static Carry ifALessB=Carry::no;
	static alu_ctl_t add(Carry carry) {
		return {ALUFN::Arith_AplusB, carry};
	}
	static alu_ctl_t add() {
		return add(Carry::no);
	}
	static alu_ctl_t sub(Carry carry) {
		return {ALUFN::Arith_AminusBminus1, carry};
	}
	static alu_ctl_t sub() {
		return sub(Carry::yes);
	}
	static alu_ctl_t inc(Carry carry) {
		return {ALUFN::Arith_A, carry};
	}
	static alu_ctl_t inc() {
		return inc(Carry::yes);
	}
	static alu_ctl_t dec(Carry carry) {
		return {ALUFN::Arith_Aminus1, carry};
	}
	static alu_ctl_t dec() {
		return dec(Carry::no);
	}
	static alu_ctl_t shiftLeft(Carry carry) {
		return {ALUFN::Arith_AplusA, carry};
	}
	static alu_ctl_t shiftLeft(unsigned pad) {
		return shiftLeft(pad==1?Carry::yes:Carry::no);
	}
	static alu_ctl_t shiftLeft() {
		return shiftLeft(0);
	}
	static alu_ctl_t AND() {
		return {ALUFN::Logic_AandB,Carry::no};
	}
	static alu_ctl_t OR() {
		return {ALUFN::Logic_AorB,Carry::no};
	}
	static alu_ctl_t XOR() {
		return {ALUFN::Logic_AxorB,Carry::no};
	}
	static alu_ctl_t NOT() {
		return {ALUFN::Logic_notA,Carry::no};
	}
}

#endif //SOCPU_ALU_HPP
