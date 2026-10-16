param(
    [string]$RepoRoot,
    [switch]$Strict,
    [switch]$RequireBehavioralClosure,
    [switch]$NoWrite
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
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

function Convert-RomAddressToInt([string]$Value) {
    $normalized = Normalize-RomAddress $Value
    return [Convert]::ToInt32($normalized.Substring(2), 16)
}

function Remove-CppComments([string]$Text) {
    $withoutBlocks = [regex]::Replace($Text, '(?s)/\*.*?\*/', ' ')
    return [regex]::Replace($withoutBlocks, '(?m)//.*$', ' ')
}

function Add-Evidence([hashtable]$Index, [string]$Address, [string]$Path) {
    $key = Normalize-RomAddress $Address
    if (-not $Index.ContainsKey($key)) {
        $Index[$key] = New-Object System.Collections.Generic.List[string]
    }
    if ($Path -notin $Index[$key]) {
        [void]$Index[$key].Add($Path)
    }
}

$exportRoot = Join-Path $RepoRoot "export"
$docsRoot = Join-Path $RepoRoot "docs\ghidra"
$conversionRoot = Join-Path $RepoRoot "ghidra_conversion"
$functions = @(Import-Csv (Join-Path $exportRoot "functions.tsv") -Delimiter "`t")
$callSites = @(Import-Csv (Join-Path $exportRoot "call-sites.tsv") -Delimiter "`t")
$statusRows = @(Import-Csv (Join-Path $docsRoot "decompiled-function-status.tsv") -Delimiter "`t")
$issues = New-Object System.Collections.Generic.List[string]

$functionByAddress = @{}
foreach ($function in $functions) {
    $address = Normalize-RomAddress $function.entry
    if ($functionByAddress.ContainsKey($address)) {
        [void]$issues.Add("Duplicate function export entry: $address")
    }
    $functionByAddress[$address] = $function
}

$statusByAddress = @{}
foreach ($status in $statusRows) {
    $address = Normalize-RomAddress $status.rom_entry
    if ($statusByAddress.ContainsKey($address)) {
        [void]$issues.Add("Duplicate status row: $address")
    }
    $statusByAddress[$address] = $status
}

$callSitesByCaller = @{}
foreach ($call in $callSites) {
    $caller = Normalize-RomAddress $call.caller
    if (-not $callSitesByCaller.ContainsKey($caller)) {
        $callSitesByCaller[$caller] = New-Object System.Collections.Generic.List[object]
    }
    [void]$callSitesByCaller[$caller].Add($call)
}

$disassemblyPath = Join-Path $exportRoot "disassembly.s"
$instructions = @(
    foreach ($line in [IO.File]::ReadLines($disassemblyPath)) {
        $match = [regex]::Match($line, '^(?<address>[0-9A-Fa-f]{8}):\s(?<text>.*)$')
        if ($match.Success) {
            [pscustomobject]@{
                address = Convert-RomAddressToInt $match.Groups['address'].Value
                text = $match.Groups['text'].Value
            }
        }
    }
)

# Index each disassembly instruction into its exported function span once.
# Re-scanning the complete listing for every one of the 910 functions makes
# the audit needlessly expensive and obscures the useful report.
$instructionsByFunction = @{}
$sortedFunctions = @($functions | ForEach-Object {
    [pscustomobject]@{
        entry = Normalize-RomAddress $_.entry
        start = Convert-RomAddressToInt $_.entry
        end = (Convert-RomAddressToInt $_.entry) + [int]$_.size
    }
} | Sort-Object start)
$nextFunctionByEntry = @{}
for ($sortedIndex = 0; $sortedIndex -lt $sortedFunctions.Count - 1; ++$sortedIndex) {
    $current = $sortedFunctions[$sortedIndex]
    $next = $sortedFunctions[$sortedIndex + 1]
    if ($current.end -eq $next.start) {
        $nextFunctionByEntry[$current.entry] = $next
    }
}
$sortedInstructions = @($instructions | Sort-Object address)
$functionIndex = 0
$activeFunctions = New-Object System.Collections.Generic.List[object]
foreach ($instruction in $sortedInstructions) {
    while ($functionIndex -lt $sortedFunctions.Count -and
           $sortedFunctions[$functionIndex].start -le $instruction.address) {
        [void]$activeFunctions.Add($sortedFunctions[$functionIndex])
        ++$functionIndex
    }
    for ($activeIndex = $activeFunctions.Count - 1; $activeIndex -ge 0; --$activeIndex) {
        if ($activeFunctions[$activeIndex].end -le $instruction.address) {
            $activeFunctions.RemoveAt($activeIndex)
        }
    }
    foreach ($span in $activeFunctions) {
        if (-not $instructionsByFunction.ContainsKey($span.entry)) {
            $instructionsByFunction[$span.entry] = New-Object System.Collections.Generic.List[object]
        }
        [void]$instructionsByFunction[$span.entry].Add($instruction)
    }
}

$conversionEvidence = @{}
$conversionFiles = @(Get-ChildItem $conversionRoot -Recurse -File |
    Where-Object { $_.Extension -in @('.cpp', '.hpp') })
foreach ($file in $conversionFiles) {
    $relative = $file.FullName.Substring($conversionRoot.Length + 1).Replace('\', '/')
    $text = [IO.File]::ReadAllText($file.FullName)
    foreach ($match in [regex]::Matches($text, '(?i)\bROM\s+0x([0-9A-F]+)')) {
        Add-Evidence $conversionEvidence $match.Groups[1].Value $relative
    }
}

$report = New-Object System.Collections.Generic.List[object]
foreach ($function in $functions) {
    $address = Normalize-RomAddress $function.entry
    $size = [int]$function.size
    $asm = @(
        if ($instructionsByFunction.ContainsKey($address)) {
            $instructionsByFunction[$address].ToArray()
        }
    )
    if ($asm.Count -eq 0) {
        [void]$issues.Add("No disassembly instructions for $address")
    }

    $adjacentEntryTarget = ''
    $adjacentEntryKind = ''
    $fallthroughTarget = ''
    if ($nextFunctionByEntry.ContainsKey($address) -and $asm.Count -gt 0) {
        $nextEntry = $nextFunctionByEntry[$address]
        $nextStart = $nextEntry.start
        $directTransfersToNext = @($asm | Where-Object {
            $_.text -match '\b(?:bra|bsr|jsr|jmp)(?:\.[bwl])?\s+0x([0-9A-Fa-f]+)' -and
            (Convert-RomAddressToInt $matches[1]) -eq $nextStart
        })
        $adjacentEntryTarget = "$($nextEntry.entry) $($nextEntry.name)"
        if ($directTransfersToNext.Count -gt 0) {
            # The exported spans are adjacent, but this entry explicitly
            # calls/branches to the next entry. It is a shared-entry review,
            # not evidence that execution falls through the boundary.
            $adjacentEntryKind = 'direct-transfer-to-adjacent-entry'
        } elseif ($asm[-1].text -notmatch '\b(?:rts|rte|rtr|bra(?:\.[bwl])?|b[a-z]+(?:\.[bwl])?|jmp|jsr|bsr(?:\.[bwl])?|db[a-z]+|trap|stop)\b') {
            $adjacentEntryKind = 'linear-fallthrough'
            $fallthroughTarget = $adjacentEntryTarget
        } else {
            $adjacentEntryKind = 'adjacent-span-with-explicit-exit'
        }
    }

    $sourcePath = Join-Path $exportRoot ($function.file.Replace('/', '\'))
    $source = if (Test-Path -LiteralPath $sourcePath) {
        [IO.File]::ReadAllText($sourcePath)
    } else {
        ''
    }
    $cleanSource = Remove-CppComments $source
    $body = ''
    $openBrace = $cleanSource.IndexOf('{')
    $closeBrace = $cleanSource.LastIndexOf('}')
    if ($openBrace -ge 0 -and $closeBrace -gt $openBrace) {
        $body = $cleanSource.Substring($openBrace + 1, $closeBrace - $openBrace - 1)
    }
    $bodyCompact = [regex]::Replace($body, '\s', '')
    $returnOnly = $bodyCompact -match '^return[^;{}]*;$'

    $directCalls = @(
        if ($callSitesByCaller.ContainsKey($address)) {
            $callSitesByCaller[$address] | Where-Object {
                $_.status -eq 'resolved' -and
                $_.text -match '^(?:jsr|bsr(?:\.[bwl])?)\s+0x'
            }
        }
    )
    $missingDirectCalls = New-Object System.Collections.Generic.List[string]
    $missingDirectCallKeys = @{}
    foreach ($call in $directCalls) {
        $target = Normalize-RomAddress $call.target
        if (-not $functionByAddress.ContainsKey($target)) {
            [void]$issues.Add("Direct call target is not exported: $address -> $target")
            continue
        }
        $targetName = $functionByAddress[$target].name
        $pattern = '(?<![A-Za-z0-9_])' + [regex]::Escape($targetName) + '\s*\('
        $missingKey = "$target $targetName"
        if ($cleanSource -notmatch $pattern -and
            -not $missingDirectCallKeys.ContainsKey($missingKey)) {
            $missingDirectCallKeys[$missingKey] = $true
            [void]$missingDirectCalls.Add("$target $targetName")
        }
    }

    $status = if ($statusByAddress.ContainsKey($address)) {
        $statusByAddress[$address]
    } else {
        $null
    }
    if ($null -eq $status) {
        [void]$issues.Add("Missing status row: $address")
    }

    $evidenceFiles = if ($conversionEvidence.ContainsKey($address)) {
        @($conversionEvidence[$address] | Sort-Object)
    } else {
        @()
    }

    $risk = New-Object System.Collections.Generic.List[string]
    if ($asm.Count -eq 0) { [void]$risk.Add('missing_rom_span') }
    if (-not (Test-Path -LiteralPath $sourcePath)) { [void]$risk.Add('missing_ghidra_c') }
    if ($adjacentEntryKind -eq 'linear-fallthrough') {
        [void]$risk.Add('fallthrough_to_next_export')
    }
    if ($returnOnly -and $size -gt 8) { [void]$risk.Add('ghidra_c_return_only') }
    if ($missingDirectCalls.Count -gt 0) { [void]$risk.Add('native_substitution_review') }
    if ($evidenceFiles.Count -eq 0) { [void]$risk.Add('no_conversion_rom_evidence') }
    if ($null -eq $status -or [string]::IsNullOrWhiteSpace($status.next)) {
        [void]$risk.Add('missing_audit_next_step')
    }
    if ($null -ne $status -and $status.status -ne 'defended') {
        [void]$risk.Add('not_defended')
    }

    $classification = if ($risk -contains 'missing_rom_span' -or
                          $risk -contains 'missing_ghidra_c') {
        'source-coverage-risk'
    } elseif ($risk -contains 'fallthrough_to_next_export') {
        'fallthrough-review'
    } elseif ($risk -contains 'native_substitution_review') {
        'native-substitution-review'
    } elseif ($risk -contains 'ghidra_c_return_only') {
        'decompiler-omission-risk'
    } elseif ($null -ne $status -and $status.status -eq 'defended' -and
              $evidenceFiles.Count -gt 0) {
        'defended-candidate'
    } else {
        'manual-source-trace-required'
    }

    [void]$report.Add([pscustomobject]@{
        rom_entry = $address
        name = $function.name
        rom_size = $size
        disassembly_lines = $asm.Count
        direct_call_count = $directCalls.Count
        missing_direct_calls = ($missingDirectCalls -join '; ')
        ghidra_c_return_only = [int]$returnOnly
        adjacent_entry_target = $adjacentEntryTarget
        adjacent_entry_kind = $adjacentEntryKind
        fallthrough_target = $fallthroughTarget
        conversion_evidence_files = ($evidenceFiles -join ';')
        status = if ($null -ne $status) { $status.status } else { '' }
        native_owner = if ($null -ne $status) { $status.native_owner } else { '' }
        test_name = if ($null -ne $status) { $status.test_name } else { '' }
        audit_next = if ($null -ne $status) { $status.next } else { '' }
        classification = $classification
        risk_flags = ($risk -join ';')
    })
}

$report = @($report | Sort-Object { Convert-RomAddressToInt $_.rom_entry })
$reportPath = Join-Path $docsRoot "rom-conversion-audit.tsv"
if (-not $NoWrite) {
    $csv = $report | ConvertTo-Csv -Delimiter "`t" -NoTypeInformation
    $utf8 = New-Object Text.UTF8Encoding($false)
    [IO.File]::WriteAllText($reportPath, (($csv -join "`n") + "`n"), $utf8)
}

$sourceRisk = @($report | Where-Object {
    $_.classification -eq 'source-coverage-risk'
})
$nativeSubstitutionReview = @($report | Where-Object {
    $_.classification -eq 'native-substitution-review'
})
$decompilerRisk = @($report | Where-Object {
    $_.risk_flags -match 'ghidra_c_return_only'
})
$fallthroughRisk = @($report | Where-Object {
    $_.risk_flags -match 'fallthrough_to_next_export'
})
$adjacentEntryRisk = @($report | Where-Object {
    $_.risk_flags -match 'fallthrough_to_next_export'
})
$noEvidence = @($report | Where-Object {
    $_.risk_flags -match 'no_conversion_rom_evidence'
})
$defended = @($report | Where-Object { $_.status -eq 'defended' })

"ROM/conversion source audit"
"Repository: $RepoRoot"
"Exported function entries: $($functions.Count)"
"Disassembly instruction records: $($instructions.Count)"
"Rows written: $($report.Count)"
"Source-coverage risks: $($sourceRisk.Count)"
"Native substitution reviews: $($nativeSubstitutionReview.Count)"
"Generated-C omission risks: $($decompilerRisk.Count)"
"Linear adjacent-export boundary reviews: $($adjacentEntryRisk.Count)"
"Linear fall-through risks: $($fallthroughRisk.Count)"
"Entries without explicit conversion ROM evidence: $($noEvidence.Count)"
"Defended status rows: $($defended.Count)"
"Structural issues: $($issues.Count)"

if ($decompilerRisk.Count -gt 0) {
    ""
    "Generated-C omission-risk entries:"
    $decompilerRisk | Select-Object rom_entry, name, rom_size, disassembly_lines, direct_call_count, missing_direct_calls, risk_flags | Format-Table -AutoSize
}

if ($adjacentEntryRisk.Count -gt 0) {
    ""
    "Linear adjacent-export boundary reviews (classify shared entries before merging):"
    $adjacentEntryRisk | Select-Object rom_entry, name, adjacent_entry_target, adjacent_entry_kind, fallthrough_target, risk_flags | Format-Table -Wrap -AutoSize
}

if ($nativeSubstitutionReview.Count -gt 0) {
    ""
    "Native-substitution review entries (ROM direct calls are represented by host abstractions or callbacks):"
    $nativeSubstitutionReview | Select-Object rom_entry, name, missing_direct_calls, audit_next | Format-Table -Wrap -AutoSize
}

if ($sourceRisk.Count -gt 0) {
    ""
    "Source-coverage-risk entries:"
    $sourceRisk | Select-Object rom_entry, name, risk_flags | Format-Table -AutoSize
}

if ($issues.Count -gt 0) {
    ""
    "Structural issues:"
    $issues | ForEach-Object { "- $_" }
}

if ($Strict -and $issues.Count -gt 0) {
    throw "ROM/conversion source audit failed with $($issues.Count) structural issue(s)."
}
if ($RequireBehavioralClosure -and ($report | Where-Object {
    $_.status -ne 'defended' -or $_.risk_flags -ne ''
}).Count -gt 0) {
    throw "Behavioral closure is not complete: every entry must be defended with no audit risk flags."
}
