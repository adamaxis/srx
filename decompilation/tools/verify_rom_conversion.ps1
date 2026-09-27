param(
    [string]$RepoRoot,
    [switch]$Strict,
    [switch]$RequireBehavioralClosure
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

function Normalize-RomAddress([string]$Value) {
    $hex = $Value.Trim() -replace '^0x', ''
    $hex = $hex.TrimStart('0')
    if ([string]::IsNullOrEmpty($hex)) {
        return "0x0"
    }
    return "0x$($hex.ToUpperInvariant())"
}

function Is-GenericLabel([string]$Value) {
    return $Value -match '^(convert_|routine_|function_|unknown|sub_|fn_)'
}

function Read-Tsv([string]$Path) {
    return @(Import-Csv -LiteralPath $Path -Delimiter "`t")
}

$exportRoot = Join-Path $RepoRoot "decompilation\export"
$docsRoot = Join-Path $RepoRoot "decompilation\docs"
$conversionRoot = Join-Path $RepoRoot "decompilation\ghidra_conversion"
$issues = New-Object System.Collections.Generic.List[string]

$functions = Read-Tsv (Join-Path $exportRoot "functions.tsv")
$functionByAddress = @{}
$duplicateFunctions = @($functions | Group-Object { Normalize-RomAddress $_.entry } |
    Where-Object Count -gt 1)
foreach ($group in $duplicateFunctions) {
    [void]$issues.Add("Duplicate function export entry: $($group.Name)")
}
foreach ($function in $functions) {
    $functionByAddress[(Normalize-RomAddress $function.entry)] = $function
    $file = Join-Path $exportRoot $function.file.Replace('/', '\')
    if (-not (Test-Path -LiteralPath $file)) {
        [void]$issues.Add("Missing Ghidra source file for $($function.entry): $($function.file)")
        continue
    }
    $source = [IO.File]::ReadAllText($file)
    $nameMatch = [regex]::Match($source, '(?m)^// Name: ([A-Za-z_][A-Za-z0-9_]*)')
    if (-not $nameMatch.Success) {
        [void]$issues.Add("Ghidra source has no Name header: $($function.entry)")
    } elseif ($nameMatch.Groups[1].Value -ne $function.name) {
        [void]$issues.Add("Ghidra source name mismatch at $($function.entry): export '$($function.name)' source '$($nameMatch.Groups[1].Value)'")
    } elseif (Is-GenericLabel $nameMatch.Groups[1].Value) {
        [void]$issues.Add("Ghidra source Name header remains generic at $($function.entry)")
    }
}

$genericFunctions = @($functions | Where-Object { Is-GenericLabel $_.name })
$review = Read-Tsv (Join-Path $docsRoot "ghidra-function-label-review.tsv")
$expectedReviewedGenericLabels = 119
if ($review.Count -ne $expectedReviewedGenericLabels) {
    [void]$issues.Add("Expected $expectedReviewedGenericLabels reviewed generic labels, found $($review.Count)")
}
$reviewGroups = @($review | Group-Object { Normalize-RomAddress $_.rom_entry } |
    Where-Object Count -gt 1)
foreach ($group in $reviewGroups) {
    [void]$issues.Add("Duplicate label review entry: $($group.Name)")
}
$reviewLabelGroups = @($review | Group-Object export_label | Where-Object Count -gt 1)
foreach ($group in $reviewLabelGroups) {
    [void]$issues.Add("Duplicate original export label in review: $($group.Name)")
}
$reviewByAddress = @{}
foreach ($row in $review) {
    $address = Normalize-RomAddress $row.rom_entry
    $reviewByAddress[$address] = $row
    if (-not $functionByAddress.ContainsKey($address)) {
        [void]$issues.Add("Label review address is not an export: $($row.rom_entry)")
        continue
    }
    $function = $functionByAddress[$address]
    if ($function.name -ne $row.reviewed_name) {
        [void]$issues.Add("Label mismatch at ${address}: export '$($function.name)' review '$($row.reviewed_name)'")
    }
    if (-not (Is-GenericLabel $row.export_label)) {
        [void]$issues.Add("Label review row is not for a generic export label: $address")
    }
    if (Is-GenericLabel $row.reviewed_name) {
        [void]$issues.Add("Reviewed label remains generic at ${address}: $($row.reviewed_name)")
    }
    if ([string]::IsNullOrWhiteSpace($row.behavior) -or
        [string]::IsNullOrWhiteSpace($row.evidence)) {
        [void]$issues.Add("Label review lacks behavior/evidence at $address")
    }
}

$status = Read-Tsv (Join-Path $docsRoot "decompiled-function-status.tsv")
$statusByAddress = @{}
$duplicateStatus = @($status | Group-Object { Normalize-RomAddress $_.rom_entry } |
    Where-Object Count -gt 1)
foreach ($group in $duplicateStatus) {
    [void]$issues.Add("Duplicate status ledger entry: $($group.Name)")
}
foreach ($row in $status) {
    $statusByAddress[(Normalize-RomAddress $row.rom_entry)] = $row
}
foreach ($function in $functions) {
    $address = Normalize-RomAddress $function.entry
    if (-not $statusByAddress.ContainsKey($address)) {
        [void]$issues.Add("Export has no status ledger row: $address $($function.name)")
    }
}

$callSites = Read-Tsv (Join-Path $exportRoot "call-sites.tsv")
$resolvedCalls = @($callSites | Where-Object { $_.status -eq "resolved" })
$indirectCalls = @($callSites | Where-Object { $_.status -ne "resolved" })
foreach ($call in $callSites) {
    $callerAddress = Normalize-RomAddress $call.caller
    if ($functionByAddress.ContainsKey($callerAddress) -and
        $functionByAddress[$callerAddress].name -ne $call.caller_name) {
        [void]$issues.Add("Call-site caller label mismatch at $callerAddress")
    }
    $calleeAddress = Normalize-RomAddress $call.target
    if ($functionByAddress.ContainsKey($calleeAddress) -and
        $functionByAddress[$calleeAddress].name -ne $call.callee) {
        [void]$issues.Add("Call-site callee label mismatch at $calleeAddress")
    }
}
$callGraph = Read-Tsv (Join-Path $exportRoot "callgraph.tsv")
$resolvedPairCounts = @{}
foreach ($call in $resolvedCalls) {
    $pair = "$(Normalize-RomAddress $call.caller)|$(Normalize-RomAddress $call.target)"
    if (-not $resolvedPairCounts.ContainsKey($pair)) {
        $resolvedPairCounts[$pair] = 0
    }
    $resolvedPairCounts[$pair]++
}
$callGraphPairCounts = @{}
foreach ($edge in $callGraph) {
    $callerAddress = Normalize-RomAddress $edge.caller
    if ($functionByAddress.ContainsKey($callerAddress) -and
        $functionByAddress[$callerAddress].name -ne $edge.caller_name) {
        [void]$issues.Add("Call-graph caller label mismatch at $callerAddress")
    }
    $calleeAddress = Normalize-RomAddress $edge.callee
    if ($functionByAddress.ContainsKey($calleeAddress) -and
        $functionByAddress[$calleeAddress].name -ne $edge.callee_name) {
        [void]$issues.Add("Call-graph callee label mismatch at $calleeAddress")
    }
    $pair = "$callerAddress|$calleeAddress"
    $callGraphPairCounts[$pair] = [int]$edge.call_sites
}
foreach ($pair in $resolvedPairCounts.Keys) {
    if (-not $callGraphPairCounts.ContainsKey($pair) -or
        $callGraphPairCounts[$pair] -ne $resolvedPairCounts[$pair]) {
        [void]$issues.Add("Call-graph count mismatch for $pair")
    }
}
foreach ($pair in $callGraphPairCounts.Keys) {
    if (-not $resolvedPairCounts.ContainsKey($pair)) {
        [void]$issues.Add("Call-graph contains a pair absent from resolved call sites: $pair")
    }
}

$mapRows = Read-Tsv (Join-Path $docsRoot "conversion-source-modules.tsv")
$sourceFiles = @(Get-ChildItem (Join-Path $conversionRoot "src") -Recurse -Filter *.cpp -File |
    ForEach-Object { $_.FullName.Substring($conversionRoot.Length + 1).Replace('\', '/') } |
    Where-Object {
        $_ -notin @(
            "src/integration/conversion_workbench.cpp",
            "src/integration/startup_integration_conversion.cpp"
        )
    })
$owners = @{}
foreach ($mapRow in $mapRows) {
    foreach ($glob in ($mapRow.source_globs -split ';')) {
        $pattern = Join-Path $conversionRoot ($glob.Replace('/', '\'))
        foreach ($match in @(Get-ChildItem $pattern -File -ErrorAction SilentlyContinue)) {
            $relative = $match.FullName.Substring($conversionRoot.Length + 1).Replace('\', '/')
            if ($owners.ContainsKey($relative)) {
                [void]$issues.Add("Conversion source assigned twice: $relative")
            }
            $owners[$relative] = $mapRow.module_id
        }
    }
}
$unassigned = @($sourceFiles | Where-Object { -not $owners.ContainsKey($_) })
foreach ($file in $unassigned) {
    [void]$issues.Add("Unassigned conversion source file: $file")
}

$decompilerOmissions = @(
    "0x104F6: generated C stops after the slot-6..0 scan; disassembly continues with the slot-7 0x29..0x2B table path.",
    "0x12C74: generated C is a return-only body; disassembly contains the action-class skill selection and strip merge.",
    "Indirect calls: $($indirectCalls.Count) call sites remain explicit boundaries and require per-target source tracing."
)

$statusCounts = $status | Group-Object status | Sort-Object Name
$testFiles = @(Get-ChildItem (Join-Path $conversionRoot "tests") -Recurse -Filter *.cpp -File)
$ctestCount = $testFiles.Count + 1

"ROM/conversion structural verification"
"Repository: $RepoRoot"
"Exported function entries: $($functions.Count)"
"Generic export labels remaining: $($genericFunctions.Count)"
"Generic Ghidra C Name headers remaining: $(@($functions | Where-Object { $file = Join-Path $exportRoot $_.file.Replace('/', '\'); $m = [regex]::Match([IO.File]::ReadAllText($file), '(?m)^// Name: ([A-Za-z_][A-Za-z0-9_]*)'); $m.Success -and (Is-GenericLabel $m.Groups[1].Value) }).Count)"
"Reviewed generic labels: $($review.Count)"
"Status ledger rows: $($status.Count)"
"Duplicate status rows: $($duplicateStatus.Count)"
"Resolved direct call sites: $($resolvedCalls.Count)"
"Call-graph direct edge occurrences: $(($callGraph | Measure-Object -Property call_sites -Sum).Sum)"
"Indirect/unresolved call sites: $($indirectCalls.Count)"
"Conversion implementation files: $($sourceFiles.Count)"
"Behavioral conversion modules: $($mapRows.Count)"
"Conversion test source files: $($testFiles.Count)"
"CTest cases (runner plus focused tests): $ctestCount"
""
"Status coverage:"
foreach ($group in $statusCounts) {
    "  $($group.Name): $($group.Count)"
}
""
"Known source-verification warnings:"
foreach ($warning in $decompilerOmissions) {
    "  $warning"
}

if ($issues.Count -gt 0) {
    ""
    "FAIL: structural issues"
    foreach ($issue in $issues) {
        "  $issue"
    }
    if ($Strict) {
        exit 1
    }
} else {
    ""
    "PASS: export labels, status coverage, source ownership, and source-file references are structurally consistent."
}

$behaviorallyClosed = ($genericFunctions.Count -eq 0 -and
    $duplicateStatus.Count -eq 0 -and
    $indirectCalls.Count -eq 0 -and
    @($status | Where-Object { $_.status -ne "defended" }).Count -eq 0)
if ($behaviorallyClosed) {
    "BEHAVIORAL_FIDELITY_GATE: CLOSED"
} else {
    "BEHAVIORAL_FIDELITY_GATE: OPEN (structural coverage is not behavioral proof)"
    if ($RequireBehavioralClosure) {
        exit 2
    }
}
