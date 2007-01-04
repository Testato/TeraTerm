/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTFILE.DLL, Kermit protocol */

#ifdef __cplusplus
extern "C" {
#endif

/* prototypes */
void KmtInit
  (PFileVar fv, PKmtVar kv, PComVar cv, PTTSet ts);
void KmtTimeOutProc(PFileVar fv, PKmtVar kv, PComVar cv);
BOOL KmtReadPacket
#ifdef I18N
  (PFileVar fv,  PKmtVar kv, PComVar cv, PTTSet);
#else
  (PFileVar fv,  PKmtVar kv, PComVar cv);
#endif
void KmtCancel
  (PFileVar fv, PKmtVar kv, PComVar cv);

#ifdef __cplusplus
}
#endif
