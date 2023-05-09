
#include <iostream>
#include <fstream>
#include <socpu/soarchv2.hpp>
#include <socpu/vm.hpp>

using namespace SOCPU;
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
		os<<(unsigned long long)v<<(i%8==7?"\r\n":" ");
	}
}

int main() {
	/*std::cout << "Hello, World!" << std::endl;
	if(std::ofstream fout("soarchv2-soisv1.txt");fout) {
		using namespace std::views;
		generateROM(fout,iota(0uz)|take(1uz<<19uz)|transform([](size_t i){
			return SOARCHv2::uCode(i).generate<SOASM::SOISv1::InstrSet>().val();
		}));
	}*/
	Sim::VM<SOARCHv2::CPU,SOASM::SOISv1::InstrSet> vm;
	{
		using namespace SOASM::SOISv1;
		//using sois=InstrSet;
		//std::cout<<sois::get_id<Jump>()<<std::endl;
		//std::cout<<ezis::get_id<Pop>()<<std::endl;
		//std::cout<<sizeof(Pop)<<std::endl;
		SOASM::Label::tbl_t LT;
		SOASM::ASM<InstrSet>::Code program{
			ImmVal{}(3),
			Pop{.to=Reg::A}(),
			ImmVal{}(0),
			Pop{.to=Reg::B}(),
			LT["start"],
			Push{.from=Reg::A}(),
			BranchZero{}(LT["end"]),

			Push{.from=Reg::A}(),
			Push{.from=Reg::B}(),
			Calc{.fn=Calc::FN::ADD}(),
			Pop{.to=Reg::B}(),

			Push{.from=Reg::A}(),
			ImmVal{}(1),
			Calc{.fn=Calc::FN::SUB}(),
			Pop{.to=Reg::A}(),

			Jump{}(LT["start"]),
			LT["end"],
			Halt{}(),
		};
		vm.load(program.assemble());
		//for(auto [name,id,w]:ezis::list_instr()){
		//	std::cout<<std::format("{}:{} {}",name,id<<w,1<<w)<<std::endl;
		//}
	}
	//Sim::VM<SOARCHv2::CPU,SOASM::SOISv1::InstrSet> vm;
	/*{
		using namespace SOASM::SOISv1;
		using sois=InstrSet;
		std::cout<<sois::get_id<Jump>()<<std::endl;
		//std::cout<<ezis::get_id<Pop>()<<std::endl;
		//std::cout<<sizeof(Pop)<<std::endl;
		auto write_data=[](uint8_t d){return SaveImm{.to=Reg16::HL}(d);};
		auto write_char=[&](char c){return write_data(c-'A'+0x21);};
		auto write_command=[](uint8_t d){return SaveImm{.to=Reg16::FE}(d);};
		sois::Instrs program{
			ImmVal{}(0x40),
			Pop{.to=Reg::H}(),
			ImmVal{}(0x00),
			Pop{.to=Reg::L}(),
			ImmVal{}(0x40),
			Pop{.to=Reg::F}(),
			ImmVal{}(0x01),
			Pop{.to=Reg::E}(),
			ImmVal{}(40),
			Pop{.to=Reg::A}(),

			write_data(0x00),
  			write_data(0x00),
  			write_command(0x40), // set text home address

  			write_data(0x1E), // 240/8
  			write_data(0x00),
  			write_command(0x41), // set text area

  			write_command(0x80), // mode set - exor mode
  			write_command(0x94), // display mode - graphic on, text on


			write_data(0x00),
			write_data(0x00),
			write_command(0x24),

			write_command(0xB0), // auto write

			Push{.from=Reg::A}(),
			BranchZero{}(75),
			
  			write_char('H'),
  			write_char('e'),
  			write_char('l'),
  			write_char('l'),
  			write_char('o'),
  			write_char(' '),
  			write_char('w'),
  			write_char('o'),
  			write_char('r'),
  			write_char('l'),
  			write_char('d'),
  			write_char('!'),
			Push{.from=Reg::A}(),
			ImmVal{}(1),
			Calc{.fn=Calc::FN::SUB}(),
			Pop{.to=Reg::A}(),

			Jump{}(39),
			Halt{}(),
		};
		
		if(std::ofstream fout("hello-text-soisv1.txt");fout) {
			using namespace std::views;
			generateROM(fout,program);
		}
		//vm.load(program);
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

	vm.reset();
	for(int i=0;i<50;i++){
		using namespace SOARCHv2;
		//if(i==8){DCSim::Circuit::debug=true;}
		//if(i==10){DCSim::Circuit::debug=false;}
		//if(i<5){
		vm.tick_instr();
		//}else{
		//	vm.tick();
		//}
		std::cout<<i<<std::endl;
		std::cout<<vm.print(vm.cpu.get_ptrs({MReg16::SP,MReg16::PC,MReg16::HL}))<<std::endl;
		if(vm.is_halt()){
			break;
		}
	}

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
