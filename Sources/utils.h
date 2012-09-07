#ifndef _UTILS_H
#define _UTILS_H

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned long   u32;
typedef char            s8;
typedef short           s16;
typedef long            s32;

#ifdef WIN32
  #define _USE_MATH_DEFINES
  #include <stdio.h>
  #include <string.h>
  #include <cmath>
  #include <assert.h>
  #include <windows.h>
  #include <string>
  #include <hash_map>
  #include <queue>

  #define fopen_s(a,b,c) _fopen_s_s(a,b,c)
#endif

#ifdef LINUX
  #include <cstring>
  #include <iostream>
  #include <cmath>
  #include <assert.h>
  #include <queue>
  #include <tr1/unordered_map>
  #include <tr1/memory>

  namespace std { using namespace __gnu_cxx; }

//  #define strcat_s(a,b,c)   strcat(a,c)
//  #define strcpy_s(a,b,c)   strcpy(a,c)
//  #define _strcasecmp(a,b)     strcasecmp(a,b)
//  #define strcasecmp(a,b)     wcscasecmp(a,b)  // Just as reminder : strcmp can also be used
  #define errno_t int
  #define max(x,y) ((x)>=(y)?(x):(y))
  #define min(x,y) ((x)<=(y)?(x):(y))
  extern errno_t fopen_s(FILE ** pFile, const char * sFilename, const char * sMode);
  #define GLUT_WHEEL_UP   3 // freeglut uses 3 and 4 as wheel buttons
  #define GLUT_WHEEL_DOWN 4
#endif

typedef std::tr1::unordered_map<std::string, long> long_hash;
typedef std::tr1::unordered_map<std::string, std::string> wstr_hash;
typedef std::tr1::unordered_map<char, char> wch_hash;

#ifndef MAX_PATH
  #define MAX_PATH    512
#endif
#define DATA_PATH             "data/"
#define GAME_TEXTURES_PATH    "data/textures/"
#define GAME_MUSICS_PATH      "data/musics/"
#define GAME_SOUNDS_PATH      "data/sounds/"
#define MAPS_PATH             "data/maps/"
#define SHADERS_PATH          "data/shaders/"
#define PROFILES_PATH         "players/"
#define EDITIONS_PATH         "editions/"
#define SAVES_PATH            "saves/"
#define FREE(m)               { if (m != NULL) { delete m; m = NULL; } }
#define FREEARR(m)            { if (m != NULL) { delete[] m; m = NULL; } }
#define EPSILON               0.000001f
#define PI                    3.141592f

enum Cardinal9
{
  c9_NW = 1,
  c9_N,
  c9_NE,
  c9_W,
  c9_C,
  c9_E,
  c9_SW,
  c9_S,
  c9_SE
};

#define XYZ_EMPTY_CONSTRUCTOR(T)  T() { x = 0; y = 0; z = 0; };
#define XYZ_CONSTRUCTOR(T, T2)    T(T2 x, T2 y, double z = 0.0f) { this->x = x; this->y = y; this->z = z; };
#define XYZ_OPDIF(T)              bool operator!= (T other) { return (x != other.x || y != other.y || z != other.z); };
#define XYZ_OPEQ(T)               bool operator== (T other) { return (x == other.x && y == other.y && z == other.z); };
#define XYZ_OPPLUS(T)             T operator+ (T other) { return T(x + other.x, y + other.y, z + other.z); };
#define XYZ_OPPLUSEQ(T)           void operator+= (T other) { x += other.x; y += other.y; z += other.z; };
#define XYZ_OPMINUS(T)            T operator- (T other) { return T(x - other.x, y - other.y, z - other.z); };
#define XYZ_OPMINUSEQ(T)          void operator-= (T other) { x -= other.x; y -= other.y; z -= other.z; };
#define XYZ_OPMULT(T, T2)         T operator* (T2 k) { return T(x * k, y * k, z * (double)k); };
#define XYZ_OPDIV(T, T2)          T operator/ (T2 k) { return T(x / k, y / k, z / (double)k); };
#define XYZ_DECLAREVAR(T)         T x, y; double z;

#define XY_EMPTY_CONSTRUCTOR(T)   T() { x = 0; y = 0; };
#define XY_CONSTRUCTOR(T, T2)     T(T2 x, T2 y) { this->x = x; this->y = y; };
#define XY_OPDIF(T)               bool operator!= (T other) { return (x != other.x || y != other.y); };
#define XY_OPEQ(T)                bool operator== (T other) { return (x == other.x && y == other.y); };
#define XY_OPPLUS(T)              T operator+ (T other) { return T(x + other.x, y + other.y); };
#define XY_OPPLUSEQ(T)            void operator+= (T other) { x += other.x; y += other.y; };
#define XY_OPMINUS(T)             T operator- (T other) { return T(x - other.x, y - other.y); };
#define XY_OPMINUSEQ(T)           void operator-= (T other) { x -= other.x; y -= other.y; };
#define XY_OPMULT(T, T2)          T operator* (T2 k) { return T(x * k, y * k); };
#define XY_OPDIV(T, T2)           T operator/ (T2 k) { return T(x / k, y / k); };
#define XY_DECLAREVAR(T)          T x, y;

class Coords {};
class CoordsScreen : public Coords
{
public:
  XYZ_EMPTY_CONSTRUCTOR(CoordsScreen)
  XYZ_CONSTRUCTOR(CoordsScreen, int)
  XYZ_OPDIF(CoordsScreen)
  XYZ_OPEQ(CoordsScreen)
  XYZ_OPPLUS(CoordsScreen)
  XYZ_OPPLUSEQ(CoordsScreen)
  XYZ_OPMINUS(CoordsScreen)
  XYZ_OPMINUSEQ(CoordsScreen)
  XYZ_OPMULT(CoordsScreen, int)
  XYZ_OPDIV(CoordsScreen, int)
  XYZ_DECLAREVAR(int)
};
class CoordsMap : public Coords
{
public:
  XY_EMPTY_CONSTRUCTOR(CoordsMap)
  XY_CONSTRUCTOR(CoordsMap, int)
  XY_OPDIF(CoordsMap)
  XY_OPEQ(CoordsMap)
  XY_OPPLUS(CoordsMap)
  XY_OPPLUSEQ(CoordsMap)
  XY_OPMINUS(CoordsMap)
  XY_OPMINUSEQ(CoordsMap)
  XY_OPMULT(CoordsMap, int)
  XY_OPDIV(CoordsMap, int)
  XY_DECLAREVAR(int)
};
class Coords3D : public Coords
{
public:
  XYZ_EMPTY_CONSTRUCTOR(Coords3D)
  XYZ_CONSTRUCTOR(Coords3D, double)
  XYZ_OPDIF(Coords3D)
  XYZ_OPEQ(Coords3D)
  XYZ_OPPLUS(Coords3D)
  XYZ_OPPLUSEQ(Coords3D)
  XYZ_OPMINUS(Coords3D)
  XYZ_OPMINUSEQ(Coords3D)
  XYZ_OPMULT(Coords3D, double)
  XYZ_OPDIV(Coords3D, double)
  double getsize() {return sqrt(x*x+y*y+z*z); };
  double dot(Coords3D op2) { return ( x * op2.x + y * op2.y + z * op2.z); }; // Dot product
  void cross(Coords3D op1, Coords3D op2) {x=(op1.y*op2.z)-(op1.z*op2.y);y=(op1.z*op2.x)-(op1.x*op2.z);z=(op1.x*op2.y)-(op1.y*op2.x);}; // Cross product
  Coords3D getUnitVector() { double size=getsize(); if (size!=0) return (*this/size); else return Coords3D(); };
  XYZ_DECLAREVAR(double)
};

#define f3d   Coords3D

typedef struct struct_F_RGBA
{
  float r;
  float g;
  float b;
  float a;
  struct struct_F_RGBA operator+(struct struct_F_RGBA color) { struct struct_F_RGBA ret; ret.r = color.r + r; ret.g = color.g + g; ret.b = color.b + b; ret.a = color.a + a; return ret; }
  struct struct_F_RGBA operator-(struct struct_F_RGBA color) { struct struct_F_RGBA ret; ret.r = r - color.r; ret.g = g - color.g; ret.b = b - color.b; ret.a = a - color.a; return ret; }
  struct struct_F_RGBA operator*(float f) { struct struct_F_RGBA ret; ret.r = f * r; ret.g = f * g; ret.b = f * b; ret.a = f * a; return ret; }
  struct struct_F_RGBA operator/(float f) { struct struct_F_RGBA ret; ret.r = r / f; ret.g = g / f; ret.b = b / f; ret.a = a / f; return ret; }
} F_RGBA;

typedef struct struct_RELINK_PTR_DATA
{
  u8 type;
  u32 data1;
  u32 data2;
  u32 data3;
  u32 data4;
} RELINK_PTR_DATA;

extern F_RGBA                     rgba(float r, float g, float b, float a);
extern F_RGBA                     rgb(float r, float g, float b);
#define F_RGBA_NULL               rgba(-1, -1, -1, -1)
#define F_RGBA_ISNULL(c)          (c.r==-1&&c.g==-1&&c.b==-1&&c.a==-1)
#define F_RGBA_MULTIPLY(c1, c2)   (F_RGBA_ISNULL(c1) ? c2 : (F_RGBA_ISNULL(c2) ? c1 : rgba(c1.r*c2.r, c1.g*c2.g, c1.b*c2.b, c1.a*c2.a)))

extern void chop(char * str);
extern void add_long_to_wstr(long iVal, int precision, char * sDst, int * iDst);
extern void add_double_to_wstr(double fVal, int precision, char * sDst, int * iDst);
extern void wsafecpy(char * dst, unsigned int size, const char * src);
extern void wsafecat(char * dst, unsigned int size, const char * src);

// The following is defined in platform-specific cpp files
extern u32 getRandom(u32 max);  // retourne un nombre entre 0 et max-1
//extern size_t strtow(char * sDst, int sizemax, const char * sSrc);
//extern size_t wtostr(char * sDst, int sizemax, const char * sSrc);
extern bool md5folder(const char * sFolder, char * sDigest);
extern bool copyStringToClipboard(const char * wsource);
extern char * getStringFromClipboard(char * sBuffer, int iBufSize);
extern int getEditions(char ** sEditionsList, unsigned int iListSize, int iEditionNameSize);
extern int getSkills(char ** sSkillsList, unsigned int iListSize, int iSkillNameSize, const char * sEdition);
extern int getProfiles(char ** sProfilesList, unsigned int iListSize, int iProfileNameSize);
extern int getSavedGames(char ** sSavesList, unsigned int iListSize, int iSavesNameSize);
extern int getMaps(char ** sMapsList, unsigned int iListSize, int iMapNameSize);
extern int getAvailableDisplayModes(CoordsScreen * pResolution, int * pBpp, int iMaxEntries);

#endif
