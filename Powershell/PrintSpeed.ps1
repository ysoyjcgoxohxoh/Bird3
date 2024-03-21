Add-Type -Path "$env:USERPROFILE\source\repos\Candle.NET\bin\x64\Release\netcoreapp3.1\Candle.NET.dll"

$devices = [Candle.Device]::ListDevices()

$device = $devices[0]
$device.Open()

$channel = $device.Channels[0]

$channel.Start(500000)
try {
    while ($true) {
        $frames = $channel.Receive()
        $frames = $frames | Where-Object {$_.Identifier -eq 338}
        foreach ($frame in $frames) {
            #[string]::Join(' ', $frame.Data)
            [System.BitConverter]::ToInt16($frame.Data, 2)# / 36.75
        }
    }
}
finally {
    $channel.Stop()

    $device.Close()
}
