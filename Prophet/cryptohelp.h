#pragma once
 
#ifndef __PROPHET_CRYPTOHELP_H__
#define __PROPHET_CRYPTOHELP_H__
 
#include "prophet.h"

void RC4_KeySchedule(pbyte S, const pbyte key, int n);
bool RC4_IsValidSbox(const pbyte S, const pbyte key, int keylen);
void RC4_Crypt(pbyte S, const pbyte src, pbyte dest, int n);
bool RC4_IsValidCrypt(pbyte S, const pbyte pt, const pbyte ct, int n);
 
#endif // __PROPHET_CRYPTOHELP_H__