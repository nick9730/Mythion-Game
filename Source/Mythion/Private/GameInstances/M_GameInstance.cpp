// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstances/M_GameInstance.h"

UM_GameInstance::UM_GameInstance()
{

	
}

void UM_GameInstance::Logout()
{
	AuthToken.Empty();
	LoggedInUsername.Empty();
	bIsLoggedIn = false;

}
