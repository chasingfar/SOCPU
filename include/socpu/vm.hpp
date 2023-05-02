
#ifndef SOPU_VM_HPP
#define SOPU_VM_HPP
#include "sim.hpp"

namespace SOCPU::Sim{

	template<typename CPU,typename InstrSet>
	struct VM:Circuit{
		using BUS=CPU::BUS;
		using MEM=Memory<0x40,8,BUS>;

		BUS bus;
		CPU cpu;
		MEM mem;
		size_t clk_count=0;
		explicit VM(std::string name=""):Circuit(std::move(name)){
			add_comps(cpu,mem);

			bus>>cpu.bus>>mem.bus;

			cpu.template init<InstrSet>();
		}
		void reset(){
			clk_count+=0;
			bus.RST.enable();
			run();
			bus.RST.disable();
			run();
		}
		void clock(){
			clk_count+=1;
			bus.CLK.clock();
			run();
		}
		void tick(){
			do{
				clock();
			}while(!cpu.is_tick_end());
		}
		void tick_instr(){
			do{
				tick();
			}while(!cpu.is_instr_end());
		}
		[[nodiscard]] bool is_halt() const{
			return cpu.is_halt();
		}
		void load(std::ranges::input_range auto data,BUS::addr_t start=0){
			mem.load(data,start);
		}
		[[nodiscard]] Util::Printable print(const MEM::ptrs_t& ptrs,BUS::addr_t d=2) const {
			return [=](std::ostream& os){
				os<<"INSTR:"<<InstrSet::visit([](auto instr){
					return decltype(instr)::name;
				},cpu.get_instr())<<std::endl;
				os<<cpu.print();
				mem.print_ptrs(os, ptrs,d);
			};
		}
		[[nodiscard]] Util::Printable print() const override{
			return print(cpu.get_ptrs());
		}
	};

};

#endif //SOPU_VM_HPP