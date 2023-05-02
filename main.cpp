
#include <iostream>
#include <socpu/cpuv2/ucode.hpp>
#include "src/soarchv2"
#include <socpu/vm.hpp>

using namespace EZCPU;
/*
std::ostream& operator<<(std::ostream& os,CPUv2::CTL ctl){
	using namespace CPUv2;
	using namespace magic_enum;
	using namespace ostream_operators;
	return os<<" Dir:"<<ctl.dir
		<<" ALU:"<<enum_name(ctl.xs)<<"="
			<<ALU::parse_fn(ctl.alu_c, ctl.alu_fn,
			enum_name(ctl.ys),
			enum_name(ctl.zs))
		<<" CF:"<<ctl.CF
		<<" index:"<<(unsigned)ctl.index;
}*/

auto get_linked(const DCSim::Wire *p){
	std::vector<std::pair<DCSim::Chip*,size_t>> arr;
	for(const auto& c:p->get_related()){
		if(auto chip=dynamic_cast<DCSim::Chip*>(c)){
			for(size_t i=0;const auto [w,b]:chip->pins){
				if(w->has(p)){
					arr.emplace_back(chip,i);
				}
				i++;
			}
		}
	}
	return arr;
}
template<typename T>
void port_print_linked(const T& port){
	for(const auto& p: port.pins){
		for(auto [chip,i]:get_linked(&p)){
			std::cout<<chip->name<<"."<<i<<" ";
		}
		std::cout<<std::endl;
		//std::cout<<p.get_related()<<std::endl;
	}
}

void generateROM(std::ostream& os,std::ranges::input_range auto data){
	os<<"v2.0 raw\r\n";
	os<<std::hex;
	for(auto [i,v]:std::views::zip(std::views::iota(0),data)){
		os<<v<<(i%8==7?"\r\n":" ");
	}
}

int main() {
	std::cout << "Hello, World!" << std::endl;
	if(std::ofstream fout("ezcpuv2-ezisv1.txt");fout) {
		using namespace EZCPU;
		using namespace std::views;
		generateROM(fout,iota(0uz)|take(1uz<<19uz)|transform([](size_t i){
			return CPUv2::uCode(i).generate<EZISv1::InstrSet>().val();
		}));
	}
	/*Sim::VM<Sim::CPUv2::CPU,EZISv1::InstrSet> vm;
	{
		using namespace EZISv1;
		using ezis=EZISv1::InstrSet;
		std::cout<<ezis::get_id<Jump>()<<std::endl;
		//std::cout<<ezis::get_id<Pop>()<<std::endl;
		//std::cout<<sizeof(Pop)<<std::endl;
		EZISv1::InstrSet::Instrs program{
			ImmVal{}(3),
			Pop{.to=Reg::A}(),
			ImmVal{}(0),
			Pop{.to=Reg::B}(),

			Push{.from=Reg::A}(),
			BranchZero{}(22),

			Push{.from=Reg::A}(),
			Push{.from=Reg::B}(),
			Calc{.fn=Calc::FN::ADD}(),
			Pop{.to=Reg::B}(),

			Push{.from=Reg::A}(),
			ImmVal{}(1),
			Calc{.fn=Calc::FN::SUB}(),
			Pop{.to=Reg::A}(),

			Jump{}(6),

			Halt{}(),
		};
		vm.load(program);
		//for(auto [name,id,w]:ezis::list_instr()){
		//	std::cout<<std::format("{}:{} {}",name,id<<w,1<<w)<<std::endl;
		//}
	}*/
	/*{
		using namespace EZCPU::CPUv2;
		Config cfg{ARG{.instr=63}.val()};
		do{
			cfg.idx=0;
			cfg=EZISv1::InstrSet::gen(cfg);
			std::cout<<std::bitset<19>(cfg.arg.val())<<"=>"<<std::bitset<32>(cfg.ctl.val())<<" "<<cfg.ctl.get_action()<<std::endl;
			STATE::from(cfg.ctl).set_to(cfg.arg);
		}while(cfg.ctl.index!=0);
	}*/
	/*for(auto chip:vm.get_chips()){
		std::cout<<"\""<<chip->name<<"\":[";
		for(size_t i=0;auto [p,_]:chip->pins){
			if(i!=0){std::cout<<",";}
			std::cout<<&p;
			i++;
		}
		std::cout<<"],"<<std::endl;
	}*/
/*
	vm.reset();
	for(int i=0;i<50;i++){
		using namespace EZCPU::CPUv2;
		//if(i==8){DCSim::Circuit::debug=true;}
		//if(i==10){DCSim::Circuit::debug=false;}
		vm.tick_instr();
		std::cout<<i<<std::endl;
		std::cout<<vm.print(vm.cpu.get_ptrs({MReg16::SP,MReg16::PC}))<<std::endl;
		if(vm.is_halt()){
			break;
		}
	}
	*/
	//port_print_linked(vm.cpu.mo.buf.O);
	//port_print_linked(vm.cpu.mi.buf.I);
	//port_print_linked(vm.cpu.mi.buf.O);
	/*for(uint32_t i=0;i<0b00000010'00000000000ul;i++){
		auto cfg=Instrs::InstrSet::gen(CPUv2::Config{i});
		if(cfg.ctl.val()!=static_cast<uint32_t>(-1)){
			std::cout<<std::bitset<19>(cfg.arg.val())<<"=>"<<std::bitset<32>(cfg.ctl.val())<<cfg.ctl<<std::endl;
		}
	}*/
	return 0;
}
