local E = require("enums")

local ScreenEffects = {}

local function clamp(value, minimum, maximum)
    return math.max(minimum, math.min(maximum, value))
end

local function line_scroll_reveal(parameters)
    local effect = {
        kind = E.ScreenEffectKind.LINE_SCROLL_REVEAL,
        status = E.ScreenEffectStatus.ACTIVE,
        duration_vblanks = parameters.duration_vblanks,
        elapsed_vblanks = 0,
        effect_counter = parameters.start_counter,
        counter_cadence = parameters.counter_cadence,
        eligible_vblanks = 0,
        counter_step_count = 0,
        counter_stepped = false,
        first_reveal_counter = parameters.first_reveal_counter,
        full_reveal_counter = parameters.full_reveal_counter,
        initial_visible_rows = parameters.initial_visible_rows,
        line_count = parameters.line_count,
        shimmer_tail_rows = parameters.shimmer_tail_rows,
        max_line_offset = parameters.max_line_offset,
        -- Native callers provide the source stream when they share the
        -- game's PRNG; zero is only the deterministic module-local fallback.
        random_state = parameters.random_seed or 0,
        shimmer_phase = 0,
        stage = E.PreTitleLogoReveal.STAGE_HIDDEN,
        visible_rows = 0,
        edge_row = -1,
        settled_rows = 0,
        line_offsets = {},
    }
    for row = 1, effect.line_count do
        effect.line_offsets[row] = 0
    end
    return effect
end

local function palette_fade(parameters)
    return {
        kind = E.ScreenEffectKind.PALETTE_FADE,
        status = E.ScreenEffectStatus.ACTIVE,
        duration_vblanks = parameters.duration_vblanks,
        elapsed_vblanks = 0,
        direction = parameters.direction,
        start_alpha = parameters.start_alpha,
        end_alpha = parameters.end_alpha,
        alpha = parameters.start_alpha,
        progress = 0,
    }
end

function ScreenEffects.create(kind, parameters)
    parameters = parameters or {}
    if kind == E.ScreenEffectKind.LINE_SCROLL_REVEAL then
        local R = E.PreTitleLogoReveal
        return line_scroll_reveal{
            duration_vblanks = parameters.duration_vblanks or R.DURATION_VBLANKS,
            start_counter = parameters.start_counter or R.START_COUNTER,
            counter_cadence = parameters.counter_cadence or
                R.COUNTER_STEP_CADENCE,
            first_reveal_counter = parameters.first_reveal_counter or
                R.FIRST_REVEAL_COUNTER,
            full_reveal_counter = parameters.full_reveal_counter or
                R.FULL_REVEAL_COUNTER,
            initial_visible_rows = parameters.initial_visible_rows or
                R.INITIAL_VISIBLE_ROWS,
            line_count = parameters.line_count or R.LOGO_HEIGHT,
            shimmer_tail_rows = parameters.shimmer_tail_rows or
                R.SHIMMER_TAIL_ROWS,
            max_line_offset = parameters.max_line_offset or
                R.MAX_LINE_OFFSET,
            random_seed = parameters.random_seed,
        }
    end
    if kind == E.ScreenEffectKind.PALETTE_FADE then
        local direction = parameters.direction or E.ScreenEffectFadeDirection.OUT
        local default_start = direction == E.ScreenEffectFadeDirection.OUT
            and 255 or 0
        local default_end = direction == E.ScreenEffectFadeDirection.OUT
            and 0 or 255
        return palette_fade{
            duration_vblanks = parameters.duration_vblanks or
                E.FieldSceneTransitionVBlanks.BUILDING_FADE,
            direction = direction,
            start_alpha = parameters.start_alpha or default_start,
            end_alpha = parameters.end_alpha or default_end,
        }
    end
    error("screen_effects.create: unknown effect kind " .. tostring(kind))
end

local function next_line_offset(effect)
    -- 0x0D44: the shared host PRNG uses state*0x20E5+1 before this five-bit
    -- line-scroll sample. The boot runtime normally supplies its already
    -- measured line samples; this path keeps the effect reusable on its own.
    effect.random_state =
        (effect.random_state * E.ScreenEffectPrng.MULTIPLIER + 1) % 0x10000
    return math.floor(effect.random_state / 32) %
        (effect.max_line_offset + 1)
end

local function resolve_line_offsets(effect, input)
    local samples = input.line_scroll_values
    local reveal_band_start = math.max(
        1, effect.visible_rows - effect.shimmer_tail_rows + 1)
    effect.settled_rows = effect.stage == E.PreTitleLogoReveal.STAGE_REVEAL
        and reveal_band_start - 1 or effect.visible_rows

    for row = 1, effect.line_count do
        if row > effect.visible_rows or row <= effect.settled_rows then
            effect.line_offsets[row] = 0
        else
            local sample = samples and samples[row] or next_line_offset(effect)
            effect.line_offsets[row] = clamp(
                math.floor(sample), 0, effect.max_line_offset)
        end
    end
end

local function update_line_scroll_reveal(effect, input)
    effect.elapsed_vblanks = effect.elapsed_vblanks + 1
    effect.counter_stepped = false
    if input.counter_eligible then
        effect.eligible_vblanks = effect.eligible_vblanks + 1
        if effect.eligible_vblanks % effect.counter_cadence == 0 then
            effect.effect_counter = effect.effect_counter - 1
            effect.counter_step_count = effect.counter_step_count + 1
            effect.counter_stepped = true
        end
    end

    effect.shimmer_phase = math.floor(input.shimmer_phase or 0) %
        E.PreTitleLogoReveal.SHIMMER_PHASE_COUNT
    if effect.effect_counter > effect.first_reveal_counter then
        effect.stage = E.PreTitleLogoReveal.STAGE_HIDDEN
        effect.visible_rows = 0
    elseif effect.effect_counter <= effect.full_reveal_counter then
        effect.stage = E.PreTitleLogoReveal.STAGE_COMPLETE
        effect.visible_rows = effect.line_count
    else
        effect.stage = E.PreTitleLogoReveal.STAGE_REVEAL
        local progress = (effect.first_reveal_counter - effect.effect_counter) /
            (effect.first_reveal_counter - effect.full_reveal_counter)
        local row_span = effect.line_count - effect.initial_visible_rows
        effect.visible_rows = clamp(
            math.ceil(effect.initial_visible_rows + progress * row_span),
            effect.initial_visible_rows, effect.line_count)
    end
    effect.edge_row = effect.visible_rows > 0 and effect.visible_rows - 1 or -1
    resolve_line_offsets(effect, input)

    if effect.elapsed_vblanks >= effect.duration_vblanks then
        ScreenEffects.finish(effect)
    end
    return effect
end

local function update_palette_fade(effect)
    effect.elapsed_vblanks = effect.elapsed_vblanks + 1
    effect.progress = clamp(
        effect.elapsed_vblanks / effect.duration_vblanks, 0, 1)
    effect.alpha = math.floor(effect.start_alpha +
        (effect.end_alpha - effect.start_alpha) * effect.progress + 0.5)
    if effect.elapsed_vblanks >= effect.duration_vblanks then
        ScreenEffects.finish(effect)
    end
    return effect
end

function ScreenEffects.update(effect, input)
    if effect.status ~= E.ScreenEffectStatus.ACTIVE then
        return effect
    end
    input = input or {}
    if effect.kind == E.ScreenEffectKind.LINE_SCROLL_REVEAL then
        return update_line_scroll_reveal(effect, input)
    end
    if effect.kind == E.ScreenEffectKind.PALETTE_FADE then
        return update_palette_fade(effect)
    end
    error("screen_effects.update: unknown effect kind " .. tostring(effect.kind))
end

function ScreenEffects.finish(effect)
    effect.status = E.ScreenEffectStatus.FINISHED
    if effect.kind == E.ScreenEffectKind.LINE_SCROLL_REVEAL then
        effect.stage = E.PreTitleLogoReveal.STAGE_COMPLETE
        effect.visible_rows = effect.line_count
        effect.edge_row = effect.line_count - 1
        effect.settled_rows = effect.line_count
        for row = 1, effect.line_count do
            effect.line_offsets[row] = 0
        end
    elseif effect.kind == E.ScreenEffectKind.PALETTE_FADE then
        effect.progress = 1
        effect.alpha = effect.end_alpha
    end
    return effect
end

return ScreenEffects
