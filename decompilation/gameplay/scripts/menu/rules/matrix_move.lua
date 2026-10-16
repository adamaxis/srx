-- rules/matrix_move.lua: Matrix movement, ROM-traced 2026-10-15 [C unless marked].
-- nodes[i] = node view (fn +0; fn < 0 terminates the system list), links[0..7] = +0x07..+0x0E (< 0 = none).
local M = {}
-- 0x53704 (Leave Node / Enter System): cursor k over the current node's links.
--   Right: k+1, and if links[k] < 0 -> k = 0 (0x537FA; link 0 is not re-checked). Left: k-1; below 0 -> scan 7..1 downward
--   for the last valid link (none found -> no move, cursor stays at -1 -> 0x537FA shows link 0) [C].
-- Quirk [C]: with all 8 links valid, Right from 7 reads +0x0F (IC condition) as link 8; we wrap to 0 instead (MIGRATION D31).
function M.link_right(links, k) k = k + 1; if k > 7 or links[k] < 0 then k = 0 end; return k end
function M.link_left(links, k)
    k = k - 1; if k >= 0 then return links[k] >= 0 and k or 0 end      -- 0x537FA reset
    for j = 7, 1, -1 do if links[j] >= 0 then return j end end
    return 0
end
-- A/B/C: previous = current, current = links[k] (0x53782). Start: back to the node menu unless FFF0A9 (forced map).
function M.follow(cur, links, k) return links[k], cur end              -- new current, new previous
-- 0x53864 Go To Node (CPU): cursor over ALL nodes, starts at 0 (even if that is the current node, quirk).
--   Right: +1, wrap to 0 at the terminator, skip current. Left: -1, below 0 -> last node, skip current.
local function count(nodes) local n = 0; while nodes[n] and nodes[n].fn >= 0 do n = n + 1 end; return n end
function M.goto_right(nodes, cur, i) local n = count(nodes); repeat i = i + 1; if i >= n then i = 0 end until i ~= cur; return i end
function M.goto_left(nodes, cur, i) local n = count(nodes); repeat i = i - 1; if i < 0 then i = n - 1 end until i ~= cur; return i end
-- A/B/C: previous = current if target is one of its links, else -1 (no retreat after a jump) (0x53920).
function M.goto_commit(nodes, cur, i)
    for j = 7, 0, -1 do if nodes[cur].links[j] == i then return i, cur end end
    return i, -1
end
return M
