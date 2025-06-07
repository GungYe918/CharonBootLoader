#pragma once
#include <Uefi.h>
#include "../Boot/BootInfo.h"
#define HASH_SIZE 32

extern CONST UINT8 kPasswordHash[HASH_SIZE];

VOID RegisterHash(BootInfo* Info, CONST UINT8 *PwdHash, int HashSize);