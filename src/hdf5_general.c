#include <stdlib.h>
#include "k.h"
#include "hdf5.h"
#include "kdb_utils.h"
#include "hdf5_utils.h"

// initialize hdf5-kdb library
EXP K hdf5init(K UNUSED(dummy)){
  disableErr();
  return KNL;
}

EXP K hdf5version(K UNUSED(x)){
  unsigned int maj, min, rel;
  if(H5get_libversion(&maj, &min, &rel) < 0)
    return krr((S)"Error evaluating version of HDF5 C api");
  else
    return kdbCreateDict("Major", kj(maj), "Minor", kj(min), "Release", kj(rel));
}

EXP K hdf5gc(K UNUSED(x)){
  return ki(H5garbage_collect());
}

// Retrieve the shape of a dataspace
K getShape(hid_t space){
  hsize_t *dims;
  int rank = H5Sget_simple_extent_ndims(space);
  K kdims = ktn(KJ, rank);
  dims = calloc(rank, sizeof(hsize_t));
  H5Sget_simple_extent_dims(space, dims, NULL);
  for(int i = 0; i < rank; i++)
    kJ(kdims)[i] = dims[i];
  free(dims);
  return(kdims);
}

EXP K hdf5fileSize(K fname){
  if(!kdbCheckType("[Cs]", fname))
    return KNL;
  hsize_t fsize;
  hid_t file;
  char *filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  H5Fget_filesize(file, &fsize);
  free(filename);
  H5Fclose(file);
  return(kf(fsize / 1000000.0));
}

EXP K hdf5dataSize(K fname, K dname){
  if(!kdbCheckType("[Cs][Cs]", fname, dname))
    return KNL;
  hsize_t dsize;
  hid_t file, data;
  char *filename, *dataname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Dopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset could not be opened");
  }
  dsize = H5Dget_storage_size(data);
  free(filename);
  free(dataname);
  H5Fclose(file);
  H5Dclose(data);
  return(kf(dsize / 1000000.0));
}

EXP K hdf5getDataShape(K fname, K dname){
  if(!kdbCheckType("[Cs][Cs]", fname, dname))
    return KNL;
  K kdims;
  hid_t file, data, space;
  char *filename, *dataname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Dopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset could not be opened");
  }
  space = H5Dget_space(data);
  kdims = getShape(space);
  // clean up
  free(filename);
  free(dataname);
  H5Fclose(file);
  H5Dclose(data);
  H5Sclose(space);
  return kdims;
}

EXP K hdf5getDataPoints(K fname, K dname){
  if(!kdbCheckType("[Cs][Cs]", fname, dname))
    return KNL;
  J points;
  hid_t file, data, space;
  char *filename, *dataname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    H5Fclose(file);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Dopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset/group could not be opened");
  }
  space  = H5Dget_space(data);
  points = H5Sget_simple_extent_npoints(space); // number of datapoints
  // clean up
  free(filename);
  free(dataname);
  H5Fclose(file);
  H5Dclose(data);
  H5Sclose(space);
  return kj(points);
}

EXP K hdf5getAttrShape(K fname, K dname, K aname){
  if(!kdbCheckType("[Cs][Cs][Cs]", fname, dname, aname))
    return KNL;
  K kdims;
  hid_t file, data, attr, space;
  char *filename, *dataname, *attrname;
  filename = kdbGetString(fname);
  file  = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data  = H5Oopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset/group could not be opened");
  }
  attrname = kdbGetString(aname);
  attr  = H5Aopen(data, attrname, H5P_DEFAULT);
  if(attr < 0){
    free(attrname);
    free(dataname);
    free(filename);
    H5Oclose(data);
    H5Fclose(file);
    return krr((S)"attribute could not be opened");
  }
  space = H5Aget_space(attr);
  kdims = getShape(space);
  // clean up
  free(filename);
  free(dataname);
  free(attrname);
  H5Oclose(data);
  H5Aclose(attr);
  H5Fclose(file);
  H5Sclose(space);
  return kdims;
}

EXP K hdf5getAttrPoints(K fname, K dname, K aname){
  if(!kdbCheckType("[Cs][Cs][Cs]", fname, dname, aname))
    return KNL;
  hid_t file, data, attr, space;
  J points;
  char *filename, *dataname, *attrname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Oopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset/group could not be opened");
  }
  attrname = kdbGetString(aname);
  attr   = H5Aopen(data, attrname, H5P_DEFAULT);
  if(attr < 0){
    free(attrname);
    free(dataname);
    free(filename);
    H5Oclose(data);
    H5Fclose(file);
    return krr((S)"attribute could not be opened");
  }
  space  = H5Aget_space(attr);
  points = H5Sget_simple_extent_npoints(space);
  // clean up
  free(filename); 
  free(dataname);
  free(attrname);
  H5Oclose(data);
  H5Fclose(file);
  H5Aclose(attr);
  H5Sclose(space);
  return kj(points);
}

// Is the file being passed a hdf5 file
EXP K hdf5ishdf5(K fname){
  if(!kdbCheckType("[Cs]", fname))
    return KNL;
  htri_t filechk;
  char *filename = kdbGetString(fname);
  filechk = H5Fis_hdf5(filename);
  free(filename);
  return kb(filechk > 0 ? 1 : 0);
}

// Does the requested object (group/dataset) exist
EXP K hdf5isObject(K fname, K oname){
  if(!kdbCheckType("[Cs][Cs]", fname, oname))
    return KNL;
  htri_t oexists;
  hid_t file;
  char *filename, *objname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  objname = kdbGetString(oname);
  oexists = H5Oexists_by_name(file,objname, H5P_DEFAULT);
  // clean up
  free(filename);
  free(objname);
  H5Fclose(file);
  return kb(oexists > 0 ? 1 : 0);
}

// Does the requested attribute exist
EXP K hdf5isAttr(K fname, K dname, K aname){
  if(!kdbCheckType("[Cs][Cs][Cs]", fname, dname, aname))
    return KNL;
  htri_t aexists;
  hid_t file, data;
  char *filename, *dataname, *attrname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Oopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset/group could not be opened");
  }
  attrname = kdbGetString(aname);
  aexists = H5Aexists(data, attrname);
  // clean up
  free(filename);
  free(dataname);
  free(attrname);
  H5Oclose(data);
  H5Fclose(file);
  return kb(aexists > 0 ? 1 : 0);
}

// return type, ndims, dims
EXP K hdf5datasetInfo(K fname, K dname){
  if(!kdbCheckType("[Cs][Cs]", fname, dname))
    return KNL;
  int rank;
  hsize_t *dims;
  K kdims, kdtype;
  hid_t file, data, dtype, ntype, space;
  char *filename, *dataname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  dataname = kdbGetString(dname);
  data = H5Dopen(file, dataname, H5P_DEFAULT);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"dataset could not be opened");
  }
  space = H5Dget_space(data);
  rank = H5Sget_simple_extent_ndims(space); // number of dimensions
  dims = calloc(rank, sizeof(hsize_t));
  H5Sget_simple_extent_dims(space, dims, NULL);
  kdims = ktn(KI, rank);
  for(int i = 0; i < rank; ++i)
    kI(kdims)[i] = dims[i];
  dtype = H5Dget_type(data);
  ntype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
  if     (H5Tequal(ntype, HDF5SHORT))
    kdtype = ks((S)"short");
  else if(H5Tequal(ntype, HDF5INT))
    kdtype = ks((S)"integer");
  else if(H5Tequal(ntype, HDF5LONG))
    kdtype = ks((S)"long");
  else if(H5Tequal(ntype, HDF5REAL))
    kdtype = ks((S)"real");
  else if(H5Tequal(ntype, HDF5FLOAT))
    kdtype = ks((S)"float");
  else 
    kdtype = ks((S)"char");
  // clean up
  free(dims);
  free(filename);
  free(dataname);
  H5Dclose(data);
  H5Tclose(dtype);
  H5Tclose(ntype);
  H5Sclose(space);
  H5Fclose(file);
  return kdbCreateDict("type", kdtype, "ndims", ki(rank), "dims", kdims);
}

EXP K hdf5copyObject(K srcfile, K src_obj, K dstfile, K dst_obj){
  if(!kdbCheckType("[Cs][Cs][Cs][Cs]", srcfile, src_obj, dstfile, dst_obj))
    return KNL;
  hid_t src, dst;
  char *srcfilename, *dstfilename, *src_objname, *dst_objname;
  srcfilename = kdbGetString(srcfile);
  dstfilename = kdbGetString(dstfile);
  src = H5Fopen(srcfilename, H5F_ACC_RDWR, H5P_DEFAULT);
  dst = H5Fopen(dstfilename, H5F_ACC_RDWR, H5P_DEFAULT);
  if((src < 0) || (dst < 0)){
    free(srcfilename);
    free(dstfilename);
    H5Fclose(src);
    H5Fclose(dst);
    return krr((S)"opening files unsuccessful");
  }
  src_objname = kdbGetString(src_obj);
  dst_objname = kdbGetString(dst_obj);
  if(H5Ocopy(src, src_objname, dst, dst_objname, H5P_DEFAULT, H5P_DEFAULT)<0)
    krr((S)"copying object unsuccessful");
  // clean up
  free(srcfilename);
  free(dstfilename);
  free(src_objname);
  free(dst_objname);
  H5Fclose(src);
  H5Fclose(dst);
  return KNL;
}
