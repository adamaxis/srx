// Ghidra headless export
// Entry: 0x000105E6
// Name: FUN_000105e6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_title_text_source
//   0x00010660 FUN_00010660


void FUN_000105e6(void)

{
  char cVar1;
  bool bVar2;
  char cVar3;
  
  cVar1 = DAT_00fff096 < 0;
  bVar2 = DAT_00fff096 == 0;
  cVar3 = '\0';
  FUN_00010660();
  if (bVar2 || cVar3 != cVar1) {
    render_title_text_source();
  }
  else {
    write_vdp_grid();
  }
  return;
}


