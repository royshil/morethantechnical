// VirtualSurgeon_CLI.cpp : main project file.

#include "stdafx.h"

using namespace System;

int main(array<System::String ^> ^args)
{
    Console::WriteLine(L"Hello World");

	VirtualSurgeon_CSWrapper::VirtualSurgeonWrapper w;
	w.Initialize(args[0]);

    return 0;
}
