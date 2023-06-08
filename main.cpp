#include <iostream>
#include <fstream>
#include <socpu/soarchv2.hpp>
#include <socpu/vm.hpp>
#include <soasm/asm.hpp>

using namespace SOCPU;
using CPU=SOARCHv2::CPU;
using InstrSet=SOASM::SOISv1::InstrSet;
using Code=SOASM::Code;
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
void generateROM(std::string filename,std::ranges::input_range auto data){
	if(std::ofstream fout(filename);fout) {
		generateROM(fout,data);
	}else{
		std::cout<<std::format("Open {} failed\n",filename);
	}
}
namespace DEV{
	using namespace SOASM::SOISv1;
	using Lazy=SOASM::Lazy;
	using Label=SOASM::Label;

	static constexpr Reg16 dev_ptr=Reg16::HL;

	Code imm(Reg reg,uint8_t v){return {ImmVal{}(v),Pop{.to=reg}(),};}
	Code imm(Lazy lazy,size_t offset=0){
		lazy.offset=offset;
		return {ImmVal{}(lazy)};
	}
	Code imm(Lazy lazy){
		return {imm(lazy,1),imm(lazy,0)};
	}
	Code imm(Reg reg,Lazy lazy,size_t offset=0){
		return {imm(lazy,offset),Pop{.to=reg}()};
	}
	Code imm(Reg16 reg16,Lazy lazy){
		return {imm(toH(reg16),lazy,1),imm(toL(reg16),lazy,0)};
	}
	Code init(){return imm(toH(dev_ptr),0x40);}
	Code write(uint8_t v){return SaveImm{.to=dev_ptr}(v);}
	namespace LCD{
		Code set_data(){return imm(toL(dev_ptr),0);}
		Code set_cmd(){return imm(toL(dev_ptr),1);}
		Code cmd(uint8_t code){return {set_cmd(),write(code)};}
		Code cmd(uint8_t code,uint8_t d1,uint8_t d2){
			return {
				set_data(),write(d1),write(d2),
				set_cmd(),write(code),
			};
		}
		Code data(uint8_t d){return {set_data(),write(d),};}
		Code show(){
			return {
				ImmVal{}(-32),
				Calc{.fn=Calc::FN::ADD}(),
				set_data(),
				Save{.to=dev_ptr}(),
			};
		}
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
		Code set_a0(uint8_t a0){return imm(toL(dev_ptr),0b10u|(a0&1u));}
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
	struct KB{
		Reg16 ptr=Reg16::FE;
		Label tbl;
		enum struct KeyCode:char{
			Null=0,Caps,Shift,Ctrl,Win,Alt,Right,Down,Left,Menu,Esc,Back,Del,Enter,Fn,Up,KB64_END
		};
		#define KC(name) std::to_underlying(KeyCode::name)
		Code to_code() const{
			return {tbl,
			/*
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
				*/
				'`','1','2','3','4','5','6','7',
				'\t','Q','W','E','R','T','Y','U',
				KC(Caps),'A','S','D','F','G','H','J',
				KC(Shift),'Z','X','C','V','B','N','M',
				KC(Ctrl),KC(Win),KC(Alt),KC(Right),KC(Down),KC(Left),KC(Menu),' ',
				KC(Esc),KC(Back),'=','-','0','9','8',
				'I',KC(Del),'\\',']','[','P','O','L','K',
				KC(Enter),'\'',';',KC(Fn),KC(Up),'/','.',','
			};
		}
		#undef KCs
		Code get_scan(){
			return {imm(toL(dev_ptr),0b100u),Load{.from=Reg16::HL}()};
		}
		Code scan_to_key(){
			return {
				imm(tbl.lazy(),0),
				Calc{.fn=Calc::FN::ADD}(),
				Pop{.to=toL(ptr)}(),
				ImmVal{}(0),
				Pop{.to=toH(ptr)}(),
				Load{.from=ptr}(),
			};
		}
	};
}
namespace Program{
	using namespace SOASM::SOISv1;
	using tbl_t=SOASM::Label::tbl_t;
	using namespace DEV;
	Code sum(){
		tbl_t L;
		return {
			ImmVal{}(3),
			Pop{.to=Reg::A}(),
			ImmVal{}(0),
			Pop{.to=Reg::B}(),
			L["start"],
			Push{.from=Reg::A}(),
			BranchZero{}(L["end"]),

			Push{.from=Reg::A}(),
			Push{.from=Reg::B}(),
			Calc{.fn=Calc::FN::ADD}(),
			Pop{.to=Reg::B}(),

			Push{.from=Reg::A}(),
			ImmVal{}(1),
			Calc{.fn=Calc::FN::SUB}(),
			Pop{.to=Reg::A}(),

			Jump{}(L["start"]),
			L["end"],
			Halt{}(),
		};
	}
	Code pic80mode(){
		tbl_t L;
		return {
			DEV::init(),
			LCD::init(),
			LCD::show("Hello, world!"),
			PIC::init80(0x00a0),
			Halt{}(),
			L["iv"].set(0x00a0),
			Jump{}(L["isr0"]),0xff,
			Jump{}(L["isr1"]),0xff,
			Jump{}(L["isr2"]),0xff,
			Jump{}(L["isr3"]),0xff,
			Jump{}(L["isr4"]),0xff,
			Jump{}(L["isr5"]),0xff,
			Jump{}(L["isr6"]),0xff,
			Jump{}(L["isr7"]),0xff,
			L["isr0"],LCD::show('a'),Return{}(),
			L["isr1"],LCD::show('b'),Return{}(),
			L["isr2"],LCD::show('c'),Return{}(),
			L["isr3"],LCD::show('d'),Return{}(),
			L["isr4"],LCD::show('e'),Return{}(),
			L["isr5"],LCD::show('g'),Return{}(),
			L["isr6"],LCD::show('h'),Return{}(),
			L["isr7"],LCD::show('i'),Return{}(),
		};
	}
	Code kb64(){
		tbl_t L;
		KB kb;
		return {
			Jump{}(L["main"]),0xff,
			Jump{}(L["isr_kb"]),0xff,
			Jump{}(L["isr2"]),0xff,
			Jump{}(L["isr3"]),0xff,
			Jump{}(L["isr4"]),0xff,
			Jump{}(L["isr5"]),0xff,
			Jump{}(L["isr6"]),0xff,
			Jump{}(L["isr7"]),0xff,
			kb,
			L["main"],
			DEV::init(),
			LCD::init(),
			LCD::show("Hello, world!"),
			PIC::init86(0x00),
			Halt{}(),
			L["isr_kb"],
			kb.get_scan(),
			kb.scan_to_key(),
			LCD::show(),
			Return{}(),
			L["isr2"],LCD::show('2'),Return{}(),
			L["isr3"],LCD::show('3'),Return{}(),
			L["isr4"],LCD::show('4'),Return{}(),
			L["isr5"],LCD::show('5'),Return{}(),
			L["isr6"],LCD::show('6'),Return{}(),
			L["isr7"],LCD::show('7'),Return{}(),
		};
	}
}
void run_vm(Code program){
	Sim::VM<CPU,InstrSet> vm;
	
	vm.load(program.assemble());
	vm.reset();
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
	}
}
void disassemble(std::span<uint8_t> data){
	for(auto [addr,bytes,str]:SOASM::disassemble<InstrSet>(data)){
		std::string bytes_str;
		for(auto b:bytes){
			bytes_str+=std::bitset<8>(b).to_string()+" ";
		}
		std::cout<<std::format("{0:016b}:{1:27};{0}:{2}\n",addr,bytes_str,str);
	}
}
void list_instr(){
	for(auto [name,id,w]:InstrSet::list_instr()){
		std::cout<<std::format("{}:{} {}",name,id<<w,1<<w)<<std::endl;
	}
}
void generateCUROM(std::string filename){
	using namespace std::views;
	generateROM(filename,iota(0uz)|take(1uz<<19uz)|transform([](size_t i){
		return SOARCHv2::uCode(i).generate<InstrSet>().val();
	}));
}
int main() {
	run_vm(Program::sum());
	return 0;
}
