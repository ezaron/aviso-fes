//
// gcc fes_uv_drifters.c -O -g -I /home/ezaron/opt/include -L /home/ezaron/opt/lib -l fes -L /usr/lib -l netcdf
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ezaron/opt/lib
// ldconfig
// How to compile libfes with -O3 and -march=native ?
// Also see /mnt/Data1/ezaron/Drifters/v1.04c/code.c and code_p.c .
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <math.h>

#include "fes.h"

// Path to the configuration file and data used to test the library
// Change these settings to your liking.
#define INI_TIDE "../../data/fes2014/ocean_tide.ini"
#define INI_RADIAL "../../data/fes2014/load_tide.ini"
//#define INI_U "../../data/fes2014/eastward_velocity.ini"
//#define INI_V "../../data/fes2014/northward_velocity.ini"
#define INI_U "/mnt/SSD/ezaron/FES2014/CNESpredictionsoftware20220307/data/fes2014/eastward_velocity.ini"
#define INI_V "/mnt/SSD/ezaron/FES2014/CNESpredictionsoftware20220307/data/fes2014/northward_velocity.ini"


void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create driftertraj_1.04c.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* dimension ids */
    int TIME_dim;

    /* dimension lengths */
    //    size_t TIME_len = NC_UNLIMITED;
    size_t TIME_len;

    /* variable ids */
    int ID_id;
    int WMO_id;
    int TIME_id;
    int LON_id;
    int LAT_id;
    int LON_ERR_id;
    int LAT_ERR_id;
    int U_id;
    int V_id;
    int U_ERR_id;
    int V_ERR_id;
    int LTB_id;
    int GP_id;
    int DS_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_ID 1
#   define RANK_WMO 1
#   define RANK_TIME 1
#   define RANK_LON 1
#   define RANK_LAT 1
#   define RANK_LON_ERR 1
#   define RANK_LAT_ERR 1
#   define RANK_U 1
#   define RANK_V 1
#   define RANK_U_ERR 1
#   define RANK_V_ERR 1
#   define RANK_LTB 1
#   define RANK_GP 1
#   define RANK_DS 1

    /* variable shapes */
    int ID_dims[RANK_ID];
    int WMO_dims[RANK_WMO];
    int TIME_dims[RANK_TIME];
    int LON_dims[RANK_LON];
    int LAT_dims[RANK_LAT];
    int LON_ERR_dims[RANK_LON_ERR];
    int LAT_ERR_dims[RANK_LAT_ERR];
    int U_dims[RANK_U];
    int V_dims[RANK_V];
    int U_ERR_dims[RANK_U_ERR];
    int V_ERR_dims[RANK_V_ERR];
    int LTB_dims[RANK_LTB];
    int GP_dims[RANK_GP];
    int DS_dims[RANK_DS];

    stat = nc_open("driftertraj_1.04c.nc", NC_CLOBBER, &ncid);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_inq_dimid(ncid, "TIME", &TIME_dim);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_inq_dimlen(ncid, TIME_dim, &TIME_len);
    check_err(stat,__LINE__,__FILE__);

    (void)fprintf(stdout,"Found TIME_len = %lu \n", TIME_len);
    fflush(stdout);

    // Allocate:
    double* TIME;
    double* LON;
    double* LAT;
    //    ptr = (int*)malloc(n * sizeof(int));
    TIME = (double*)malloc( TIME_len * sizeof(double));
    LON  = (double*)malloc( TIME_len * sizeof(double));
    LAT  = (double*)malloc( TIME_len * sizeof(double));
    
    /* inq variables */
    (void)fprintf(stdout,"Loading TIME values\n");
    fflush(stdout);
    TIME_dims[0] = TIME_dim;
    //    stat = nc_def_var(ncid, "TIME", NC_DOUBLE, RANK_TIME, TIME_dims, &TIME_id);
    stat = nc_inq_varid(ncid, "TIME", &TIME_id);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_get_var_double(ncid, TIME_id, &TIME[0]);
    check_err(stat,__LINE__,__FILE__);

    (void)fprintf(stdout,"Loading LON values\n");
    fflush(stdout);
    LON_dims[0] = TIME_dim;
    //    stat = nc_def_var(ncid, "LON", NC_DOUBLE, RANK_LON, LON_dims, &LON_id);
    stat = nc_inq_varid(ncid, "LON", &LON_id);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_get_var_double(ncid, LON_id, &LON[0]);
    check_err(stat,__LINE__,__FILE__);

    (void)fprintf(stdout,"Loading LAT values\n");
    fflush(stdout);
    LAT_dims[0] = TIME_dim;
    //    stat = nc_def_var(ncid, "LAT", NC_DOUBLE, RANK_LAT, LAT_dims, &LAT_id);
    stat = nc_inq_varid(ncid, "LAT", &LAT_id);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_get_var_double(ncid, LAT_id, &LAT[0]);
    check_err(stat,__LINE__,__FILE__);

    // Done.
    (void)fprintf(stdout,"Closing the file\n");
    fflush(stdout);
    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);

    // Allocate the arrays for the tidal current predictions:
    (void)fprintf(stdout,"Allocating space for the predictions\n");
    fflush(stdout);
    double* U;
    double* V;
    double* Ulp;
    double* Vlp;
    U    = (double*)malloc( TIME_len * sizeof(double));
    V    = (double*)malloc( TIME_len * sizeof(double));
    Ulp  = (double*)malloc( TIME_len * sizeof(double));
    Vlp  = (double*)malloc( TIME_len * sizeof(double));

    // Init the FES prediction
    FES u_tide;
    FES v_tide;
    
    (void)fprintf(stdout,"Initting libfes\n");
    fflush(stdout);
    
    if (fes_new(&u_tide, FES_TIDE, FES_IO, INI_U)) {
      printf("fes error : %s\n", fes_error(u_tide));
      goto error;
    }
    if (fes_new(&v_tide, FES_TIDE, FES_IO, INI_V)) {
      printf("fes error : %s\n", fes_error(v_tide));
      goto error;
    }
    
    int k;
    double time;
    
    (void)fprintf(stdout,"Entering the loop\n");
    fflush(stdout);
    
    // separate the U and V loops in case that helps caching:
    for (k = 0; k < TIME_len; k++) {
      
      if ( k % 1000 == 0 ) {printf("Working on k = %d\n", k);}
      
      // For FES:
      // Time in CNES Julian days, defined as Modified Julian Day minus 33282.
      // Thus CNES 0 is at midnight between the 31 December and 01 January 1950
      // AD Gregorian.
      //
      // From the drifter file:
      // TIME:units = "hours since 1979-01-01 00:00:00" ;
      //
      // ymd2jd(1950,1,1) = 2433283
      // ymd2jd(1979,1,1) = 2443875
      // ==> add 43875-33283 = 10592 days to the drifter time to obtain FES
      //     time.
      
      time = TIME[k]/24.0 + 10592.0;
      
      //      printf("  %f %f %f \n", LAT[k], LON[k], time);
      
      if (isnan(time) | isinf(time)) {
	U[k] = time;
	Ulp[k] = time;
	continue;
      }
      
      // Compute ocean tide currents:
      if (fes_core(u_tide, LAT[k], LON[k], time, &U[k], &Ulp[k])) {
	// If the current point is undefined (i.e. the point is on land), the
	// tide is not defined.
	if (fes_errno(u_tide) == FES_NO_DATA)
	  continue;
	else {
	  fprintf(stderr, "%s\n", fes_error(u_tide));
	  goto error;
	}
      }
      
      // COMBINE THE PREDICTIONS AND CONVERT TO m/s
      U[k] = (U[k] + Ulp[k])*1.0e-2;
    }
    
    for (k = 0; k < TIME_len; k++) {
      
      if ( k % 1000 == 0 ) {printf("Working on k = %d\n", k);}
      
      time = TIME[k]/24.0 + 10592.0;
      
      //      printf("  %f %f %f \n", LAT[k], LON[k], time);
      
      if (isnan(time) | isinf(time)) {
	V[k] = time;
	Vlp[k] = time;
	continue;
      }
      
      if (fes_core(v_tide, LAT[k], LON[k], time, &V[k], &Vlp[k])) {
	// If the current point is undefined (i.e. the point is on land), the
	// tide is not defined.
	if (fes_errno(v_tide) == FES_NO_DATA)
	  continue;
	else {
	  fprintf(stderr, "%s\n", fes_error(v_tide));
	  goto error;
	}
      }
      
      // COMBINE THE PREDICTIONS AND CONVERT TO m/s
      V[k] = (V[k] + Vlp[k])*1.0e-2;
    }
    
    goto finish;
    
 error:
    (void)fprintf(stdout,"Error at k = %d \n", k);
    fflush(stdout);
    exit(99);
    
 finish:
    // Release the memory used by the FES handlers.
    fes_delete(u_tide);
    fes_delete(v_tide);

    // Write the results:
    /* enter define mode */
    stat = nc_create("driftertraj_1.04c_xFESpred.nc", NC_CLOBBER|NC_64BIT_OFFSET, &ncid);
    check_err(stat,__LINE__,__FILE__);

    /* define dimensions */
    stat = nc_def_dim(ncid, "TIME", TIME_len, &TIME_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */
    
    TIME_dims[0] = TIME_dim;
    stat = nc_def_var(ncid, "TIME", NC_DOUBLE, RANK_TIME, TIME_dims, &TIME_id);
    check_err(stat,__LINE__,__FILE__);

    LON_dims[0] = TIME_dim;
    stat = nc_def_var(ncid, "LON", NC_DOUBLE, RANK_LON, LON_dims, &LON_id);
    check_err(stat,__LINE__,__FILE__);
            
    LAT_dims[0] = TIME_dim;
    stat = nc_def_var(ncid, "LAT", NC_DOUBLE, RANK_LAT, LAT_dims, &LAT_id);
    check_err(stat,__LINE__,__FILE__);

    U_dims[0] = TIME_dim;
    stat = nc_def_var(ncid, "U", NC_DOUBLE, RANK_U, U_dims, &U_id);
    check_err(stat,__LINE__,__FILE__);
    
    V_dims[0] = TIME_dim;
    stat = nc_def_var(ncid, "V", NC_DOUBLE, RANK_V, V_dims, &V_id);
    check_err(stat,__LINE__,__FILE__);

    /* assign global attributes */
    {
    stat = nc_put_att_text(ncid, NC_GLOBAL, "note", 184,
			   "FES2014 predicted tidal currents, based on the CNES libfes library. The current predictions include all available constituents, including the long-period tides and the inferred tides.");
    check_err(stat,__LINE__,__FILE__);
    }
    {
    stat = nc_put_att_text(ncid, NC_GLOBAL, "creator", 30, "Edward.D.Zaron@oregonstate.edu");
    check_err(stat,__LINE__,__FILE__);
    }
    {
    stat = nc_put_att_text(ncid, NC_GLOBAL, "date", 10, "2022-03-10");
    check_err(stat,__LINE__,__FILE__);
    }
    {
    stat = nc_put_att_text(ncid, NC_GLOBAL, "script", 47, "namako:/mnt/Data1/ezaron/Drifters/v1.04c/code.c");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, TIME_id, "standard_name", 16, "CNES Julian days");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, TIME_id, "units", 30, "days since 1950-01-01 00:00:00");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, LON_id, "standard_name", 9, "longitude");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, LON_id, "units", 12, "degrees_east");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, LAT_id, "standard_name", 8, "latitude");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, LAT_id, "units", 13, "degrees_north");
    check_err(stat,__LINE__,__FILE__);
    }

        {
    stat = nc_put_att_text(ncid, U_id, "standard_name", 27, "eastward_sea_water_velocity");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, U_id, "long_name", 27, "zonal component of velocity");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, U_id, "units", 6, "m s^-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, V_id, "standard_name", 28, "northward_sea_water_velocity");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, V_id, "long_name", 32, "meridional component of velocity");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(ncid, V_id, "units", 6, "m s^-1");
    check_err(stat,__LINE__,__FILE__);
    }

    /* leave define mode */
    stat = nc_enddef (ncid);
    check_err(stat,__LINE__,__FILE__);

    
    /* assign variable data */
    stat = nc_put_var_double(ncid, LAT_id, &LAT[0]);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_double(ncid, LON_id, &LON[0]);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_double(ncid, TIME_id, &TIME[0]);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_double(ncid, U_id, &U[0]);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_put_var_double(ncid, V_id, &V[0]);
    check_err(stat,__LINE__,__FILE__);
    

    // close the file
    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);

    (void)fprintf(stdout,"COMPLETED WITH NO ERRORS \n");
    fflush(stdout);

    return 0;
}
