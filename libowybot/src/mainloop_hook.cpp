#include <Windows.h>
#include <grinder.hpp>
#include <mainloop_hook.hpp>
#include <stdio.h>

void
implant()
{
  grinder::grind_the_livin_shit_out_of_the_game();
}

__declspec(naked) void trampolinka()
{
  __asm {
		XOR EBX, EBX
		MOV DWORD PTR SS : [EBP - 0x4], EBX
		PUSHAD
		CALL implant
		POPAD
		RET
  }
}

void
mainloop_hook::install_hook()
{
  int engineBase = (int)LoadLibraryA("Engine.dll");
  printf("Engine base %X\n", engineBase);
  DWORD old, old2;
  VirtualProtect((void*)(engineBase + 0x2D544B), 5, PAGE_EXECUTE_READWRITE, &old);
  *(char*)(engineBase + 0x2D544B) = '\xE8';
  *(int*)(engineBase + 0x2D544B + 1) = (int)trampolinka - (engineBase + 0x2D544B) - 5;
  VirtualProtect((void*)(engineBase + 0x2D544B), 5, old, &old2);
  puts("Hello mainloop_hook");
}
