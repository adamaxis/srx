-- Scene activity descriptions. The native adapter supplies completed story
-- facts by name; this content does not depend on packed flag positions.
return {
    [10] = {
        { activity_id = "scene_10_event_6_script",
          variant = "alternate",
          world_activity_id = "scene_10_event_6_script" },
        { activity_id = "scene_10_default_script",
          variant = "standard",
          world_activity_id = "scene_10_default_script" },
    },
    [11] = {
        { activity_id = "scene_11_script",
          world_activity_id = "scene_11_script" },
    },
    -- Scene 12 has no authored world-activity list.
    [13] = {
        { activity_id = "scene_13_ilene_gate_script",
          required_story_fact = "ilene_met",
          world_activity_id = "scene_13_ilene_gate_script" },
    },
    [14] = {
        { activity_id = "scene_14_script",
          world_activity_id = "scene_14_script" },
    },
    [15] = {
        { activity_id = "scene_15_script",
          required_story_fact = "heart_delivered",
          world_activity_id = "scene_15_script" },
    },
    [16] = {
        { activity_id = "scene_16_script",
          world_activity_id = "scene_16_script" },
    },
    [17] = {
        { activity_id = "scene_17_script",
          world_activity_id = "scene_17_script" },
    },
    [18] = {
        { activity_id = "scene_18_script",
          world_activity_id = "scene_18_script" },
    },
    [19] = {
        { activity_id = "scene_19_script",
          world_activity_id = "scene_19_script" },
    },
}
