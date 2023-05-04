
#ifndef SOCPU_SOARCHV2_CPU_HPP
#define SOCPU_SOARCHV2_CPU_HPP

#include "../sim/rect_board.hpp"
#include "../sim/alu.hpp"
#include "../sim/fn_rom.hpp"
#include "ucode.hpp"
#include <cstddef>

namespace SOCPU::SOARCHv2{
	namespace Sim{
		using namespace SOCPU::Sim;

		template<size_t Size>
		struct RFReg:Circuit,RectBoard<1,5+Size,2+Size,5+Size,1,1,2,1>{
			using RECT=RectBoard<1,5+Size,2+Size,5+Size,1,1,2,1>;
			RegCECLR<Size> reg{name+"[Reg]"};
			Port<Size> A;
			Port<1>& ce1=reg.ce1,&clr=reg.clr,&clk=reg.clk;
			Port<Size>& D=reg.D,&Q=reg.Q;
			explicit RFReg(size_t i,std::string name=""):Circuit(std::move(name)){
				add_comps(reg);
				A=i;
				this->prtW[0]>>(Port<1>{}|reg.D|Port<1>{}|reg.ce1|reg.clr|reg.clk)>>this->prtE[0];
				(reg.ce2|reg.Q|Port<1>{})>>this->prtS[0];
				(reg.ce2|A|Port<1>{})>>this->prtS[1];
			}
		};
		template<size_t Size>
		struct RFSel:Circuit,RectBoard<2+Size,2+Size,2+Size,2+Size,2,2,2,2>{
			using RECT=RectBoard<2+Size,2+Size,2+Size,2+Size,2,2,2,2>;
			Eq<Size> cmp{name+"[EQ]"};
			Buffer<Size> buf{name+"[BUF]"};

			Port<1>& oe=buf.oe,&eq=cmp.PeqQ;
			Port<Size>& A=cmp.P,&S=cmp.Q,&I=buf.I,&O=buf.O;
			explicit RFSel(std::string name=""):Circuit(std::move(name)){
				add_comps(cmp,buf);
				Port<1> tmp;

				this->prtN[0]>>(oe|I|Port<1>{});
				this->prtN[1]>>(eq|A|Port<1>{});
				(tmp|I|Port<1>{})>>this->prtS[0];
				(tmp|A|Port<1>{})>>this->prtS[1];
				this->prtW[0]>>(Port<1>{}|O|Port<1>{})>>this->prtE[0];
				this->prtW[1]>>(Port<1>{}|S|Port<1>{})>>this->prtE[1];
				cmp.oe.enable();
			}
		};
		
		template<size_t Size=8>
		struct ALUBrd:Circuit,RectBoard<7,Size+2,7,1,1,1,1,1>{
			Sim::ALU<Size> alu{name+"[ALU]"};
			Buffer<Size> buf{name+"[Buf]"};
			Port<Size> L,R;
			Port<1> Ci,Co,oe;
			Port<6> CMS;
			explicit ALUBrd(std::string name=""):Circuit(std::move(name)){
				add_comps(alu,buf);
				alu.A>>L;
				alu.B>>R;
				alu.Co>>Co;
				alu.O>>buf.I;
				buf.oe>>oe;
				(Port<1>{}|buf.O|Port<1>{})>>this->prtE[0];
				(oe|((alu.Ci>>Ci|alu.fn)>>CMS))>>this->prtN[0];
				(oe|alu.Co|alu.fn)>>this->prtS[0];
			}
		};

		struct CPU:Circuit{
			using word_t = uint8_t;
			using addr_t = uint16_t;
			static constexpr size_t word_size=sizeof(word_t)*8;
			static constexpr size_t addr_size=sizeof(addr_t)*8;
			using BUS=BusPort<16,8,addr_t,word_t>;
			BUS bus;

			FnROM<19,32,ARG::val_t,CTL::val_t> CU{name+"[CU]"};
			ALUBrd<8> ALU{name+"[ALU]"};
			RegCLR<11> sreg{name+"[SReg]"};
			std::array<RFReg<8>,16> regs;
			std::array<RFSel<8>,48> sel;
			RFSel<8> mi{name+"[MI]"},mo{name+"[MO]"};
			NOT<1> rw{name+"[RW]"},rr{name+"[RR]"};
			OR<1> mw_or{name+"[MW_OR]"};

			Port<10> ctl_state;
			Port<4> xs,ys,zs;
			Port<1> mr,mw;

			static constexpr auto get_reg_name(size_t idx){
				return magic_enum::enum_name(static_cast<Regs::MReg>(idx));
			}
			explicit CPU(std::string name=""): Circuit(std::move(name)), regs{
				[name]<size_t ...I>(std::index_sequence<I...>){
					return std::array{RFReg<8>(I,std::format("{}[Reg {}]",name,get_reg_name(I)))...};
				}(std::make_index_sequence<16>{})
			},sel{
				[name]<size_t ...I>(std::index_sequence<I...>){
					return std::array{RFSel<8>(std::format("{}[Sel {}]",name,I))...};
				}(std::make_index_sequence<48>{})
			}{
				add_comps(sreg,CU,rw,rr,mw_or,mi,mo,ALU);
				[&]<size_t ...I>(std::index_sequence<I...>){
					(RectBoard<7,3,7,3>{}/ALU)|(([&]<size_t ...J>(size_t i,std::index_sequence<J...>){
						add_comps(regs[i], sel[i * 3 + J]...);
						return (regs[i]/.../sel[i * 3 + J]);
					}(I,std::make_index_sequence<3>{}))|...);//|(RectBoard<10,3,10,3>{}/mo.rot_r().rot_r()/mi.rot_r().rot_r().rot_r()/RectBoard<10,20,10,20>{});
				}(std::make_index_sequence<16>{});
				mo.O>>regs[0].D;
				mi.I>>sel[0].O;
				mo.I>>mi.O>>bus.D;
				(sel[2].O|sel[1].O)>>(ALU.R|ALU.L)>>bus.A;

				bus.RST >> sreg.clr >> regs[0].clr;
				bus.CLK >> sreg.clk >> regs[0].clk >> mw_or.A;

				CU.D>>(zs|ys|xs|Port<2>{}|ctl_state|ALU.CMS|mr|mw);
				sreg.D>>(ctl_state|ALU.Co);
				CU.A>>(regs[0].Q | sreg.Q);
				mw_or.Y>>bus.WR;

				sel[0].S>>(Port<4>{0}|xs);
				sel[1].S>>(Port<4>{0}|ys);
				sel[2].S>>(Port<4>{0}|zs);

				regs[0].reg.ce1 >> rw.Y;
				mr>>rr.A>>bus.RD>>mo.buf.oe;
				mw>>rw.A>>mw_or.B>>mi.buf.oe;
				rr.Y>>ALU.oe;

				bus.RST=Level::PullUp;
				bus.CLK=Level::PullDown;
			}
			template<typename InstrSet>
			void init(){
				using namespace std::views;
				CU.fn=[](ARG::val_t i){
					return uCode(i).generate<InstrSet>().val();
				};
			}
			[[nodiscard]] bool is_halt() const{
				auto arg=ARG::make(CU.A.value());
				auto ctl=CTL::make(CU.D.value());
				return STATE::from(ctl)==STATE::from(arg);
			}
			[[nodiscard]] bool is_tick_end() const{
				return bus.CLK.value()==0;
			}
			[[nodiscard]] bool is_instr_end() const{
				return CTL::make(CU.D.value()).index==0 || is_halt();
			}

			void set_reg(MReg mreg,word_t v){
				regs[std::to_underlying(mreg)].reg.set(v);
			}
			void set_reg(MReg16 mreg16,addr_t v){
				set_reg(toH(mreg16),(v>>8u)&0xffu);
				set_reg(toL(mreg16),      v&0xffu);
			}
			void set_reg(Reg reg,word_t v){
				set_reg(toM(reg),v);
			}
			void set_reg(Reg16 reg16,addr_t v){
				set_reg(toM(reg16),v);
			}

			[[nodiscard]] word_t get_reg(MReg mreg) const {
				return regs[std::to_underlying(mreg)].reg.Q.value();
			}
			[[nodiscard]] addr_t get_reg(MReg16 mreg16) const {
				return (static_cast<addr_t>(get_reg(toH(mreg16)))<<8u)|get_reg(toL(mreg16));
			}
			[[nodiscard]] word_t get_reg(Reg reg) const {
				return get_reg(toM(reg));
			}
			[[nodiscard]] addr_t get_reg(Reg16 reg16) const {
				return get_reg(toM(reg16));
			}
			[[nodiscard]] auto get_ptrs(const std::vector<MReg16>& reg16s) const {
				std::multimap<addr_t,std::string> ptrs{};
				for(auto ptr:reg16s){
					ptrs.emplace(get_reg(ptr),magic_enum::enum_name(ptr));
				}
				return ptrs;
			}
			[[nodiscard]] auto get_ptrs() const {
				std::multimap<addr_t,std::string> ptrs{};
				for(auto [r,str]:magic_enum::enum_entries<MReg16>()){
					ptrs.emplace(get_reg(r),str);
				}
				return ptrs;
			}

			void set_instr(word_t instr){
				set_reg(MReg::INST,instr);
			}
			[[nodiscard]] word_t get_instr() const {
				return get_reg(MReg::INST);
			}

			[[nodiscard]] Util::Printable print_regs() const {
				return [=](std::ostream& os){
					for(size_t i=0;i<16;++i){
						os<<std::format("Reg[{}]={:d}",get_reg_name(i),get_reg(static_cast<MReg>(i)));
						if(i%8==7){
							os<<std::endl;
						}else{
							os<<" ";
						}
					}
				};
			}
			[[nodiscard]] Util::Printable print() const override{
				return [=](std::ostream& os){
					os<<"ARG:"<<std::bitset<ARG::size>(CU.A.value())<<std::endl;
					os<<"CTL:"<<std::bitset<CTL::size>(CU.D.value())<<std::endl;
					os<<"ACT:"<<CTL::make(CU.D.value()).get_action()<<std::endl;
					os<<"IDX:"<<CTL::make(CU.D.value()).index<<std::endl;
					os<<print_regs();
				};
			}
		};
	}
	using CPU=Sim::CPU;
}
#endif //SOCPU_SOARCHV2_CPU_HPP
