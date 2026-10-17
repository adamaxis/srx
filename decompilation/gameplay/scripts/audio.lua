-- Audio rules: Lua decides what plays; the host mixer only plays resources
-- named in assets/audio/audio_manifest.tsv (doc 26 §4.5).
local M = {}

-- Field music on scene init. ROM 0x2830 sets FFE172 = FFFF on every scene
-- change (also 0x0968 / 0x17532); 0x2900 then clears it and calls
-- play_music(FFDE06) at 0x2916. FFDE06 is scene profile word +0x40, copied
-- by 0x3084 [C]. There is no "same song keeps playing" test: the song
-- restarts even when it is unchanged (doc 10 §6) [C]. A same-scene redraw
-- (0x2BD2 with FFDF8E clear) does not set FFE172 and so does not reach here.
-- Returns an E.MusicId value, or nil for silence.
function M.scene_music(scene)
    return scene.profile_music
end

return M
