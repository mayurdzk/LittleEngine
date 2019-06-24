#include "stdafx.h"
#include "GameLoop/GameLoop.h"
#include "LEGameCode/GameInit.h"

using namespace LittleEngine;

int main(int argc, char** argv)
{
	GameInit::CreateWorlds();
	return GameLoop::Run(argc, argv);
}
