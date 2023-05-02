
#ifndef SOCPU_SIM_RECT_BOARD_HPP
#define SOCPU_SIM_RECT_BOARD_HPP

#include <socpu/sim.hpp>

namespace SOCPU::Sim{

	template<size_t S,size_t L>
	using MultiPort=std::array<Port<S>,L>;

	template<typename T>
	concept MultiPortLike=requires(T p) {{p[0]}->LinkablePort;};

	template<MultiPortLike L,MultiPortLike R>
	auto operator >>(L&& lhs,R&& rhs){
		for(auto [l,r]:std::views::zip(lhs,rhs)){
			l>>r;
		}
		return rhs;
	}
	template<LinkablePort L,MultiPortLike R>
	auto operator >>(L&& lhs,R&& rhs){
		return std::array{lhs}>>rhs;
	}
	template<MultiPortLike L,LinkablePort R>
	auto operator >>(L&& lhs,R&& rhs){
		return lhs>>std::array{rhs};
	}
	template<MultiPortLike L,MultiPortLike R>
	auto operator |(L&& lhs,R&& rhs){
		constexpr size_t LL=std::tuple_size<std::remove_cvref_t<L>>::value,RL=std::tuple_size<std::remove_cvref_t<R>>::value;
		constexpr size_t LS=std::remove_cvref_t<decltype(lhs[0])>::size,RS=std::remove_cvref_t<decltype(rhs[0])>::size;
		MultiPort<LS+RS,std::max(LL,RL)> port;
		for(auto [p,r]:std::views::zip(port,rhs)){
			p.template sub<RS>(0)>>r;
		}
		for(auto [p,l]:std::views::zip(port,lhs)){
			p.template sub<LS>(RS)>>l;
		}
		return port;
	}
	template<MultiPortLike T>
	static inline auto reverse(T&& port){
		std::remove_cvref_t<T> res;
		for(auto [o,n]:std::views::zip(port,res)){
			o.reverse()>>n;
		}
		return res;
	}

	template<
		size_t NS  ,size_t ES=NS,size_t SS=NS,size_t WS=ES,
		size_t NL=1,size_t EL=1 ,size_t SL=NL,size_t WL=EL
		>
	struct RectBoard{
		MultiPort<NS,NL> prtN;
		MultiPort<ES,EL> prtE;
		MultiPort<SS,SL> prtS;
		MultiPort<WS,WL> prtW;

		template<
			size_t rNS,size_t rES,size_t rSS,size_t rWS,
			size_t rNL,size_t rEL,size_t rSL,size_t rWL
		>
		static constexpr auto h_merge_brd(const RectBoard<rNS,rES,rSS,rWS,rNL,rEL,rSL,rWL>& rhs){
			return RectBoard<
					NS+rNS ,rES,         SS+rSS ,WS,
					std::max(NL,rNL),rEL,std::max(SL,rSL),WL
			>{};
		}
		template<typename T>
		auto operator |(T&& rhs){
			auto brd=h_merge_brd(rhs);
			
			brd.prtN>>(prtN|rhs.prtN);
			brd.prtE>>rhs.prtE;
			brd.prtS>>(prtS|rhs.prtS);
			brd.prtW>>prtW;
			prtE>>rhs.prtW;
			
			return brd;
		}
		template<
			size_t bNS,size_t bES,size_t bSS,size_t bWS,
			size_t bNL,size_t bEL,size_t bSL,size_t bWL
		>
		static constexpr auto v_merge_brd(const RectBoard<bNS,bES,bSS,bWS,bNL,bEL,bSL,bWL>& btm){
			return RectBoard<
					NS,         ES+bES ,bSS,         WS+bWS ,
					NL,std::max(EL,bEL),bSL,std::max(WL,bWL)
			>{};
		}
		template<typename T>
		auto operator /(T&& btm){
			auto brd=v_merge_brd(btm);
			
			brd.prtN>>prtN;
			brd.prtE>>(btm.prtE|prtE);
			brd.prtS>>btm.prtS;
			brd.prtW>>(btm.prtW|prtW);
			prtS>>btm.prtN;
			return brd;
		}
		auto rot_r(){
			RectBoard<
				WS,NS,ES,SS,
				WL,NL,EL,SL
			> brd;
			
			reverse(prtN)>>brd.prtE;
			prtE>>brd.prtS;
			reverse(prtS)>>brd.prtW;
			prtW>>brd.prtN;

			return brd;
		}
	};

};

#endif //SOCPU_SIM_RECT_BOARD_HPP