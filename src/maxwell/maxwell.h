#ifndef MAXWELL_H
#define MAXWELL_H

#include <scalar.h>
#include <matrices.h>

#if defined(HAVE_LIBFFTW)
#  include <fftw.h>
#  include <rfftw.h>
#  ifdef HAVE_MPI
#    include <fftwnd_mpi.h>
#  endif
#elif defined(HAVE_LIBDFFTW)
#  include <dfftw.h>
#  include <drfftw.h>
#  ifdef HAVE_MPI
#    include <dfftwnd_mpi.h>
#  endif
#elif defined(HAVE_LIBSFFTW)
#  include <sfftw.h>
#  include <srfftw.h>
#  ifdef HAVE_MPI
#    include <sfftwnd_mpi.h>
#  endif
#endif

/* This data structure is designed to hold k+G related data at a given
   point.  kmag is the length of the k+G vector.  The m and n vectors are
   orthonormal vectors orthogonal to (kx,ky,kz).  These are used
   as the basis for the H vector (to maintain transversality). */
typedef struct {
     real kmag;
     real mx, my, mz;
     real nx, ny, nz;
} k_data;

/* Data structure to hold the upper triangle of a symmetric real matrix
   (e.g. the dielectric tensor). */
typedef struct {
     real m00, m01, m02,
               m11, m12,
                    m22;
} symmetric_matrix;

typedef enum { 
     TE_POLARIZATION, TM_POLARIZATION, NO_POLARIZATION
} polarization_t;

typedef struct {
     int nx, ny, nz;
     int local_nx, local_ny;
     int local_x_start, local_y_start;
     int last_dim, other_dims;

     int num_bands;
     int N, local_N, N_start, alloc_N;

     int fft_output_size;

     int num_fft_bands;

     real current_k[3];
     polarization_t polarization;

#ifdef HAVE_FFTW
#  ifdef HAVE_MPI
     fftwnd_mpi_plan plan, iplan;
#  else
#    ifdef SCALAR_COMPLEX
     fftwnd_plan plan, iplan;
#    else
     rfftwnd_plan plan, iplan;
#    endif
#  endif
#endif

     scalar *fft_data;
     
     k_data *k_plus_G;
     real *k_plus_G_normsqr;

     symmetric_matrix *eps_inv;
     real eps_inv_mean;
} maxwell_data;

extern maxwell_data *create_maxwell_data(int nx, int ny, int nz,
					 int *local_N, int *N_start,
					 int *alloc_N,
					 int num_bands,
					 int num_fft_bands);
extern void destroy_maxwell_data(maxwell_data *d);

extern void update_maxwell_data_k(maxwell_data *d, real k[3],
				  real G1[3], real G2[3], real G3[3]);

extern void set_maxwell_data_polarization(maxwell_data *d,
					  polarization_t polarization);

typedef real (*dielectric_function) (real r[3], void *epsilon_data);

extern void set_maxwell_dielectric(maxwell_data *md,
				   int mesh_size[3],
				   real R1[3], real R2[3], real R3[3],
				   dielectric_function epsilon,
				   void *epsilon_data);

extern void maxwell_operator(evectmatrix Xin, evectmatrix Xout, void *data,
			     int is_current_eigenvector);
extern void maxwell_preconditioner(evectmatrix Xin, evectmatrix Xout,
				   void *data,
				   evectmatrix Y, real *eigenvals);

extern void maxwell_constraint(evectmatrix X, void *data);

typedef struct {
     maxwell_data *d;
     evectmatrix T;
     real target_frequency;
} maxwell_target_data;

extern maxwell_target_data *create_maxwell_target_data(maxwell_data *d,
						       real target_frequency);
extern void destroy_maxwell_target_data(maxwell_target_data *d);
extern void maxwell_target_operator(evectmatrix Xin, evectmatrix Xout,
				    void *data,
				    int is_current_eigenvector);
extern void maxwell_target_preconditioner(evectmatrix Xin, evectmatrix Xout,
					  void *data,
					  evectmatrix Y, real *eigenvals);

#endif /* MAXWELL_H */