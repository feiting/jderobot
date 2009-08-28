#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "encoders.h"

Encoders* new_Encoders(const char* interface_name,
		       JDESchema* const supplier){
  Encoders* e;
  
  assert(supplier!=0 && supplier->hierarchy!=0);
  e = (Encoders*)calloc(1,sizeof(Encoders));
  assert(e!=0);
  SUPER(e) = new_JDEInterface(interface_name,supplier);
  assert(SUPER(e) != 0);
  /*interface export*/
  if (JDEHierarchy_myexport(supplier->hierarchy,interface_name,"Encoders",e) == 0){
    delete_JDEInterface(SUPER(e));
    free(e);
    return 0;
  }
  /*backwards compatibility exports*/
  myexport(interface_name,"jde_robot",e->robot);
  myexport(interface_name,"clock",&(e->clock));
  myexport(interface_name,"id",supplier->id);
  myexport(interface_name,"run",supplier->run);
  myexport(interface_name,"stop",supplier->stop);
  return e;
}

void delete_Encoders(Encoders* const self){
  if (self==0)
    return;
  delete_JDEInterface(SUPER(self));
  free(self);/*FIXME: un-export symbols. references?*/
}

/*for old interfaces proxy->refers_to == 0*/
EncodersPrx* new_EncodersPrx(const char* interface_name,
			     JDESchema* const user){
  EncodersPrx* eprx;

  assert(user!=0 && user->hierarchy!=0);
  eprx = (EncodersPrx*)calloc(1,sizeof(EncodersPrx));
  assert(eprx!=0);
  PRX_REFERS_TO(eprx) = (Encoders*)JDEHierarchy_myimport(user->hierarchy,interface_name,"Encoders");
  if (PRX_REFERS_TO(eprx) == 0){ /*we are connecting with an old
				   encoders interface*/
    int *idp;
    JDESchema *supplier;

    idp = (int*)myimport(interface_name,"id");/*we have to get the
						supplier*/
    if (idp != 0){
      JDEInterface *i;

      supplier = get_schema(*idp);
      assert(supplier!=0);
      i = new_JDEInterface(interface_name,supplier);/*pointer stored with myimport*/
      assert(i!=0);
    }else{/*no interface found with this name*/
      free(eprx);
      return 0;
    }
  }
  SUPER(eprx) = new_JDEInterfacePrx(interface_name,user);
  assert(SUPER(eprx)!=0);
  return eprx;
}

void delete_EncodersPrx(EncodersPrx* const self){
  JDEInterface *e = 0;
  if (self==0)
    return;
  
  if (PRX_REFERS_TO(self)==0)/*free interface allocated for backwards compatibility*/
    e = PRX_REFERS_TO(SUPER(self));
  delete_JDEInterfacePrx(SUPER(self));
  delete_JDEInterface(e);
  free(self);
}

float* EncodersPrx_robot_get(const EncodersPrx* self){
  float* robot = 0;

  assert(self!=0);
  if (PRX_REFERS_TO(self))
    robot=PRX_REFERS_TO(self)->robot;
  else/*backwards compatibility*/
    robot=(float *)myimport(INTERFACEPRX_NAME(self),"jde_robot");
  assert(robot!=0);
  return robot;
}

float EncodersPrx_x_get(const EncodersPrx* self){
  return EncodersPrx_robot_get(self)[ROBOT_X];
}

float EncodersPrx_y_get(const EncodersPrx* self){
  return EncodersPrx_robot_get(self)[ROBOT_Y];
}

float EncodersPrx_theta_get(const EncodersPrx* self){
  return EncodersPrx_robot_get(self)[ROBOT_THETA];
}

float EncodersPrx_cos_get(const EncodersPrx* self){
  return EncodersPrx_robot_get(self)[ROBOT_COS];
}

float EncodersPrx_sin_get(const EncodersPrx* self){
  return EncodersPrx_robot_get(self)[ROBOT_SIN];
}

unsigned long int EncodersPrx_clock_get(const EncodersPrx* self){
  unsigned long int* clockp = 0;

  assert(self!=0);
  if (PRX_REFERS_TO(self))
    clockp=&(PRX_REFERS_TO(self)->clock);
  else/*backwards compatibility*/
    clockp=(unsigned long int *)myimport(INTERFACEPRX_NAME(self),"clock");
  assert(clockp!=0);
  return *clockp;
} 

void EncodersPrx_robot_set(EncodersPrx* const self, float* new_robot){
  float *robot;

  assert(self!=0);
  robot = EncodersPrx_robot_get(self);
  memmove(robot,new_robot,sizeof(float)*ROBOT_NELEM);
}

void EncodersPrx_clock_set(EncodersPrx* const self, unsigned long int new_clock){
  unsigned long int* clockp = 0;

  assert(self!=0);
  if (PRX_REFERS_TO(self))
    clockp=&(PRX_REFERS_TO(self)->clock);
  else/*backwards compatibility*/
    clockp=(unsigned long int *)myimport(INTERFACEPRX_NAME(self),"clock");
  assert(clockp!=0);
  *clockp = new_clock;
}

