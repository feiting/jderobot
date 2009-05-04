%module interface

%{
#include <interface.h>
%}


typedef struct{
  char interface_name[MAX_NAME];
  %extend{
    Interface(JDESchema* const owner,
	      const char* interface_name,
	      const int implemented = 0);	
    void run();
    void stop();
  }
} Interface;
