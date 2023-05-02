
#ifndef SOCPU_SIM_FN_ROM_HPP
#define SOCPU_SIM_FN_ROM_HPP
#include <socpu/sim.hpp>
namespace SOCPU::Sim{

	template<size_t ASize=19,size_t DSize=8,typename addr_t=size_t,typename data_t=val_t>
	struct FnROM:Chip{
        std::function<val_t(addr_t)> fn;
		Port<ASize> A;
		Port<DSize> D;
		explicit FnROM(std::string name=""):Chip(std::move(name)){
			add_inputs(A);
			add_ports(D);
		}
		void run() override {
			D=fn(A.template value<addr_t>());
		}
		[[nodiscard]] Util::Printable print(std::span<const Level> s) const override{
			return [=](std::ostream& os){
				os<<"data["<<A(s)<<"]="<<D(s);
			};
		}
	};
}

#endif //SOCPU_SIM_FN_ROM_HPP
