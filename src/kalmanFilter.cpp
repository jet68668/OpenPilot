/**
 * \file kalmanFilter.cpp
 *
 * Extended Kalman filter with sparse access governed by boost::indirect_array
 *
 *   \author: jsola@laas.fr
 *
 * \date 25/03/2010
 *
 *
 * \ingroup rtslam
 */

#include "rtslam/kalmanFilter.hpp"
#include "rtslam/observationAbstract.hpp"
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;
		using namespace jblas;
		using namespace ublas;
		using namespace jmath::ublasExtra;

		ExtendedKalmanFilterIndirect::ExtendedKalmanFilterIndirect(size_t _size) :
			size_(_size), x_(size_), P_(size_)
		{
			x_.clear();
			P_.clear();
		}

		void ExtendedKalmanFilterIndirect::predict(const ind_array & ia_x, const mat & F_v, const ind_array & ia_v,
		    const mat & F_u, const sym_mat & U)
		{
			ind_array ia_invariant = ublasExtra::ia_complement(ia_x, ia_v);
			ixaxpy_prod(P_, ia_invariant, F_v, ia_v, ia_v, prod_JPJt(U, F_u));
		}

		void ExtendedKalmanFilterIndirect::predict(const ind_array & ia_x, const mat & F_v, const ind_array & ia_v,
		    const sym_mat & Q)
		{
			ind_array ia_inv = ublasExtra::ia_complement(ia_x, ia_v);
			ixaxpy_prod(P_, ia_inv, F_v, ia_v, ia_v, Q);
		}

		void ExtendedKalmanFilterIndirect::initialize(const ind_array & ia_x, const mat & G_v, const ind_array & ia_rs, const ind_array & ia_l, const mat & G_y, const sym_mat & R){
			ind_array ia_invariant = ia_complement(ia_x, ia_l);
			ixaxpy_prod(P_, ia_invariant, G_v, ia_rs, ia_l, prod_JPJt(R, G_y));
		}

		void ExtendedKalmanFilterIndirect::initialize(const ind_array & ia_x, const mat & G_v, const ind_array & ia_rs, const ind_array & ia_l, const mat & G_y, const sym_mat & R, const mat & G_n, const sym_mat & N){
			ind_array ia_invariant = ia_complement(ia_x, ia_l);
			ixaxpy_prod(P_, ia_invariant, G_v, ia_rs, ia_l, prod_JPJt(R, G_y) + prod_JPJt(N, G_n));
		}

		void ExtendedKalmanFilterIndirect::reparametrize(const ind_array & ia_x, const mat & J_l, const ind_array & ia_old, const ind_array & ia_new){
			ind_array ia_invariant = ia_complement(ia_x, ia_union(ia_old,ia_new));
			ixaxpy_prod(P_, ia_invariant, J_l, ia_old, ia_new);
		}

		void ExtendedKalmanFilterIndirect::computeKalmanGain(const ind_array & ia_x, Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl){
			PJt_tmp.resize(ia_x.size(),inn.size());
			PJt_tmp = prod(project(P_, ia_x, ia_rsl), trans(INN_rsl));
			inn.invertCov();
			K = - prod(PJt_tmp, inn.iP_);
		}

		void ExtendedKalmanFilterIndirect::correct(const ind_array & ia_x, Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl)
		{
			// first the kalman gain
			computeKalmanGain(ia_x, inn, INN_rsl, ia_rsl);

			// mean and covariances update:
			ublas::project(x_, ia_x) += prod(K, inn.x());
			ublas::project(P_, ia_x, ia_x) += prod<sym_mat> (K, trans(PJt_tmp));
		}




		void ExtendedKalmanFilterIndirect::stackCorrection(Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl)
		{
			corrStack.stack.push_back(StackedCorrection(inn, INN_rsl, ia_rsl));
			corrStack.inn_size += inn.size();
		}
		
		void ExtendedKalmanFilterIndirect::correctAllStacked(const ind_array & ia_x)
		{
			PJt_tmp.resize(ia_x.size(), corrStack.inn_size);
			stackedInnovation_x.resize(corrStack.inn_size);
			stackedInnovation_P.resize(corrStack.inn_size);
			stackedInnovation_iP.resize(corrStack.inn_size);
			
			// 1 build PJt_tmp and stackedInnovation
			int col1 = 0;
			for(CorrectionList::iterator corrIter1 = corrStack.stack.begin(); corrIter1 != corrStack.stack.end(); ++corrIter1)
			{
				int nextcol1 = col1 + corrIter1->inn.size();
				
				// 1a update PJt_tmp
				ublas::subrange(PJt_tmp, 0, ia_x.size(), col1, nextcol1) =
					ublas::prod(ublas::project(P_, ia_x, corrIter1->ia_rsl), trans(corrIter1->INN_rsl));
				
				// 1b update diagonal of stackedInnovation
				ublas::subrange(stackedInnovation_x, col1, nextcol1) = corrIter1->inn.x();
				ublas::subrange(stackedInnovation_P, col1, nextcol1, col1, nextcol1) = corrIter1->inn.P();
				
				int col2 = nextcol1;
				CorrectionList::iterator corrIter2 = corrIter1; ++corrIter1;
				for(; corrIter2 != corrStack.stack.end(); ++corrIter2)
				{
					int nextcol2 = col2 + corrIter2->inn.size();
					
					// update off diagonal stackedInnovation
					mat m = ublas::prod(corrIter1->INN_rsl, ublas::project(P_, corrIter1->ia_rsl, corrIter2->ia_rsl));
					ublas::subrange(stackedInnovation_P, col1, nextcol1, col2, nextcol2) = ublas::prod(m, trans(corrIter2->INN_rsl));
					col2 = nextcol2;
				}
				
				col1 = nextcol1;
			}
			
			// 2 compute Kalman gain
			ublasExtra::lu_inv(stackedInnovation_P, stackedInnovation_iP);
			K = - prod(PJt_tmp, stackedInnovation_iP);
			
			// 3 correct
			ublas::project(x_, ia_x) += prod(K, stackedInnovation_x);
			ublas::project(P_, ia_x, ia_x) += prod<sym_mat>(K, trans(PJt_tmp));
			
			corrStack.clear();
		}


	}
}
