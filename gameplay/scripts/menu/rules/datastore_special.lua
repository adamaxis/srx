-- rules/datastore_special.lua: AUTO-TRANSLATED from ROM 0x55058..0x5559C (tools/translate_55058.py logic, 2026-10-15) [C].
-- Story datafiles: called by Transfer Data before the random file. Returns true when this datastore (system FFFBBC,
-- node file class d0 = node +1) yields a story file: the bit in FFFC38 (or FFFC34 for system 0x12) is set (text 0x166968).
-- r = { system, fc38, fc34, fc1e, fc22, fbfa, fbfb, fc26, fc27, nuyen }; r is updated in place.
-- Quirk kept [C]: system 0x10 class 1 needs bits 0x1A..0x1E of FFFC38 and pays +50,000 nuyen (0xC350).
local function s8(v) v = v & 0xFF; return v >= 0x80 and v - 0x100 or v end
return function(r, d0)
    local d5, d1, cc = 0, 0, 0
    ::L055058:: -- clr.l d0
    ::L05505A:: -- clr.l d1
    ::L05505C:: -- move.w $fff118.l, d1
    ::L055062:: -- mulu.w #$14, d1
    ::L055066:: -- lea.l $ff1100.l, a1
    ::L05506C:: -- move.b $1(a1, d1.w), d0
    ::L055070:: d5 = r.fc38
    ::L055076:: d1 = r.fc1e
    ::L05507C:: cc = s8(r.system) - 18
    ::L055084:: if cc ~= 0 then goto L0550F4 end
    ::L055088:: d5 = r.fc34
    ::L05508E:: cc = d0 - 3
    ::L055092:: if cc ~= 0 then goto L0550A6 end
    ::L055096:: cc = (d5 >> 28) & 1
    ::L05509A:: if cc ~= 0 then goto L05559A end
    ::L05509E:: d5 = d5 | (1 << 28)
    ::L0550A2:: goto L0550EA
    ::L0550A6:: cc = d0 - 10
    ::L0550AA:: if cc ~= 0 then goto L0550BE end
    ::L0550AE:: cc = (d5 >> 29) & 1
    ::L0550B2:: if cc ~= 0 then goto L05559A end
    ::L0550B6:: d5 = d5 | (1 << 29)
    ::L0550BA:: goto L0550EA
    ::L0550BE:: cc = d0 - 9
    ::L0550C2:: if cc ~= 0 then goto L0550D6 end
    ::L0550C6:: cc = (d5 >> 30) & 1
    ::L0550CA:: if cc ~= 0 then goto L05559A end
    ::L0550CE:: d5 = d5 | (1 << 30)
    ::L0550D2:: goto L0550EA
    ::L0550D6:: cc = d0 - 2
    ::L0550DA:: if cc ~= 0 then goto L05559A end
    ::L0550DE:: cc = (d5 >> 31) & 1
    ::L0550E2:: if cc ~= 0 then goto L05559A end
    ::L0550E6:: d5 = d5 | (1 << 31)
    ::L0550EA:: r.fc34 = d5
    ::L0550F0:: goto L05557E
    ::L0550F4:: cc = s8(r.system) - 17
    ::L0550FC:: if cc ~= 0 then goto L055138 end
    ::L055100:: cc = d0 - 9
    ::L055104:: if cc ~= 0 then goto L05559A end
    ::L055108:: cc = (d1 >> 11) & 1
    ::L05510C:: if cc == 0 then goto L05559A end
    ::L055110:: cc = (d5 >> 0) & 1
    ::L055114:: if cc ~= 0 then goto L05559A end
    ::L055118:: d5 = d5 | (1 << 0)
    ::L05511C:: goto L055578
    ::L055120:: cc = d0 - 0
    ::L055124:: if cc ~= 0 then goto L05559A end
    ::L055128:: cc = (d5 >> 1) & 1
    ::L05512C:: if cc ~= 0 then goto L05559A end
    ::L055130:: d5 = d5 | (1 << 1)
    ::L055134:: goto L055578
    ::L055138:: cc = s8(r.system) - 7
    ::L055140:: if cc ~= 0 then goto L05515C end
    ::L055144:: cc = d0 - 10
    ::L055148:: if cc ~= 0 then goto L05559A end
    ::L05514C:: cc = (d5 >> 26) & 1
    ::L055150:: if cc ~= 0 then goto L05559A end
    ::L055154:: d5 = d5 | (1 << 26)
    ::L055158:: goto L055578
    ::L05515C:: cc = s8(r.system) - 22
    ::L055164:: if cc ~= 0 then goto L0551C0 end
    ::L055168:: cc = d0 - 2
    ::L05516C:: if cc ~= 0 then goto L055188 end
    ::L055170:: cc = (d1 >> 29) & 1
    ::L055174:: if cc ~= 0 then goto L05559A end
    ::L055178:: cc = (d5 >> 2) & 1
    ::L05517C:: if cc ~= 0 then goto L05559A end
    ::L055180:: d5 = d5 | (1 << 2)
    ::L055184:: goto L055578
    ::L055188:: cc = d0 - 4
    ::L05518C:: if cc ~= 0 then goto L0551A8 end
    ::L055190:: cc = (d1 >> 29) & 1
    ::L055194:: if cc ~= 0 then goto L0551A8 end
    ::L055198:: cc = (d5 >> 3) & 1
    ::L05519C:: if cc ~= 0 then goto L0551A8 end
    ::L0551A0:: d5 = d5 | (1 << 3)
    ::L0551A4:: goto L055578
    ::L0551A8:: cc = d0 - 4
    ::L0551AC:: if cc ~= 0 then goto L05559A end
    ::L0551B0:: cc = (d5 >> 27) & 1
    ::L0551B4:: if cc ~= 0 then goto L05559A end
    ::L0551B8:: d5 = d5 | (1 << 27)
    ::L0551BC:: goto L055578
    ::L0551C0:: cc = s8(r.system) - 20
    ::L0551C8:: if cc ~= 0 then goto L055224 end
    ::L0551CC:: cc = d0 - 10
    ::L0551D0:: if cc ~= 0 then goto L0551E4 end
    ::L0551D4:: cc = (d5 >> 28) & 1
    ::L0551D8:: if cc ~= 0 then goto L05559A end
    ::L0551DC:: d5 = d5 | (1 << 28)
    ::L0551E0:: goto L055578
    ::L0551E4:: cc = d0 - 10
    ::L0551E8:: if cc ~= 0 then goto L055204 end
    ::L0551EC:: cc = (d1 >> 3) & 1
    ::L0551F0:: if cc ~= 0 then goto L05559A end
    ::L0551F4:: cc = (d5 >> 5) & 1
    ::L0551F8:: if cc ~= 0 then goto L05559A end
    ::L0551FC:: d5 = d5 | (1 << 5)
    ::L055200:: goto L055578
    ::L055204:: cc = d0 - 4
    ::L055208:: if cc ~= 0 then goto L05559A end
    ::L05520C:: cc = (d1 >> 3) & 1
    ::L055210:: if cc ~= 0 then goto L05559A end
    ::L055214:: cc = (d5 >> 4) & 1
    ::L055218:: if cc ~= 0 then goto L05559A end
    ::L05521C:: d5 = d5 | (1 << 4)
    ::L055220:: goto L055578
    ::L055224:: cc = s8(r.system) - 23
    ::L05522C:: if cc ~= 0 then goto L055302 end
    ::L055230:: cc = d0 - 13
    ::L055234:: if cc ~= 0 then goto L0552A2 end
    ::L055238:: cc = s8(r.fbfa)
    ::L05523E:: if cc == 0 then goto L05559A end
    ::L055242:: cc = s8(r.fbfa) - 8
    ::L05524A:: if cc >= 0 then goto L05528A end
    ::L05524E:: cc = s8(r.fbfa) - 3
    ::L055256:: if cc >= 0 then goto L055272 end
    ::L05525A:: cc = (d5 >> 6) & 1
    ::L05525E:: if cc ~= 0 then goto L05559A end
    ::L055262:: d5 = d5 | (1 << 6)
    ::L055266:: d5 = d5 & ~(1 << 7)
    ::L05526A:: d5 = d5 & ~(1 << 8)
    ::L05526E:: goto L055578
    ::L055272:: cc = (d5 >> 7) & 1
    ::L055276:: if cc ~= 0 then goto L05559A end
    ::L05527A:: d5 = d5 | (1 << 7)
    ::L05527E:: d5 = d5 & ~(1 << 6)
    ::L055282:: d5 = d5 & ~(1 << 8)
    ::L055286:: goto L055578
    ::L05528A:: cc = (d5 >> 8) & 1
    ::L05528E:: if cc ~= 0 then goto L05559A end
    ::L055292:: d5 = d5 | (1 << 8)
    ::L055296:: d5 = d5 & ~(1 << 6)
    ::L05529A:: d5 = d5 & ~(1 << 7)
    ::L05529E:: goto L055578
    ::L0552A2:: cc = d0 - 12
    ::L0552A6:: if cc ~= 0 then goto L0552E2 end
    ::L0552AA:: cc = (r.fc27 >> 3) & 1
    ::L0552B2:: if cc == 0 then goto L0552C6 end
    ::L0552B6:: cc = (d5 >> 9) & 1
    ::L0552BA:: if cc ~= 0 then goto L05559A end
    ::L0552BE:: d5 = d5 | (1 << 9)
    ::L0552C2:: goto L055578
    ::L0552C6:: cc = (r.fc26 >> 0) & 1
    ::L0552CE:: if cc == 0 then goto L05559A end
    ::L0552D2:: cc = (d5 >> 10) & 1
    ::L0552D6:: if cc ~= 0 then goto L05559A end
    ::L0552DA:: d5 = d5 | (1 << 10)
    ::L0552DE:: goto L055578
    ::L0552E2:: cc = d0 - 9
    ::L0552E6:: if cc ~= 0 then goto L05559A end
    ::L0552EA:: cc = (d1 >> 29) & 1
    ::L0552EE:: if cc ~= 0 then goto L05559A end
    ::L0552F2:: cc = (d5 >> 11) & 1
    ::L0552F6:: if cc ~= 0 then goto L05559A end
    ::L0552FA:: d5 = d5 | (1 << 11)
    ::L0552FE:: goto L055578
    ::L055302:: cc = s8(r.system) - 24
    ::L05530A:: if cc ~= 0 then goto L055346 end
    ::L05530E:: cc = d0 - 10
    ::L055312:: if cc ~= 0 then goto L05559A end
    ::L055316:: cc = (d5 >> 29) & 1
    ::L05531A:: if cc ~= 0 then goto L055326 end
    ::L05531E:: d5 = d5 | (1 << 29)
    ::L055322:: goto L055578
    ::L055326:: cc = d0 - 10
    ::L05532A:: if cc ~= 0 then goto L05559A end
    ::L05532E:: cc = (d1 >> 19) & 1
    ::L055332:: if cc ~= 0 then goto L05559A end
    ::L055336:: cc = (d5 >> 12) & 1
    ::L05533A:: if cc ~= 0 then goto L05559A end
    ::L05533E:: d5 = d5 | (1 << 12)
    ::L055342:: goto L055578
    ::L055346:: cc = s8(r.system) - 19
    ::L05534E:: if cc ~= 0 then goto L0553B2 end
    ::L055352:: cc = d0 - 12
    ::L055356:: if cc ~= 0 then goto L055372 end
    ::L05535A:: cc = (d1 >> 19) & 1
    ::L05535E:: if cc ~= 0 then goto L05559A end
    ::L055362:: cc = (d5 >> 13) & 1
    ::L055366:: if cc ~= 0 then goto L05559A end
    ::L05536A:: d5 = d5 | (1 << 13)
    ::L05536E:: goto L055578
    ::L055372:: cc = d0 - 0
    ::L055376:: if cc ~= 0 then goto L05559A end
    ::L05537A:: cc = (r.fc27 >> 3) & 1
    ::L055382:: if cc == 0 then goto L055396 end
    ::L055386:: cc = (d5 >> 14) & 1
    ::L05538A:: if cc ~= 0 then goto L055396 end
    ::L05538E:: d5 = d5 | (1 << 14)
    ::L055392:: goto L055578
    ::L055396:: cc = (r.fc26 >> 0) & 1
    ::L05539E:: if cc == 0 then goto L05559A end
    ::L0553A2:: cc = (d5 >> 15) & 1
    ::L0553A6:: if cc ~= 0 then goto L05559A end
    ::L0553AA:: d5 = d5 | (1 << 15)
    ::L0553AE:: goto L055578
    ::L0553B2:: cc = s8(r.system) - 21
    ::L0553BA:: if cc ~= 0 then goto L055526 end
    ::L0553BE:: cc = d0 - 10
    ::L0553C2:: if cc ~= 0 then goto L0553F6 end
    ::L0553C6:: cc = (d5 >> 30) & 1
    ::L0553CA:: if cc ~= 0 then goto L0553D6 end
    ::L0553CE:: d5 = d5 | (1 << 30)
    ::L0553D2:: goto L055578
    ::L0553D6:: cc = (d1 >> 11) & 1
    ::L0553DA:: if cc ~= 0 then goto L0553E6 end
    ::L0553DE:: cc = (d1 >> 13) & 1
    ::L0553E2:: if cc == 0 then goto L05559A end
    ::L0553E6:: cc = (d5 >> 18) & 1
    ::L0553EA:: if cc ~= 0 then goto L05559A end
    ::L0553EE:: d5 = d5 | (1 << 18)
    ::L0553F2:: goto L055578
    ::L0553F6:: cc = d0 - 3
    ::L0553FA:: if cc ~= 0 then goto L055474 end
    ::L0553FE:: cc = (d1 >> 7) & 1
    ::L055402:: if cc == 0 then goto L05541E end
    ::L055406:: cc = (d1 >> 8) & 1
    ::L05540A:: if cc ~= 0 then goto L05541E end
    ::L05540E:: cc = (d5 >> 16) & 1
    ::L055412:: if cc ~= 0 then goto L05541E end
    ::L055416:: d5 = d5 | (1 << 16)
    ::L05541A:: goto L055578
    ::L05541E:: cc = (d1 >> 11) & 1
    ::L055422:: if cc == 0 then goto L055436 end
    ::L055426:: cc = (d5 >> 19) & 1
    ::L05542A:: if cc ~= 0 then goto L055436 end
    ::L05542E:: d5 = d5 | (1 << 19)
    ::L055432:: goto L055578
    ::L055436:: cc = (d1 >> 10) & 1
    ::L05543A:: if cc ~= 0 then goto L055446 end
    ::L05543E:: cc = (d1 >> 13) & 1
    ::L055442:: if cc == 0 then goto L055456 end
    ::L055446:: cc = (d5 >> 24) & 1
    ::L05544A:: if cc ~= 0 then goto L055456 end
    ::L05544E:: d5 = d5 | (1 << 24)
    ::L055452:: goto L055578
    ::L055456:: d1 = r.fc22
    ::L05545C:: cc = (d1 >> 8) & 1
    ::L055460:: if cc == 0 then goto L05559A end
    ::L055464:: cc = (d5 >> 25) & 1
    ::L055468:: if cc ~= 0 then goto L05559A end
    ::L05546C:: d5 = d5 | (1 << 25)
    ::L055470:: goto L055578
    ::L055474:: cc = d0 - 2
    ::L055478:: if cc ~= 0 then goto L055494 end
    ::L05547C:: cc = (d1 >> 10) & 1
    ::L055480:: if cc == 0 then goto L05559A end
    ::L055484:: cc = (d5 >> 17) & 1
    ::L055488:: if cc ~= 0 then goto L05559A end
    ::L05548C:: d5 = d5 | (1 << 17)
    ::L055490:: goto L055578
    ::L055494:: cc = d0 - 4
    ::L055498:: if cc ~= 0 then goto L05559A end
    ::L05549C:: cc = (d1 >> 10) & 1
    ::L0554A0:: if cc ~= 0 then goto L0554AC end
    ::L0554A4:: cc = (d1 >> 13) & 1
    ::L0554A8:: if cc == 0 then goto L0554BC end
    ::L0554AC:: cc = (d5 >> 20) & 1
    ::L0554B0:: if cc ~= 0 then goto L0554BC end
    ::L0554B4:: d5 = d5 | (1 << 20)
    ::L0554B8:: goto L055578
    ::L0554BC:: cc = s8(r.fbfb)
    ::L0554C2:: if cc == 0 then goto L05559A end
    ::L0554C6:: cc = s8(r.fbfb) - 8
    ::L0554CE:: if cc >= 0 then goto L05550E end
    ::L0554D2:: cc = s8(r.fbfb) - 3
    ::L0554DA:: if cc >= 0 then goto L0554F6 end
    ::L0554DE:: cc = (d5 >> 21) & 1
    ::L0554E2:: if cc ~= 0 then goto L05559A end
    ::L0554E6:: d5 = d5 | (1 << 21)
    ::L0554EA:: d5 = d5 & ~(1 << 22)
    ::L0554EE:: d5 = d5 & ~(1 << 23)
    ::L0554F2:: goto L055578
    ::L0554F6:: cc = (d5 >> 22) & 1
    ::L0554FA:: if cc ~= 0 then goto L05559A end
    ::L0554FE:: d5 = d5 | (1 << 22)
    ::L055502:: d5 = d5 & ~(1 << 21)
    ::L055506:: d5 = d5 & ~(1 << 23)
    ::L05550A:: goto L055578
    ::L05550E:: cc = (d5 >> 23) & 1
    ::L055512:: if cc ~= 0 then goto L05559A end
    ::L055516:: d5 = d5 | (1 << 23)
    ::L05551A:: d5 = d5 & ~(1 << 21)
    ::L05551E:: d5 = d5 & ~(1 << 22)
    ::L055522:: goto L055578
    ::L055526:: cc = s8(r.system) - 16
    ::L05552E:: if cc ~= 0 then goto L05559A end
    ::L055532:: cc = d0 - 1
    ::L055536:: if cc ~= 0 then goto L05559A end
    ::L05553A:: cc = (d5 >> 26) & 1
    ::L05553E:: if cc == 0 then goto L05559A end
    ::L055542:: cc = (d5 >> 27) & 1
    ::L055546:: if cc == 0 then goto L05559A end
    ::L05554A:: cc = (d5 >> 28) & 1
    ::L05554E:: if cc == 0 then goto L05559A end
    ::L055552:: cc = (d5 >> 29) & 1
    ::L055556:: if cc == 0 then goto L05559A end
    ::L05555A:: cc = (d5 >> 30) & 1
    ::L05555E:: if cc == 0 then goto L05559A end
    ::L055562:: cc = (d5 >> 31) & 1
    ::L055566:: if cc ~= 0 then goto L05559A end
    ::L05556A:: d5 = d5 | (1 << 31)
    ::L05556E:: r.nuyen = r.nuyen + 0xC350
    ::L055578:: r.fc38 = d5
    ::L05557E:: -- lea.l $166968.l, a1
    ::L055584:: -- movea.l #$c616, a2
    ::L05558A:: -- move.w #$0, d1
    ::L05558E:: -- jsr $c0da.l
    ::L055594:: do return true end
    ::L055598:: -- rts
    ::L05559A:: do return false end
end
