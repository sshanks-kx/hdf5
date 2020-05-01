#include "hdf5_utils.h"

#include <stdlib.h>
#include "k.h"
#include "hdf5.h"
#include "kdb_utils.h"

EXP K hdf5createFile(K fname){
  if(!kdbCheckType("[Cs]", fname))
    return KNL;
  hid_t file;
  char *filename;
  filename = kdbGetString(fname);
  file = H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
  free(filename);
  if(file < 0)
    return krr((S)"error creating file");
  H5Fclose(file);
  return KNL;
}

EXP K hdf5createGroup(K fname, K gname){
  if(!kdbCheckType("[Cs][Cs]", fname, gname))
    return KNL;
  hid_t file, group;
  hid_t gcpl; // group creation property list
  char *filename, *groupnames;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  free(filename);
  if(file < 0)
    return krr((S)"error opening file");
  groupnames = kdbGetString(gname);
  gcpl = H5Pcreate(H5P_LINK_CREATE);
  H5Pset_create_intermediate_group(gcpl, 1); // create intermediate groups
  group = H5Gcreate(file, groupnames, gcpl, H5P_DEFAULT, H5P_DEFAULT);
  free(groupnames);
  H5Pclose(gcpl);
  H5Fclose(file);
  if(group < 0)
    return krr((S)"error creating group");
  H5Gclose(group);
  return KNL;
}

EXP K hdf5createDataset(K fname, K dname, K kdims, K ktype){
  if(!kdbCheckType("[Cs][Cs]Ic", fname, dname, kdims, ktype))
    return KNL;
  hid_t file;
  ktypegroup_t dtype;
  char *filename, *dataname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  free(filename);
  if(file < 0)
    return krr((S)"error opening file");
  dataname = kdbGetString(dname);
  dtype = getKTypeGroup(ktype->g);
  if(dtype == NUMERIC)
    createNumericDataset(file, dataname, kdims, ktype);
  else if(dtype == STRING)
    createStringDataset(file, dataname, kdims);
  free(dataname);
  H5Fclose(file);
  return KNL;
}

EXP K hdf5createAttr(K fname, K dname, K aname, K kdims, K ktype){
  if(!kdbCheckType("[Cs][Cs][Cs]Ic", fname, dname, aname, kdims, ktype))
    return KNL;
  hid_t file, data;
  ktypegroup_t dtype;
  char *filename, *dataname, *attrname;
  filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  free(filename);
  if(file < 0)
    return krr((S)"error opening file");
  dataname = kdbGetString(dname);
  data = H5Oopen(file, dataname, H5P_DEFAULT);
  free(dataname);
  H5Fclose(file);
  if(data < 0)
    return krr((S)"error opening dataset/group");
  attrname = kdbGetString(aname);
  dtype = getKTypeGroup(ktype->g);
  if(dtype == NUMERIC)
    createNumericAttribute(data, attrname, kdims, ktype);
  else if(dtype == STRING)
    createStringAttribute(data, attrname, kdims);
  free(attrname);
  H5Oclose(data);
  return KNL;
}
