-- US ROM 0x1684C pending actor selection. The pointer table at 0x16FD4
-- supplies the per-scene lists; marker values are the 0xEEFE byte plus one
-- as tested against FFFC0C/FFFC0D at 0x168EE-0x16900. Scene 10's two
-- FFFBF9-dependent lists are selected directly at 0x1686C-0x16880.
-- Scene 13 and 15 retain the adjacent FFFC1E bit gates from 0x168CE-0x168EA.
return {
    [10] = {
        { activity_id = "scene_10_event_6_script",
          variant = "alternate", list_id = "scene_10_event_6_script" },
        { activity_id = "scene_10_default_script",
          variant = "standard", list_id = "scene_10_default_script" },
    },
    [11] = {
        { activity_id = "scene_11_script", list_id = "scene_11_script" },
    },
    -- Scene 12 has no authored world-activity list.
    [13] = {
        { activity_id = "scene_13_ilene_gate_script",
          required_gate = "scene_13_activity_gate_passed",
          list_id = "scene_13_ilene_gate_script" },
    },
    [14] = {
        { activity_id = "scene_14_script", list_id = "scene_14_script" },
    },
    [15] = {
        { activity_id = "scene_15_script",
          required_gate = "scene_15_activity_gate_passed",
          list_id = "scene_15_script" },
    },
    [16] = {
        { activity_id = "scene_16_script", list_id = "scene_16_script" },
    },
    [17] = {
        { activity_id = "scene_17_script", list_id = "scene_17_script" },
    },
    [18] = {
        { activity_id = "scene_18_script", list_id = "scene_18_script" },
    },
    [19] = {
        { activity_id = "scene_19_script", list_id = "scene_19_script" },
    },
}
