#pragma once

#define FREEIF(p) do{if(p){free(p);p=NULL;}}while(0);