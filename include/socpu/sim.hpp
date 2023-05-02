
#ifndef SOCPU_SIM_HPP
#define SOCPU_SIM_HPP
#include <dcsim/dcsim.hpp>
#include <iomanip>
#include <map>
#include <sstream>

namespace SOCPU::Sim{
	using namespace DCSim;

	template<size_t ASize=16,size_t DSize=8,typename Addr_t=size_t,typename Data_t=val_t>
	struct BusPort{
		using addr_t=Addr_t;
		using data_t=Data_t;
		static constexpr size_t addr_size=ASize;
		static constexpr size_t data_size=DSize;
		Clock CLK;
		Enable RST,WR,RD;
		Port<ASize> A;
		Port<DSize> D;
		BusPort& operator >>(BusPort& bus){
			CLK>>bus.CLK;
			RST>>bus.RST;
			WR >>bus.WR ;
			RD >>bus.RD ;
			A  >>bus.A  ;
			D  >>bus.D  ;
			return bus;
		}
	};
	template<size_t ASize=8,size_t DSize=8,typename Addr_t=size_t,typename Data_t=val_t>
	struct DevPort{
		using addr_t=Addr_t;
		using data_t=Data_t;
		static constexpr size_t addr_size=ASize;
		static constexpr size_t data_size=DSize;
		Enable CS,WR,RD;
		Port<ASize> A;
		Port<DSize> D;
	};
	
	template<
		size_t DevA,size_t DevSize=8,
		typename BUS=BusPort<16,8,uint16_t,uint8_t>
	>
	struct Memory:Circuit{
		using addr_t=BUS::addr_t;
		using data_t=BUS::data_t;
		using DEV=DevPort<DevSize,BUS::data_size>;
		using ptrs_t=std::multimap<addr_t,std::string>;
		constexpr static addr_t addr_min=0;
		constexpr static addr_t addr_max=(1<<BUS::addr_size)-1;

		constexpr static addr_t ram_max=addr_max;
		constexpr static addr_t ram_min=(DevA+1)<<DevSize;

		constexpr static addr_t dev_max=ram_min-1;
		constexpr static addr_t dev_min=DevA<<DevSize;

		constexpr static addr_t rom_max=dev_min-1;
		constexpr static addr_t rom_min=0;

		BUS bus;
		DEV dev;

		Cmp<BUS::addr_size-DevSize> cmp{name+"[CMP]"};
		NAND<1> rom_ce{name+"[ROM_CE]"};
		RAM<BUS::addr_size,BUS::data_size,addr_t,data_t> ram{name+"[RAM]"};
		ROM<BUS::addr_size,BUS::data_size,addr_t,data_t> rom{name+"[ROM]"};
		explicit Memory(std::string name=""):Circuit(std::move(name)){
			add_comps(cmp,rom_ce,ram,rom);

			bus.RD>>ram.oe>>rom.oe;
			bus.WR>>ram.we;

			rom.we.disable();

			cmp.Q=DevA;

			bus.A>>ram.A>>rom.A>>(cmp.P|dev.A);
			bus.D>>ram.D>>rom.D>>dev.D;

			rom_ce.A>>cmp.PeqQ>>dev.CS;
			rom_ce.B>>cmp.PgtQ>>ram.ce;
			rom_ce.Y>>rom.ce;
		}

		static constexpr bool is_ram(addr_t index) { return (index>>DevSize)> DevA;}
		static constexpr bool is_dev(addr_t index) { return (index>>DevSize)==DevA;}
		static constexpr bool is_rom(addr_t index) { return (index>>DevSize)< DevA;}

		void load(std::ranges::input_range auto new_data,BUS::addr_t start=0){
			using namespace std::views;
			for(auto&& [addr,data,rom_it,ram_it]:zip(iota(start),new_data,rom,ram)){
				if(is_rom(addr)) {
					rom_it = data;
				}else if(is_ram(addr)){
					ram_it = data;
				}
			}
		}
		std::optional<data_t> get_data(addr_t index) const{
			if(is_ram(index)){
				return ram.data[index];
			}else if(is_rom(index)){
				return rom.data[index];
			}
			return {};
		}
		std::string get_data_str(addr_t index) const{
			if(auto v=get_data(index);v){
				return std::to_string(*v);
			}else{
				return "DEV";
			}
		}
		static std::vector<addr_t> get_ranges(const ptrs_t& ptrs,addr_t d=2){
			std::vector<addr_t> addrs;
			for(auto [v,name]:ptrs){

				addr_t s=(v-std::min(addr_min,v)>d)?v-d:std::min(addr_min,v);
				addr_t e=(std::max(addr_max,v)-v>d)?v+d:std::max(addr_max,v);

				size_t mid=addrs.size();
				addrs.resize(mid+1+e-s);

				std::iota(addrs.begin()+mid,addrs.end(),s);
				std::inplace_merge(addrs.begin(), addrs.begin()+mid, addrs.end());
			}
			addrs.erase(std::unique(addrs.begin(), addrs.end()), addrs.end());
			return addrs;
		}
		static std::string get_names(const ptrs_t& ptrs,addr_t v,std::string dem="/"){
			auto [first,last] = ptrs.equal_range(v);
			std::string names;
			for(auto it = first; it != last; ++it ){
				if(it!=first){ names += dem; }
				names += it->second;
			}
			return names;
		}
		void print_ptrs(std::ostream& os,const ptrs_t& ptrs,addr_t d=2) const{
			std::vector<addr_t> addrs=get_ranges(ptrs,d);
			std::stringstream addr_ss,data_ss;

			addr_ss<<std::hex;
			size_t addr_max_size=1+((BUS::addr_size-1)>>2);
			for ( auto addr_it = addrs.begin(); addr_it != addrs.end(); ++addr_it ){
				std::string names=get_names(ptrs,*addr_it,"/");
				std::string data_str=get_data_str(*addr_it);

				size_t col_size=std::max({names.size(),addr_max_size,data_str.size()});

				os<<std::setw(col_size)<<names;
				addr_ss<<std::setw(col_size)<<*addr_it;
				data_ss<<std::setw(col_size)<<data_str;

				if(auto it_next=std::next(addr_it);it_next!=addrs.end()){
					if(*it_next-*addr_it>1){
						os<<" ... ";
						addr_ss<<" ... ";
						data_ss<<" ... ";
					}else{
						os<<" ";
						addr_ss<<" ";
						data_ss<<" ";
					}
				}else{
					os<<std::endl
					  <<addr_ss.str()<<std::endl
					  <<data_ss.str()<<std::endl;
				}
			}
		}
	};
};

#endif //SOCPU_SIM_HPP