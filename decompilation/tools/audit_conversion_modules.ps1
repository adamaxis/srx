param(
    [switch]$Detailed
)

$ErrorActionPreference = "Stop"
$conversionRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\ghidra_conversion")).Path
$mapPath = Join-Path $PSScriptRoot "..\docs\conversion-source-modules.tsv"
$rows = Import-Csv $mapPath -Delimiter "`t"

# Composition entry points are not leaf behavior contracts and therefore do
# not belong to one of the 28 ownership rows. Keep this exclusion explicit so
# adding a new conversion implementation still fails the audit unless it is
# assigned to a behavioral module.
$compositionSources = @(
    "src/conversion_workbench.cpp"
)
$sourceFiles = Get-ChildItem (Join-Path $conversionRoot "src") -Recurse -Filter *.cpp -File |
    ForEach-Object { $_.FullName.Substring($conversionRoot.Length + 1).Replace('\', '/') } |
    Where-Object { $_ -notin $compositionSources }

$owners = @{}
foreach ($row in $rows) {
    foreach ($glob in ($row.source_globs -split ';')) {
        $pattern = Join-Path $conversionRoot ($glob.Replace('/', '\'))
        foreach ($match in (Get-ChildItem $pattern -File -ErrorAction SilentlyContinue)) {
            $relative = $match.FullName.Substring($conversionRoot.Length + 1).Replace('\', '/')
            if ($owners.ContainsKey($relative)) {
                throw "Source file is assigned twice: $relative ($($owners[$relative]) and $($row.module_id))"
            }
            $owners[$relative] = $row.module_id
        }
    }
}

$unassigned = @($sourceFiles | Where-Object { -not $owners.ContainsKey($_) })
if ($unassigned.Count -ne 0) {
    throw "Unassigned conversion source files:`n$($unassigned -join "`n")"
}

$unknown = @($owners.Keys | Where-Object { $_ -notin $sourceFiles })
if ($unknown.Count -ne 0) {
    throw "Module map names files that do not exist:`n$($unknown -join "`n")"
}

"Conversion source files: $($sourceFiles.Count)"
"Architecture modules: $($rows.Count)"
foreach ($group in ($owners.GetEnumerator() | Group-Object Value | Sort-Object Name)) {
    "{0}`t{1}" -f $group.Name, $group.Count
    if ($Detailed) {
        $group.Group | Sort-Object Name | ForEach-Object { "  $_" }
    }
}

$functions = Import-Csv (Join-Path $conversionRoot "..\export\functions.tsv") -Delimiter "`t"
$generic = @($functions | Where-Object {
    $_.name -match '^(convert_|routine_|function_|unknown|sub_|fn_)'
})
$callSites = Import-Csv (Join-Path $conversionRoot "..\export\call-sites.tsv") -Delimiter "`t"
$indirect = @($callSites | Where-Object { $_.status -ne "resolved" })
"Generic export labels still needing manual source naming: $($generic.Count)"
"Indirect/unresolved call sites: $($indirect.Count)"
