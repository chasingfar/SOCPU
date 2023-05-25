
#include <iostream>
#include <fstream>
#include <socpu/soarchv2.hpp>
#include <socpu/vm.hpp>
#include <soasm/asm.hpp>

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
namespace DEV{
	using namespace SOASM::SOISv1;
	using Code=SOASM::ASM<InstrSet>::Code;
	Code imm(Reg reg,uint8_t v){return {ImmVal{}(v),Pop{.to=reg}(),};}
	Code init(){return imm(Reg::H,0x40);}
	Code write(uint8_t v){return SaveImm{.to=Reg16::HL}(v);}
	namespace LCD{
		Code set_data(){return imm(Reg::L,0);}
		Code set_cmd(){return imm(Reg::L,1);}
		Code cmd(uint8_t code){return {set_cmd(),write(code)};}
		Code cmd(uint8_t code,uint8_t d1,uint8_t d2){
			return {
				set_data(),write(d1),write(d2),
				set_cmd(),write(code),
			};
		}
		Code data(uint8_t d){return {set_data(),write(d),};}
		Code show(char c){return data(c-32);}
		Code show(std::string str){
			Code code{set_data()};
			for(char c:str){
				code.add(show(c));
			}
			return code;
		}
		Code init(){
			return {
				cmd(0x40,0x00,0x00),// set text home address
				cmd(0x41,0x1E,0x00),// set text area
				cmd(0x80),// mode set - or mode
				cmd(0x94),// display mode - graphic off, text on
				cmd(0x24,0x00,0x00),// set address pointer
				cmd(0xB0),// auto write
			};
		}
	}
	namespace PIC{
		Code set_a0(uint8_t a0){return imm(Reg::L,0b10u|(a0&1u));}
		Code icw1(uint8_t AL,bool is_level_trig,bool is_4_interval,bool is_single,bool need_icw4){
			uint8_t data=0b00010000;
			data|=AL<<5;
			if(is_level_trig){data|=0b00001000;}
			if(is_4_interval){data|=0b00000100;}
			if(is_single    ){data|=0b00000010;}
			if(need_icw4    ){data|=0b00000001;}

			return {set_a0(0),write(data)};
		}
		Code icw2(uint8_t AH){
			return {set_a0(1),write(AH)};
		}
		Code icw4(bool SFNM,bool is_buf,bool is_master,bool AEOI,bool is_86){
			uint8_t data=0;
			if(SFNM     ){data|=0b00010000;}
			if(is_buf   ){data|=0b00001000;}
			if(is_master){data|=0b00000100;}
			if(AEOI     ){data|=0b00000010;}
			if(is_86    ){data|=0b00000001;}

			return {set_a0(1),write(data)};
		}
		Code init80(uint16_t addr){
			return {
				icw1((addr>>5)&0b111u,false,true,true,true),
				//icw2(0b10101000),
				icw2((addr>>8)&0xffu),
				icw4(false,false,false,true,false),
			};
		}
		Code init86(uint8_t addr){
			return {
				icw1(0,false,true,true,true),
				//icw2(0b10101000),
				icw2((addr&0x1fu)<<3),
				icw4(false,false,false,true,true),
			};
		}
	}
}

int main() {
	/*
	if(std::ofstream fout("soarchv2-soisv1-3.txt");fout) {
		using namespace std::views;
		generateROM(fout,iota(0uz)|take(1uz<<19uz)|transform([](size_t i){
			return SOARCHv2::uCode(i).generate<SOASM::SOISv1::InstrSet>().val();
		}));
	}
	std::cout << "Hello, World!" << std::endl;
	*/
	/*Sim::VM<SOARCHv2::CPU,SOASM::SOISv1::InstrSet> vm;
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
	}*/
	//Sim::VM<SOARCHv2::CPU,SOASM::SOISv1::InstrSet> vm;
	{
		using namespace SOASM::SOISv1;
		using namespace DEV;
		SOASM::Label::tbl_t L;
		SOASM::ASM<InstrSet>::Code program{
			Jump{}(L["main"]),0xff,
			Jump{}(L["isr1"]),0xff,
			Jump{}(L["isr2"]),0xff,
			Jump{}(L["isr3"]),0xff,
			Jump{}(L["isr4"]),0xff,
			Jump{}(L["isr5"]),0xff,
			Jump{}(L["isr6"]),0xff,
			Jump{}(L["isr7"]),0xff,
			L["main"],
			DEV::init(),
			LCD::init(),
			LCD::show("Hello, world!"),
			PIC::init86(0x00),
			Halt{}(),
			L["isr1"],LCD::show('1'),Return{}(),
			L["isr2"],LCD::show('2'),Return{}(),
			L["isr3"],LCD::show('3'),Return{}(),
			L["isr4"],LCD::show('4'),Return{}(),
			L["isr5"],LCD::show('5'),Return{}(),
			L["isr6"],LCD::show('6'),Return{}(),
			L["isr7"],LCD::show('7'),Return{}(),
		};
		/*SOASM::ASM<InstrSet>::Code program{
			DEV::init(),
			LCD::init(),
			LCD::show("Hello, world!"),
			PIC::init(0x00a0),
			Halt{}(),
			//LT["halt"],
			//Jump{}(LT["halt"]),
			LT["iv"].set(0x00a0),
			Jump{}(LT["isr0"]),0xff,
			Jump{}(LT["isr1"]),0xff,
			Jump{}(LT["isr2"]),0xff,
			Jump{}(LT["isr3"]),0xff,
			Jump{}(LT["isr4"]),0xff,
			Jump{}(LT["isr5"]),0xff,
			Jump{}(LT["isr6"]),0xff,
			Jump{}(LT["isr7"]),0xff,
			LT["isr0"],LCD::show('a'),Return{}(),
			LT["isr1"],LCD::show('b'),Return{}(),
			LT["isr2"],LCD::show('c'),Return{}(),
			LT["isr3"],LCD::show('d'),Return{}(),
			LT["isr4"],LCD::show('e'),Return{}(),
			LT["isr5"],LCD::show('g'),Return{}(),
			LT["isr6"],LCD::show('h'),Return{}(),
			LT["isr7"],LCD::show('i'),Return{}(),
		};*/
		
		auto data=program.assemble();
		for(auto [addr,bytes,str]:SOASM::ASM<InstrSet>::Code::disassemble(data)){
			std::string bytes_str;
			for(auto b:bytes){
				bytes_str+=std::bitset<8>(b).to_string()+" ";
			}
			std::cout<<std::format("{0:016b}:{1:27};{0}:{2}\n",addr,bytes_str,str);
		}
		if(std::ofstream fout("pic-soisv1-3.txt");fout) {
			using namespace std::views;
			generateROM(fout,data);
		}
		//vm.load(program);
		//for(auto [name,id,w]:ezis::list_instr()){
		//	std::cout<<std::format("{}:{} {}",name,id<<w,1<<w)<<std::endl;
		//}
	}
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

	/*vm.reset();
	for(int i=0;i<500;i++){
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
	}*/

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
