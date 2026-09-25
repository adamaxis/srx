// Ghidra headless export
// Entry: 0x00001872
// Name: service_display_frame
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000EC0 upload_startup_palette
//   0x00000EE6 service_pending_vdp_upload
//   0x00000F1C write_transfer_header
//   0x00000F3C write_fixed_transfer_pair
//   0x00002C92 render_normal_field_viewport
//   0x00002CB2 render_alternate_field_viewport
//   0x00006148 flush_primary_queued_transfers
//   0x000061F4 flush_secondary_queued_transfers
//   0x0005672C convert_intro_routine_0x0005672c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void service_display_frame(void)

{
  byte bVar1;
  
  DAT_00fff9ec = DAT_00fff9ec + 1;
  DAT_00fff9f0 = 0;
  bVar1 = DAT_00ffef14 & 2;
  DAT_00ffef14 = DAT_00ffef14 & 0xfd;
  if (bVar1 != 0) {
    service_pending_vdp_upload();
    DAT_00ffdd02 = 1;
  }
  bVar1 = DAT_00ffef14 & 1;
  DAT_00ffef14 = DAT_00ffef14 & 0xfe;
  if (bVar1 != 0) {
    upload_startup_palette();
  }
  if (_DAT_00ffdfe8 == 0) {
    convert_intro_routine_0x0005672c();
  }
  bVar1 = DAT_00ffef14 & 4;
  DAT_00ffef14 = DAT_00ffef14 & 0xfb;
  if (bVar1 != 0) {
    write_transfer_header();
  }
  bVar1 = DAT_00ffef14 & 8;
  DAT_00ffef14 = DAT_00ffef14 & 0xf7;
  if (bVar1 != 0) {
    write_fixed_transfer_pair();
  }
  if (_DAT_00ffe100 != 0) {
    render_normal_field_viewport();
  }
  if (_DAT_00ffe102 != 0) {
    render_alternate_field_viewport();
  }
  if (DAT_00ffdf2a != 0) {
    flush_primary_queued_transfers();
  }
  if (DAT_00ffdf30 != 0) {
    flush_secondary_queued_transfers();
  }
  return;
}


