
#ifndef SOCPU_SIM_ALU_HPP
#define SOCPU_SIM_ALU_HPP
#include <socpu/sim.hpp>
#include "../alu.hpp"
namespace SOCPU::Sim{

	template<size_t Size=8>
	struct ALU:Chip{
		Port<Size> A,B,O{0};
		Port<5> fn;
		Port<1> Ci,Co{0};
		explicit ALU(std::string name=""):Chip(std::move(name)){
			add_inputs(A,B,Ci,fn);
			add_ports(O,Co);
		}
		void run() override {			
			auto [carry,o]=ALU74181::calc<Size>(static_cast<ALU74181::Carry>(Ci.value()),
			                            static_cast<ALU74181::ALUFN>(fn.value()),
			                            A.value(),
			                            B.value());
			Co=static_cast<val_t>(carry);
			O=o;
		}

		[[nodiscard]] Util::Printable print(std::span<const Level> s) const override{
			return [=](std::ostream& os){
				std::string fn_str;
				try{
					fn_str=ALU74181::parse_fn(
							static_cast<ALU74181::Carry>(Ci(s).value()),
							static_cast<ALU74181::ALUFN>(fn(s).value()),
							"A",
							"B");
				}catch(const std::bad_optional_access& e){}
				os<<"CMS="<<Ci(s)<<fn(s)<<"(O="<<fn_str<<"),A="<<A(s)<<",B="<<B(s)<<",O="<<O(s)<<",Co="<<Co(s);
			};
		}
	};
}

#endif //SOCPU_SIM_ALU_HPP
