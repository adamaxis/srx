// Ghidra headless export
// Entry: 0x00004388
// Name: FUN_00004388
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000043B6 process_pending_entity_template_lists


void FUN_00004388(void)

{
  process_pending_entity_template_lists();
  if (DAT_00ffe186 == '\x02') {
    DAT_00ff049d = 6;
  }
  if (DAT_00ffe186 == '\x01') {
    DAT_00ffe177 = 6;
  }
  return;
}


