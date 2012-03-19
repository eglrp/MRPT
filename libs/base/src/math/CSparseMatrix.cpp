/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                       http://www.mrpt.org/                                |
   |                                                                           |
   |   Copyright (C) 2005-2011  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include <mrpt/base.h>  // Precompiled headers

#include <mrpt/math/CSparseMatrix.h>

// Read the note in the .h: Use embedded headers even for compiling against
//  a system library.
//#ifdef MRPT_HAS_CSPARSE_SYSTEM
//#	include <cs.h>
//#else
extern "C"{
#include <mrpt/otherlibs/CSparse/cs.h>
}
//#endif


using std::string;
using std::cout;
using std::endl;
using namespace mrpt;
using namespace mrpt::math;
using namespace mrpt::utils;


/** Copy constructor */
CSparseMatrix::CSparseMatrix(const CSparseMatrix & other)
{
	construct_from_existing_cs(other.sparse_matrix);
	copy(&other.sparse_matrix);
}

/** Copy constructor from an existing "cs" CSparse data structure */
CSparseMatrix::CSparseMatrix(const cs  * const sm)
{
	construct_from_existing_cs(*sm);
	copy(sm);
}

/** Copy the data from an existing "cs" CSparse data structure */
void  CSparseMatrix::copy(const cs  * const sm)
{
	ASSERTMSG_(sm->nz==-1, "I expected a column-compressed sparse matrix, not a triplet form.")

	sparse_matrix.m = sm->m;
	sparse_matrix.n = sm->n;
	sparse_matrix.nz = sm->nz;
	sparse_matrix.nzmax = sm->nzmax;

	// Do it in a different way depending on it being triplet or compressed format:
	::memcpy(sparse_matrix.i,sm->i,sizeof(int)*sm->nzmax);
	::memcpy(sparse_matrix.p,sm->p,sizeof(int)*(sm->n+1));
	::memcpy(sparse_matrix.x,sm->x,sizeof(double)*sm->nzmax);
}

/** Fast copy the data from an existing "cs" CSparse data structure, copying the pointers and leaving NULLs in the source structure. */
void  CSparseMatrix::copy_fast(cs  * const sm)
{
	// This method will work for either triplet or compressed format:

	// Free previous contents, if any.
	internal_free_mem();

	// Fast copy / Move:
	sparse_matrix.m = sm->m;
	sparse_matrix.n = sm->n;
	sparse_matrix.nz = sm->nz;
	sparse_matrix.nzmax = sm->nzmax;

	sparse_matrix.i = sm->i;
	sparse_matrix.p = sm->p;
	sparse_matrix.x = sm->x;

	// Mark source as empty:
	sm->i=NULL;
	sm->p=NULL;
	sm->x=NULL;
}


// Dtor
CSparseMatrix::~CSparseMatrix()
{
	internal_free_mem();
}

/** Erase all previous contents and leave the matrix as a "triplet" 1x1 matrix without any data. */
void CSparseMatrix::clear()
{
	// Free old data:
	internal_free_mem();

	// Init as 1x1 triplet:
	sparse_matrix.nzmax = 1;
	sparse_matrix.m = 1; //nRows;
	sparse_matrix.n = 1; // nCols;
	sparse_matrix.i = (int*)malloc(sizeof(int)*sparse_matrix.nzmax);
	sparse_matrix.p = (int*)malloc(sizeof(int)*(sparse_matrix.n+1));
	sparse_matrix.x = (double*)malloc(sizeof(double)*sparse_matrix.nzmax);
	sparse_matrix.nz = 0;  // >=0 -> triplet format.
}

/** free buffers (deallocate the memory of the i,p,x buffers) */
void CSparseMatrix::internal_free_mem()
{
  cs_free(sparse_matrix.i);
  cs_free(sparse_matrix.p);
  cs_free(sparse_matrix.x);
}

/** Initialization from a triplet "cs", which is first compressed */
void CSparseMatrix::construct_from_triplet(const cs & triplet)
{
	cs * sm = cs_compress(&triplet);
	copy_fast(sm);
	cs_spfree(sm); // This will release just the "cs" structure itself, not the internal buffers, by now set to NULL.
}

// To be called by constructors only, assume previous pointers are trash and overwrite them:
// This ONLY allocate the memory, doesn't fill it.
void CSparseMatrix::construct_from_existing_cs(const cs &sm)
{
	ASSERTMSG_(sm.nz==-1, "I expected a column-compressed sparse matrix, not a triplet form.")
	sparse_matrix.i = (int*)malloc(sizeof(int)*sm.nzmax);
	sparse_matrix.p = (int*)malloc(sizeof(int)*(sm.n+1));
	sparse_matrix.x = (double*)malloc(sizeof(double)*sm.nzmax);
}

/** Create an initially empty sparse matrix, in the "triplet" form.
*  Notice that you must call "compressFromTriplet" after populating the matrix and before using the math operatons on this matrix.
*  The initial size can be later on extended with insert_entry() or setRowCount() & setColCount().
*/
CSparseMatrix::CSparseMatrix(const size_t nRows, const size_t nCols)
{
	sparse_matrix.nzmax = 1;
	sparse_matrix.m = nRows;
	sparse_matrix.n = nCols;
	sparse_matrix.i = (int*)malloc(sizeof(int)*sparse_matrix.nzmax);
	sparse_matrix.p = (int*)malloc(sizeof(int)*(sparse_matrix.n+1));
	sparse_matrix.x = (double*)malloc(sizeof(double)*sparse_matrix.nzmax);
	sparse_matrix.nz = 0;
}


/** Insert an element into a "cs", return false on error. */
void CSparseMatrix::insert_entry(const size_t row, const size_t col, const double val )
{
	if (!isTriplet())
		THROW_EXCEPTION("insert_entry() is only available for sparse matrix in 'triplet' format.")
	if (!cs_entry(&sparse_matrix,row,col,val))
		THROW_EXCEPTION("Error inserting element in sparse matrix (out of mem?)")
}

/** Copy operator from another existing object */
void CSparseMatrix::operator = (const CSparseMatrix & other)
{
	if (&other==this) return;

	cs_free(sparse_matrix.i);
	cs_free(sparse_matrix.p);
	cs_free(sparse_matrix.x);

	sparse_matrix.i = (int*)malloc(sizeof(int)*other.sparse_matrix.nzmax);
	sparse_matrix.p = (int*)malloc(sizeof(int)*(other.sparse_matrix.n+1));
	sparse_matrix.x = (double*)malloc(sizeof(double)*other.sparse_matrix.nzmax);
	copy(&other.sparse_matrix);
}


void CSparseMatrix::add_AB(const CSparseMatrix & A,const CSparseMatrix & B)
{
	ASSERT_(A.getColCount()==B.getColCount() && A.getRowCount()==B.getRowCount())

	cs * sm = cs_add(&(A.sparse_matrix), &(B.sparse_matrix),1,1);
	ASSERT_(sm)
	this->copy_fast(sm);
	cs_spfree(sm);
}

void CSparseMatrix::multiply_AB(const CSparseMatrix & A,const CSparseMatrix & B)
{
	ASSERT_(A.getColCount()==B.getRowCount())

	cs * sm = cs_multiply(&(A.sparse_matrix), &(B.sparse_matrix));
	ASSERT_(sm)
	this->copy_fast(sm);
	cs_spfree(sm);
}

void CSparseMatrix::multiply_Ab(const mrpt::vector_double &b, mrpt::vector_double &out_res) const
{
	ASSERT_EQUAL_(int(b.size()), int(getColCount()))
	out_res.resize( getRowCount() );
	const double * y = &(b[0]);
	double * x = &(out_res[0]);
	cs_gaxpy(&sparse_matrix,y,x);
}


CSparseMatrix CSparseMatrix::transpose() const
{
	cs * sm = cs_transpose(&sparse_matrix,1);
	ASSERT_(sm)
	CSparseMatrix SM(sm);
	cs_spfree(sm);
	return SM;
}

/** Static method to convert a "cs" structure into a dense representation of the sparse matrix.
*/
void CSparseMatrix::cs2dense(const cs& SM, CMatrixDouble &d_M)
{
	d_M.zeros(SM.m,SM.n);
	if (SM.nz>=0)  // isTriplet ??
	{	// It's in triplet form.
		for (int idx=0;idx<SM.nz; ++idx)
			d_M(SM.i[idx],SM.p[idx]) += SM.x[idx];  // += since the convention is that duplicate (i,j) entries add to each other.
	}
	else
	{	// Column compressed format:
		ASSERT_(SM.x)  // JL: Could it be NULL and be OK???

		for (int j = 0 ; j < SM.n ; j++)
        {
			const int p0 = SM.p [j];
			const int p1 = SM.p [j+1];
            for (int p = p0 ; p < p1 ; p++)
				d_M(SM.i[p],j) += SM.x[p];  // += since the convention is that duplicate (i,j) entries add to each other.
        }
	}
}

void CSparseMatrix::get_dense(CMatrixDouble &d_M) const
{
	cs2dense(sparse_matrix, d_M);
}

void CSparseMatrix::compressFromTriplet()
{
	if (!isTriplet())
		THROW_EXCEPTION("compressFromTriplet(): Matrix is already in column-compressed format.")

	cs * sm = cs_compress(&this->sparse_matrix);
	copy_fast(sm);
	cs_spfree(sm); // This will release just the "cs" structure itself, not the internal buffers, now set to NULL.
}


/** save as a dense matrix to a text file \return False on any error.
*/
bool CSparseMatrix::saveToTextFile_dense(const std::string &filName)
{
	CMatrixDouble    dense;
	this->get_dense(dense);
	try
	{
		dense.saveToTextFile(filName);
		return true;
	}
	catch(...) { return false; }
}



// ===============  START OF:   CSparseMatrix::CholeskyDecomp  inner class  ==============================

/** Constructor from a square semidefinite-positive sparse matrix.
*   The actual Cholesky decomposition takes places in this constructor.
*  \exception std::runtime_error On non-square input matrix.
*  \exception mrpt::math::CExceptionNotDefPos On non-semidefinite-positive matrix as input.
*/
CSparseMatrix::CholeskyDecomp::CholeskyDecomp(const CSparseMatrix &SM) :
	m_symbolic_structure	(NULL),
	m_numeric_structure		(NULL),
	m_originalSM			(&SM)
{
	ASSERT_(SM.getColCount()==SM.getRowCount())
	ASSERT_(SM.isColumnCompressed())

	// symbolic decomposition:
	m_symbolic_structure = cs_schol(1 /* order */, &m_originalSM->sparse_matrix );

	// numeric decomposition:
	m_numeric_structure = cs_chol(&m_originalSM->sparse_matrix,m_symbolic_structure);

	if (!m_numeric_structure)
		throw mrpt::math::CExceptionNotDefPos("CSparseMatrix::CholeskyDecomp: Not positive definite matrix.");
}

// Destructor:
CSparseMatrix::CholeskyDecomp::~CholeskyDecomp()
{
	cs_nfree(m_numeric_structure);
	cs_sfree(m_symbolic_structure);
}

/** Return the L matrix (L*L' = M), as a dense matrix. */
void CSparseMatrix::CholeskyDecomp::get_L(CMatrixDouble &L) const
{
	CSparseMatrix::cs2dense(*m_numeric_structure->L, L);
}


/** Return the vector from a back-substitution step that solves: Ux=b   */
void CSparseMatrix::CholeskyDecomp::backsub(
	const mrpt::vector_double &b,
	mrpt::vector_double &sol) const
{
	mrpt::vector_double tmp = b;
	sol = b;

	cs_ipvec(m_symbolic_structure->pinv,&b[0],&tmp[0],b.size()); /* tmp = PERMUT*b */
	//permute con. pivoting
	cs_lsolve(m_numeric_structure->L,&tmp[0]);   /* tmp = L\tmp */
	cs_ltsolve(m_numeric_structure->L,&tmp[0]);  /* tmp = L'\tmp */
	cs_pvec(m_symbolic_structure->pinv,&tmp[0],&sol[0],b.size()); /* sol = PERMUT'*tmp */
	//unpermute con. pivoting

	// Result is now in "sol".
}


/** Update the Cholesky factorization from an updated vesion of the original input, square definite-positive sparse matrix.
*  NOTE: This new matrix MUST HAVE exactly the same sparse structure than the original one.
*/
void CSparseMatrix::CholeskyDecomp::update(const CSparseMatrix &new_SM)
{
	ASSERTMSG_(m_originalSM->sparse_matrix.nzmax == new_SM.sparse_matrix.nzmax, "New matrix doesn't have the same sparse structure!");
	ASSERTMSG_(m_originalSM->sparse_matrix.n == new_SM.sparse_matrix.n, "New matrix doesn't have the same sparse structure!");

	m_originalSM = &new_SM; // Just copy the reference.

	// Release old data:
	cs_nfree(m_numeric_structure);
	m_numeric_structure = NULL;

	// numeric decomposition:
	m_numeric_structure = cs_chol(&m_originalSM->sparse_matrix,m_symbolic_structure);
	if (!m_numeric_structure)
		throw mrpt::math::CExceptionNotDefPos("CholeskyDecomp::update: Not positive definite matrix.");
}

// ===============    END OF: CSparseMatrix::CholeskyDecomp  inner class  ==============================