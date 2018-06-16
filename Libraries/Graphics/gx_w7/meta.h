typedef struct _fxdpnt     /* Fixed point math type          */
   {short    fr;             /* fractional part              */
    short    wh;             /* whole part                   */
   }  fxdpnt;

typedef struct _ofswid    /* Offset/Width Table Entry              */
  { unsigned char  wid;      /* character width                    */
    signed   char  ofs;      /* character offset                   */
  } ofswid;

typedef struct  _fontRcd     /* MetaWINDOW FONT DATA HEADER RECORD */
  {
  byte           fontVer;       /* Font format version number (=2)    */
  byte           fontRev;       /* Font style revision designation    */
  byte           fontNameLen;   /* Font baseName length (0-15)        */
  char           fontBaseName[16]; /* Font Base Name                  */
  char           fontSuffix1[10];  /* Font suffix 1                   */
  char           fontSuffix2[10];  /* Font suffix 2                   */
  char           fontSuffix3[10];  /* Font suffix 3                   */
  char           fontFacing;    /* Synthesized font facing flags      */
  char           fontSign[8];   /* 'METAFONT' signature               */

  byte           fontWeight;    /* Font weight                        */
  byte           fontCoding;    /* Character set encoding             */
  long           fontSize;      /* Buffer size needed to hold font    */
  word           fontMax;       /* Maximum character code             */
  word           fontMin;       /* Minimum character code             */
  short          fontPtSize;    /* Point Size                         */
  short          fontfamily;    /* Font family                        */
  short          fontStyle;     /* Device style                       */
  short          fontFlags;     /* Font flags                         */
  long           fontColor[2];  /* Font Character & bkground colors   */

  byte           minChar;       /* Minimum ASCII character code       */
  byte           maxChar;       /* Maximum ASCII character code       */
  short          chWidth;       /* Fixed space character width        */
  short          chHeight;      /* Character height                   */
  short          chKern;        /* Fixed space character offset       */
  short          ascent;        /* Ascent                             */
  short          descent;       /* Descent                            */
  short          lnSpacing;     /* Vertical spacing between baselines */
  word           chBad;         /* Char to show for undefined codes   */
  short          chCenter[2];   /* Icon or Marker center (X,Y)        */
  short          chAngle;       /* Italicise slant angle              */
  short          chUnder;       /* recommended txUnder setting        */
  short          chScore;       /* recommended txScore setting        */
  long           locTbl;        /* offset to locTable                 */
  long           ofwdTbl;       /* offset to ofwdTable                */
  long           kernTbl;       /* offset to kernTable                */
  long           sizeTbl;       /* offset to size/rotation tbl(strk)  */
  long           grafMapTbl;    /* offset to font grafMap structure   */
  long           rowTbl;        /* offset to grafMap rowTable(s)      */
  long           fontTbl;       /* offset to font bitImage/nodes tbl  */
  long           fontNotice;    /* offset to trademk/cpyright notice  */
  long           fontSupplier;  /* offset to name of font supplier    */
  long           fontAuthor;    /* offset to name of font author      */
  long           fontInfo;      /* offset to miscellaneous font info  */
  long           fontDate;      /* offset to font creation date       */
  fxdpnt         fontSpacing;   /* Vertical spacing between baselines */
  fxdpnt         fontLowHgt;    /* baseline to top of lowercase 'x'   */
  fxdpnt         fontCapHgt;    /* baseline to top of capital 'H'     */
  fxdpnt         fontAscent;    /* baseline to top of lowercase 'd'   */
  fxdpnt         fontDescent;   /* baseline to bottom of lowercase 'p'*/
  fxdpnt         fontMaxWid;    /* maximum character width            */
  fxdpnt         fontAvgWid;    /* average character width            */
  fxdpnt         fontEmWid;     /* Em space width                     */
  short          fontRsvd;      /*   (reserved for future use)        */
  long           offwidTbl[16]; /* offset to facing off/wid tables    */
/*short          fontData[];*/  /* font data/image tables (variable)  */
  }  fontRcd;

typedef struct _grafMap       /* "grafMap" Data Structure      */
 {
  short        devClass;      /* Device class                  */
  short        devTech;       /* Device technology             */
  long         devMode;       /* Device ID                     */
  short        pixbytes;      /* bytes per scan line           */
  word         pixWidth;      /* Pixels horizontal             */
  word         pixHeight;     /* Pixels vertical               */
  short        pixResX;       /* Pixels per inch horzontally   */
  short        pixResY;       /* Pixels per inch vertically    */
  short        pixBits;       /* Color bits per pixel          */
  short        pixPlanes;     /* Color planes per pixel        */
  short        mapFlags;      /* grafMap flags                 */
  byte         **mapTable[32];/* Pointers to rowTable(s)       */
  short        mapLock;       /* busy semaphore                */ 
  short        mapNextSel;    /* OS next segment cookie        */ 
  long         mapWinType;    /* Bank window type (0-3)        */
  long         mapWinOffset;  /* Offset to 2nd bank window     */
  long         mapWinYmin[2]; /* Current bank(s) min Y value   */
  long         mapWinYmax[2]; /* Current bank(s) max Y value   */
  long         mapWinPlane;   /* Current bank plane            */
  long         mapWinScans;   /* Scan lines per bank           */
  long         mapHandle;     /* Handle to access device       */
  void      ( *mapBankMgr)(); /* Ptr to bank manager function  */
  void      ( *mapPlaneMgr)();/* Ptr to plane manager function */
  void      ( *mapAltMgr)();  /* Ptr to alt manager function   */
  void      ( *devMgr)();     /* Ptr to device manager list    */       
  long         prFill;        /*  primitive vector for fills              */
  long         prBlitSS;      /*  primitive vector for self-self blits    */
  long         prBlit1S;      /*  primitive vector for mono-self blits    */
  long         prBlitMS;      /*  primitive vector for mem-self  blits    */
  long         prBlitSM;      /*  primitive vector for self-mem  blits    */
  long         prRdImg;       /*  primitive vector for read image         */
  long         prWrImg;       /*  primitive vector for write image        */
  long         prLine;        /*  primitive vector for thin lines         */
  long         prSetPx;       /*  primitive vector for set pixel          */
  long         prGetPx;       /*  primitive vector for get pixel          */
  long         cbSyncFunc;    /*  call back to resync function            */
  long         cbPostErr;     /*  call back to post an error              */
}  grafMap;
