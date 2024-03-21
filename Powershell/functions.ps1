$candumpRegex = [regex]::new("^\((?<TimeStamp>\d+\.\d+)\) (?<Interface>\w+) (?<CANID>[A-Fa-f0-9]+)#(?<Value>[A-Fa-f0-9]+)$", [System.Text.RegularExpressions.RegexOptions]::Compiled)
$referenceTime = ([datetime]'1970-01-01T00:00:00Z').ToUniversalTime()
function Parse-CanDump($line) {
    $m = $candumpRegex.Match($line)
    if ($m.Success) {
        $obj = @{}
        $badParse = 0
        foreach ($g in ($m.Groups | Where-Object { -not [int]::TryParse($_.Name, [ref]$badparse) })) {
            $obj[$g.Name] = $g.Value
        }

        if ($obj['TimeStamp']) {
            $obj['TimeStamp'] = $referenceTime.AddSeconds([double]$obj['TimeStamp'])
            
        }

        if ($obj['Value']) {
            $obj['Value'] = $obj['Value'] -split '(..)' -ne '' | ForEach-Object { [convert]::ToByte($_, 16) }
            
        }
        [pscustomobject]$obj
    }
}

function Parse-DBC($filename) {
    $content = Get-Content $filename
    $r = [regex]::new('BO_\s+(?<ID>\d+)\s+(?<Name>\w+)\s*:\s+(?<Length>[1-8])\s+(?<GroupName>\w+)')
    $results = @()
    foreach ($line in $content) {
        $m = $r.Match($line)
        if ($m.Success) {

            $obj = @{}
            $badParse = 0
            foreach ($g in ($m.Groups | Where-Object { -not [int]::TryParse($_.Name, [ref]$badparse) })) {
                $obj[$g.Name] = $g.Value
            }

            $obj['ID'] = [int]$obj['ID']
            $obj['Length'] = [int]$obj['Length']
            $results += [pscustomobject]$obj
        }
    }

    $results
}

function DisplayAsBinary($entries, [int[]]$positions, [object[]]$filterIDs) {
    if ($filterIDs.Count) {
        $filterIDs = $filterIDs | ForEach-Object {
            if ($_ -is [int]) { [Convert]::ToString($_, 16).ToUpper() }
            elseif ($_ -is [string] -and $_ -match '^(0x)?(?<Number>)[0-9A-Fa-f]+$') {
                $matches.Number.ToUpper()
            } 
        } | Where-Object { $null -ne $_ }
        $entries = $entries | Where-Object { $filterIDs.Contains($_.CANID.ToUpper()) }
    } 
    
    $entries | ForEach-Object {
        $entry = $_.PsObject.Copy()
        $entry.TimeStamp = $entry.TimeStamp.ToString('hh:mm:ss.fff')
        $entry.Value = [string]::Join("|", ($entry.Value[$positions] | ForEach-Object { [System.Convert]::ToString($_, 2).PadLeft(8, '0') }))
        $entry | Add-Member -MemberType NoteProperty -Name CANIDDecimal -Value ([System.Convert]::ToInt32($entry.CANID, 16))
        $entry.CANID = "0x" + $entry.CANID
        $entry
    }
}
function DisplayAsByteList($entries, [int[]]$positions, [object[]]$filterIDs) {
    if ($filterIDs.Count) {
        $filterIDs = $filterIDs | ForEach-Object {
            if ($_ -is [int]) { [Convert]::ToString($_, 16).ToUpper() }
            elseif ($_ -is [string] -and $_ -match '^(0x)?(?<Number>)[0-9A-Fa-f]+$') {
                $matches.Number.ToUpper()
            } 
        } | Where-Object { $null -ne $_ }
        $entries = $entries | Where-Object { $filterIDs.Contains($_.CANID.ToUpper()) }
    }
    $entries | ForEach-Object {
        $entry = $_.PsObject.Copy()
        $entry.TimeStamp = $entry.TimeStamp.ToString('hh:mm:ss.fff')
        $entry.Value = [string]::Join(", ", ($entry.Value[$positions] | ForEach-Object { "0x" + [System.Convert]::ToString($_, 16).PadLeft(2, '0') }))
        $entry | Add-Member -MemberType NoteProperty -Name CANIDDecimal -Value ([System.Convert]::ToInt32($entry.CANID, 16))
        $entry.CANID = "0x" + $entry.CANID
        $entry
    }
}
<#
$content = Get-Content .\Dumps\PoweredBatteryOn2.candump

$dbc = Parse-DBC '.\Bird 3.dbc'

$controllerIDs = $dbc | Where-Object { $_.GroupName -eq 'ControlUnit' } | foreach-object { $_.ID }
$controllerIDs

$parsed = $content | foreach-object { Parse-CanDump $_ }

# The difficult work is done. Let's convert it to a quickly readable format.
$parsed | ConvertTo-Json | Out-File "ControllerStartup3.json"

$parsed = Get-Content ControllerStartup3.json | ConvertFrom-Json

$controllerMessages = $parsed | Where-Object { $controllerIDs.Contains([convert]::ToInt32($_.CANID, 16)) }

$controllerMessages.Count#>