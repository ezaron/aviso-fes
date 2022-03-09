/* This file is part of FES library.

   FES is free software: you can redistribute it and/or modify
   it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   FES is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU LESSER GENERAL PUBLIC LICENSE for more details.

   You should have received a copy of the GNU LESSER GENERAL PUBLIC LICENSE
   along with FES.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>

#include "fes.h"

// Path to the configuration file and data used to test the library
// Change these settings to your liking.
#define INI_TIDE "../../data/fes2014/ocean_tide.ini"
#define INI_RADIAL "../../data/fes2014/load_tide.ini"
#define INI_U "../../data/fes2014/eastward_velocity.ini"
#define INI_V "../../data/fes2014/northward_velocity.ini"
// For the CNES example, the .ini files contain ${FES_DATA} and they also use
// a naming convention for the netcdf files which is not used in my downloaded
// set of FES data files. The .ini files above contain the correct names
// of the files and their path relative to ../data/fes2014. Therefore, the
// FES_DATA environment variable is not needed.

int main(void) {
  // The return code
  int rc = 0;
  // The hour of the estimate.
  int hour;
  // Latitude and longitude of the point where the ocean tide will be
  // evaluated.
  double lon = -7.688;
  double lat = 59.195;
  // Short tides (semi_diurnal and diurnal tides)
  double tide;
  double utide;
  double vtide;
  // Time in CNES Julian days, defined as Modified Julian Day minus 33282.
  // Thus CNES 0 is at midnight between the 31 December and 01 January 1950
  // AD Gregorian.
  double time;
  // Long period tides
  double lp;
  double ulp;
  double vlp;
  // Loading effects for short tide
  double load;
  // Loading effects for long period tides (is always equal to zero)
  double loadlp;
  // FES handlers
  FES short_tide;
  FES u_tide;
  FES v_tide;
  FES radial_tide = NULL;

  // Creating the FES handler to calculate the u_tide
  if (fes_new(&u_tide, FES_TIDE, FES_IO, INI_U)) {
    printf("fes error : %s\n", fes_error(u_tide));
    goto error;
  }
  if (fes_new(&u_tide, FES_TIDE, FES_IO, INI_V)) {
    printf("fes error : %s\n", fes_error(v_tide));
    goto error;
  }

  printf("%12s %5s %9s %9s %9s %9s %9s %9s\n",
	 "JulDay", "Hour", "Latitude","Longitude",
	 "uShort_tid", "uLP_tid", "vShort_tid", "vLP_tid");

  for (hour = 0, time = 12053; hour < 24; hour++, time += 1 / 24.0) {
    // Compute ocean tide currents:
    if (fes_core(u_tide, lat, lon, time, &utide, &ulp)) {
      // If the current point is undefined (i.e. the point is on land), the
      // tide is not defined.
      if (fes_errno(u_tide) == FES_NO_DATA)
        continue;
      else {
        fprintf(stderr, "%s\n", fes_error(u_tide));
        goto error;
      }
    }

    if (fes_core(v_tide, lat, lon, time, &vtide, &vlp)) {
      // If the current point is undefined (i.e. the point is on land), the
      // tide is not defined.
      if (fes_errno(v_tide) == FES_NO_DATA)
        continue;
      else {
        fprintf(stderr, "%s\n", fes_error(v_tide));
        goto error;
      }
    }

    // tide + lp        = pure tide (as seen by a tide gauge)
    // tide + lp + load = geocentric tide (as seen by a satellite)
    printf("%12.5f %5d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f\n",
	   time, hour,lat, lon,
	   utide, ulp, vtide, vlp);
  }

  goto finish;

error:
  rc = 1;

finish:
  // Release the memory used by the FES handlers.
  fes_delete(u_tide);
  fes_delete(v_tide);

  return rc;
}
