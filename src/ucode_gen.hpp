
#ifndef SOCPU_UCODE_GEN_HPP
#define SOCPU_UCODE_GEN_HPP

#include <cstddef>
#include <variant>
#include <generator.hpp>
namespace SOCPU{
	template<typename T>
	struct uCodeGen{
		template<typename InstrSet>
		auto generate() const{
			using CTL=decltype(self()->ctl);
			return std::visit([&](auto instr_obj){
				using namespace std::views;
				CTL ctl=CTL::make();
				auto index=self()->arg.index;
				for(auto [i,cur]:zip(iota(0),self()->gen(instr_obj))|drop(index)){
					if(i==index) {
						ctl = cur;
						ctl.index = 0;
					}else if(i>index) {
						ctl.index = index + 1;
						return ctl;
					}
				}
				return ctl;
			},InstrSet::get_instr(self()->arg.instr));
		}
	private:
		//CRTP guard from https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/
		uCodeGen() = default;
		friend T;
		auto self(){return static_cast<T*>(this);}
		auto self() const{return static_cast<const T*>(this);}
	};
} // SOCPU

#endif //SOCPU_UCODE_GEN_HPP
