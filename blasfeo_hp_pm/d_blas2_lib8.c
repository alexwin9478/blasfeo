/**************************************************************************************************
*                                                                                                 *
* This file is part of BLASFEO.                                                                   *
*                                                                                                 *
* BLASFEO -- BLAS for embedded optimization.                                                      *
* Copyright (C) 2019 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* The 2-Clause BSD License                                                                        *
*                                                                                                 *
* Redistribution and use in source and binary forms, with or without                              *
* modification, are permitted provided that the following conditions are met:                     *
*                                                                                                 *
* 1. Redistributions of source code must retain the above copyright notice, this                  *
*    list of conditions and the following disclaimer.                                             *
* 2. Redistributions in binary form must reproduce the above copyright notice,                    *
*    this list of conditions and the following disclaimer in the documentation                    *
*    and/or other materials provided with the distribution.                                       *
*                                                                                                 *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND                 *
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED                   *
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE                          *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR                 *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES                  *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;                    *
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND                     *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT                      *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS                   *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                    *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include <blasfeo_common.h>
#include <blasfeo_d_kernel.h>
#include <blasfeo_d_blas.h>
#include <blasfeo_d_aux.h>
#if defined(BLASFEO_REF_API)
#include <blasfeo_d_blasfeo_ref_api.h>
#endif



void blasfeo_hp_dgemv_n(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{

	if(m<0)
		return;

	const int ps = 8;

	int i;

	int sda = sA->cn;
	double *pA = sA->pA + aj*ps + ai/ps*ps*sda;
	double *x = sx->pa + xi;
	double *y = sy->pa + yi;
	double *z = sz->pa + zi;

	int air = ai%ps;

	// clean up at the beginning
	if(air!=0)
		{
		kernel_dgemv_n_8_gen_lib8(n, &alpha, pA, x, &beta, y-air, z-air, air, m+air);
		pA += ps*sda;
		y += ps - air;
		z += ps - air;
		m -= ps - air;
		}
	// main loop
	i = 0;
#if 1
	for( ; i<m-15; i+=16)
		{
		kernel_dgemv_n_16_lib8(n, &alpha, &pA[i*sda], sda, x, &beta, &y[i], &z[i]);
		}
	if(i<m-7)
		{
		kernel_dgemv_n_8_lib8(n, &alpha, &pA[i*sda], x, &beta, &y[i], &z[i]);
		i+=8;
		}
#else
	for( ; i<m-7; i+=8)
		{
		kernel_dgemv_n_8_lib8(n, &alpha, &pA[i*sda], x, &beta, &y[i], &z[i]);
		}
#endif
	if(i<m)
		{
		kernel_dgemv_n_8_vs_lib8(n, &alpha, &pA[i*sda], x, &beta, &y[i], &z[i], m-i);
		}
		
	return;

	}



void blasfeo_hp_dgemv_t(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{

	if(n<=0)
		return;
	
	const int ps = 8;

	int i;

	int sda = sA->cn;
	double *pA = sA->pA + aj*ps + ai/ps*ps*sda; // + ai%ps; // TODO remove air from here !!!!!!!!!
	double *x = sx->pa + xi;
	double *y = sy->pa + yi;
	double *z = sz->pa + zi;

	int offsetA = ai%ps;

	i = 0;
#if 0
	for( ; i<n-7; i+=8)
		{
		kernel_dgemv_t_8_lib4(m, &alpha, offsetA, &pA[i*ps], sda, x, &beta, &y[i], &z[i]);
		}
	if(i<n-3)
		{
		kernel_dgemv_t_4_lib4(m, &alpha, offsetA, &pA[i*ps], sda, x, &beta, &y[i], &z[i]);
		i+=4;
		}
#else
	for( ; i<n-7; i+=8)
		{
		kernel_dgemv_t_8_lib8(m, &alpha, offsetA, &pA[i*ps], sda, x, &beta, &y[i], &z[i]);
		}
#endif
	if(i<n)
		{
		kernel_dgemv_t_8_vs_lib8(m, &alpha, offsetA, &pA[i*ps], sda, x, &beta, &y[i], &z[i], n-i);
		}
	
	return;

	}



void blasfeo_hp_dgemv_nt(int m, int n, double alpha_n, double alpha_t, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx_n, int xi_n, struct blasfeo_dvec *sx_t, int xi_t, double beta_n, double beta_t, struct blasfeo_dvec *sy_n, int yi_n, struct blasfeo_dvec *sy_t, int yi_t, struct blasfeo_dvec *sz_n, int zi_n, struct blasfeo_dvec *sz_t, int zi_t)
	{

	if(m<=0 | n<=0)
		return;

	const int ps = 8;

#if 0
	blasfeo_hp_dgemv_n(m, n, alpha_n, sA, ai, aj, sx_n, xi_n, beta_n, sy_n, yi_n, sz_n, zi_n);
	blasfeo_hp_dgemv_t(m, n, alpha_t, sA, ai, aj, sx_t, xi_t, beta_t, sy_t, yi_t, sz_t, zi_t);
	return;
#endif
	int sda = sA->cn;
	double *pA = sA->pA + aj*ps + ai/ps*ps*sda;
	double *x_n = sx_n->pa + xi_n;
	double *x_t = sx_t->pa + xi_t;
	double *y_n = sy_n->pa + yi_n;
	double *y_t = sy_t->pa + yi_t;
	double *z_n = sz_n->pa + zi_n;
	double *z_t = sz_t->pa + zi_t;

	int offsetA = ai%ps;

	int ii;

	// copy and scale y_n int z_n
	ii = 0;
	for(; ii<m-3; ii+=4)
		{
		z_n[ii+0] = beta_n*y_n[ii+0];
		z_n[ii+1] = beta_n*y_n[ii+1];
		z_n[ii+2] = beta_n*y_n[ii+2];
		z_n[ii+3] = beta_n*y_n[ii+3];
		}
	for(; ii<m; ii++)
		{
		z_n[ii+0] = beta_n*y_n[ii+0];
		}
	
	ii = 0;
#if 0
	for(; ii<n-5; ii+=6)
		{
		kernel_dgemv_nt_6_lib4(m, &alpha_n, &alpha_t, pA+ii*ps, sda, x_n+ii, x_t, &beta_t, y_t+ii, z_n, z_t+ii);
		}
#endif
	for(; ii<n-7; ii+=8)
		{
		kernel_dgemv_nt_8_lib8(m, &alpha_n, &alpha_t, offsetA, pA+ii*ps, sda, x_n+ii, x_t, &beta_t, y_t+ii, z_n, z_t+ii);
		}
	if(ii<n)
		{
		kernel_dgemv_nt_8_vs_lib8(m, &alpha_n, &alpha_t, offsetA, pA+ii*ps, sda, x_n+ii, x_t, &beta_t, y_t+ii, z_n, z_t+ii, n-ii);
		}
	
	return;

	}



void blasfeo_hp_dsymv_l(int m, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{

	if(m<=0)
		return;

	const int ps = 8;

	int ii, m1;

	int sda = sA->cn;
	double *pA = sA->pA + aj*ps + ai/ps*ps*sda; // + ai%ps; // TODO offsetA !!!!!!!
	double *x = sx->pa + xi;
	double *y = sy->pa + yi;
	double *z = sz->pa + zi;

	double air = ai%ps;

	// copy and scale y int z
	ii = 0;
	for(; ii<m-3; ii+=4)
		{
		z[ii+0] = beta*y[ii+0];
		z[ii+1] = beta*y[ii+1];
		z[ii+2] = beta*y[ii+2];
		z[ii+3] = beta*y[ii+3];
		}
	for(; ii<m; ii++)
		{
		z[ii+0] = beta*y[ii+0];
		}

	// clean up at the beginning
	if(air!=0)
		{
		m1 = ps-air;
		m1 = m<m1 ? m : m1;
		kernel_dsymv_l_8_gen_lib8(m-m1, &alpha, air, &pA[0], sda, &x[0], &z[0], m1);
		pA += m1*ps + sda*ps;
		x += m1;
		z += m1;
		m -= m1;
		}

	// main loop
	ii = 0;
	for(; ii<m-7; ii+=8)
		{
		kernel_dsymv_l_8_lib8(m-ii-8, &alpha, &pA[ii*ps+ii*sda], sda, &x[ii], &z[ii]);
		}
	// clean up at the end
	if(ii<m)
		{
		kernel_dsymv_l_8_vs_lib8(0, &alpha, &pA[ii*ps+ii*sda], sda, &x[ii], &z[ii], m-ii);
		}

	return;
	}


// m >= n
void blasfeo_hp_dsymv_l_mn(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{

	if(m<n)
		n = m;

	if(m<=0 | n<=0)
		return;
	
	const int ps = 8;

	int ii, m1;

	int sda = sA->cn;
	double *pA = sA->pA + aj*ps + ai/ps*ps*sda; // + ai%ps;
	double *x = sx->pa + xi;
	double *y = sy->pa + yi;
	double *z = sz->pa + zi;

	double air = ai%ps;

	// copy and scale y int z
	ii = 0;
	for(; ii<m-3; ii+=4)
		{
		z[ii+0] = beta*y[ii+0];
		z[ii+1] = beta*y[ii+1];
		z[ii+2] = beta*y[ii+2];
		z[ii+3] = beta*y[ii+3];
		}
	for(; ii<m; ii++)
		{
		z[ii+0] = beta*y[ii+0];
		}

	// clean up at the beginning
	if(air!=0)
		{
		m1 = ps-air;
		m1 = n<m1 ? n : m1;
		kernel_dsymv_l_8_gen_lib8(m-m1, &alpha, air, &pA[0], sda, &x[0], &z[0], m1);
		pA += m1*ps + sda*ps;
		x += m1;
		z += m1;
		m -= m1;
		n -= m1;
		}

	// main loop
	ii = 0;
	for(; ii<n-7; ii+=8)
		{
		kernel_dsymv_l_8_lib8(m-ii-8, &alpha, &pA[ii*ps+ii*sda], sda, &x[ii], &z[ii]);
		}
	// clean up at the end
	if(ii<n)
		{
		kernel_dsymv_l_8_vs_lib8(m-n, &alpha, &pA[ii*ps+ii*sda], sda, &x[ii], &z[ii], n-ii);
		}
	
	return;
	}



// m >= n
void blasfeo_hp_dtrmv_lnn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrmv_lnn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrmv_lnn: feature not implemented yet\n");
	exit(1);
#endif
	}



// m >= n
//void blasfeo_hp_dtrmv_lnu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
//	{
//#if defined(BLASFEO_REF_API)
//	blasfeo_ref_dtrmv_lnu(m, sA, ai, aj, sx, xi, sz, zi);
//#else
//	printf("\nblasfeo_dtrmv_lnu: feature not implemented yet\n");
//	exit(1);
//#endif
//	}



// m >= n
void blasfeo_hp_dtrmv_ltn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrmv_ltn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrmv_ltn: feature not implemented yet\n");
	exit(1);
#endif
	}



// m >= n
//void blasfeo_hp_dtrmv_ltu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
//	{
//#if defined(BLASFEO_REF_API)
//	blasfeo_ref_dtrmv_ltu(m, sA, ai, aj, sx, xi, sz, zi);
//#else
//	printf("\nblasfeo_dtrmv_ltu: feature not implemented yet\n");
//	exit(1);
//#endif
//	}



void blasfeo_hp_dtrmv_unn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrmv_unn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrmv_unn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrmv_utn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrmv_utn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrmv_utn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_lnn_mn(int m, int n, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_lnn_mn(m, n, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_lnn_mn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_lnn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_lnn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_lnn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_lnu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_lnu(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_lnu: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_ltn_mn(int m, int n, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_ltn_mn(m, n, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_ltn_mn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_ltn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_ltn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_ltn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_ltu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_ltu(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_ltu: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_unn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_unn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_unn: feature not implemented yet\n");
	exit(1);
#endif
	}



void blasfeo_hp_dtrsv_utn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
#if defined(BLASFEO_REF_API)
	blasfeo_ref_dtrsv_utn(m, sA, ai, aj, sx, xi, sz, zi);
#else
	printf("\nblasfeo_dtrsv_utn: feature not implemented yet\n");
	exit(1);
#endif
	}



#if defined(LA_HIGH_PERFORMANCE)



void blasfeo_dgemv_n(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dgemv_n(m, n, alpha, sA, ai, aj, sx, xi, beta, sy, yi, sz, zi);
	}



void blasfeo_dgemv_t(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dgemv_t(m, n, alpha, sA, ai, aj, sx, xi, beta, sy, yi, sz, zi);
	}



void blasfeo_dgemv_nt(int m, int n, double alpha_n, double alpha_t, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx_n, int xi_n, struct blasfeo_dvec *sx_t, int xi_t, double beta_n, double beta_t, struct blasfeo_dvec *sy_n, int yi_n, struct blasfeo_dvec *sy_t, int yi_t, struct blasfeo_dvec *sz_n, int zi_n, struct blasfeo_dvec *sz_t, int zi_t)
	{
	blasfeo_hp_dgemv_nt(m, n, alpha_n, alpha_t, sA, ai, aj, sx_n, xi_n, sx_t, xi_t, beta_n, beta_t, sy_n, yi_n, sy_t, yi_t, sz_n, zi_n, sz_t, zi_t);
	}



void blasfeo_dsymv_l(int m, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dsymv_l(m, alpha, sA, ai, aj, sx, xi, beta, sy, yi, sz, zi);
	}



void blasfeo_dsymv_l_mn(int m, int n, double alpha, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, double beta, struct blasfeo_dvec *sy, int yi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dsymv_l_mn(m, n, alpha, sA, ai, aj, sx, xi, beta, sy, yi, sz, zi);
	}



void blasfeo_dtrmv_lnn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrmv_lnn(m, sA, ai, aj, sx, xi, sz, zi);
	}



//void blasfeo_dtrmv_lnu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
//	{
//	blasfeo_hp_dtrmv_lnu(m, sA, ai, aj, sx, xi, sz, zi);
//	}



void blasfeo_dtrmv_ltn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrmv_ltn(m, sA, ai, aj, sx, xi, sz, zi);
	}



//void blasfeo_dtrmv_ltu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
//	{
//	blasfeo_hp_dtrmv_ltu(m, sA, ai, aj, sx, xi, sz, zi);
//	}



void blasfeo_dtrmv_unn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrmv_unn(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrmv_utn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrmv_utn(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_lnn_mn(int m, int n, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_lnn_mn(m, n, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_lnn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_lnn(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_lnu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_lnu(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_ltn_mn(int m, int n, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_ltn_mn(m, n, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_ltn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_ltn(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_ltu(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_ltu(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_unn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_unn(m, sA, ai, aj, sx, xi, sz, zi);
	}



void blasfeo_dtrsv_utn(int m, struct blasfeo_dmat *sA, int ai, int aj, struct blasfeo_dvec *sx, int xi, struct blasfeo_dvec *sz, int zi)
	{
	blasfeo_hp_dtrsv_utn(m, sA, ai, aj, sx, xi, sz, zi);
	}



#endif

