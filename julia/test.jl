using Printf

# Need to build the fes code as a shared library with -shared and -fPIC:
#
# mkdir build
# cd build
# cmake .. -DCMAKE_INSTALL_PREFIX=/home/ezaron/opt -DBUILD_SHARED_LIBS=true
# make
# make install
#
# ----> the library is installed at libfes.so.

# Copied from fes.h:
FES_IO   = Cint(0) # Int64 or UInt ?
FES_TIDE = Cint(0)
INI_U    = "../data/fes2014/eastward_velocity.ini"
INI_V    = "../data/fes2014/northward_velocity.ini"

# typdef void* FES; --> u_tide is ptr to Cvoid
# The signatures of the function calls:
#  int fes_new(FES* handle,
#              const fes_enum_tide_type tide,
#              const fes_enum_access mode,
#              const char* const path);
#  void fes_delete(FES handle);
#  int fes_core(FES handle,
#               const double lat,
#               const double lon,
#               const double time,
#               double* h,              <------- set to NaN if no data available
#               double* h_long_period);

u_tide = Ref{Ptr{Cvoid}}()
ierr=ccall((:fes_new,"/home/ezaron/opt/lib/libfes.so"),Cint,
           (Ref{Ptr{Cvoid}},Cint,Cint,Cstring),
           u_tide,FES_TIDE,FES_IO,INI_U)
if (ierr > 0)
    println("Error code from fes_new = ",ierr)
end

v_tide = Ref{Ptr{Cvoid}}()
ierr=ccall((:fes_new,"/home/ezaron/opt/lib/libfes.so"),Cint,
           (Ptr{Ptr{Cvoid}},Cint,Cint,Cstring),
           v_tide,FES_TIDE,FES_IO,INI_V)
if (ierr > 0)
    println("Error code from fes_new = ",ierr)
end

lon=Cdouble(-7.688)
lat=Cdouble(59.195)
tau=Cdouble(12053.0)
utide=Cdouble(0.0)
vtide=Cdouble(0.0)
ulp=Cdouble(0.0)
vlp=Cdouble(0.0)

for hour=collect(0:23)
    ierr=ccall((:fes_core,"/home/ezaron/opt/lib/libfes.so"),Cint,
               (Ref{Ptr{Cvoid}},Cdouble,Cdouble,Cdouble,Ref{Cdouble},Ref{Cdouble}),
               u_tide,lat,lon,tau,Ref{Cdouble}(utide),Ref{Cdouble}(ulp))
    ierr=ccall((:fes_core,"/home/ezaron/opt/lib/libfes.so"),Cint,
               (Ptr{Ptr{Cvoid}},Cdouble,Cdouble,Cdouble,Ptr{Cdouble},Ptr{Cdouble}),
               v_tide,lat,lon,tau,Ref(vtide),Ref(vlp))
    println(@sprintf("%12.5f %5d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",
                     tau,hour,lat,lon,utide,ulp,vtide,vlp))
    global tau = tau + Cdouble(1/24)
end
