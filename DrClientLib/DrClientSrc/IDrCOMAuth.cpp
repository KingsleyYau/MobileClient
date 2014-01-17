/*
 * File         : IDrCOMAuth.cpp
 * Date         : 2013-05-02
 * Author       : FGX
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM auth interface
 */

#include "IDrCOMAuth.h"
#include "DrCOMAuth.h"

IDrCOMAuth* IDrCOMAuth::CreateDrCOMAuth()
{
	return new DrCOMAuth();
}
	
void IDrCOMAuth::ReleaseDrCOMAuth(IDrCOMAuth* object)
{
	delete object;
}

bool IDrCOMAuth::InitSocketEnvironment()
{
	return DrCOMAuth::InitSocketEnvironment();
}

void IDrCOMAuth::ReleaseSocket()
{
	return DrCOMAuth::ReleaseSocketEnvironment();
}
