#ifndef _SYSTEMHEADERS_H
#define _SYSTEMHEADERS_H

#ifdef WIN32
  // GLUT
  #include <stdlib.h>
  #include <errno.h>
  #include <windows.h>
  #include <mmsystem.h>
  #ifndef __WIN32__
    #define __WIN32__
  #endif

  #include "GL\glew.h"
  #include "GL\glut.h"

  #include <png.h>
#endif

#ifdef LINUX
  // GLUT
  #include <stdlib.h>
  #include <errno.h>

  #include <GL/glew.h>
  #include <GL/glut.h>
  #include <png.h>
#endif

#ifdef __APPLE__
  #include <GLUT/glut.h>
#endif

#ifdef NDS
  // Nintendo DS
  //#include <stdarg.h>
  //#include "NDS\ndsutils.h"
  //#include "NDS\InputEngine.h"
  //#include "NDS\SoundEngine.h"

  #define MEMCPY(x,y,z)   MI_CpuCopyFast(y, x, z)
  #define STRCPY(x,y,z)   STD_CopyString(x, z)
  #define STRCMP(x,y)     STD_CompareString(x, y)
  #define STRLEN(x)       STD_GetStringLength(x)
#endif

#endif
