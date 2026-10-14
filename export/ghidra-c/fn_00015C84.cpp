// Ghidra headless export
// Entry: 0x00015C84
// Name: update_entity_heading_from_linked_target
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00015C58 is_entity_target_within_range


int update_entity_heading_from_linked_target(void)

{
  int iVar1;
  undefined2 unaff_D2w;
  int in_A0;
  int extraout_A0;
  bool bVar2;
  
  iVar1 = *(int *)(in_A0 + 0xa2);
  bVar2 = false;
  if (iVar1 != 0) {
    iVar1 = is_entity_target_within_range();
    if (bVar2) {
      *(undefined2 *)(extraout_A0 + 0x34) = unaff_D2w;
      return iVar1;
    }
  }
  return iVar1;
}


