$command = "cutechess-cli.exe"
$arguments = '-engine conf="Leandor dev 2.6.2" -engine conf="Leandor dev 2.6.2 no-tt-probe" -each tc=inf/10+0.1 -openings file="C:\Users\olive\OneDrive\Desktop\Chess\Arena\Databases\o-deville.pgn" format=pgn order=random -games 2 -rounds 5000 -repeat 2 -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -concurrency 4 -ratinginterval 10 -pgnout "sprt.pgn"'


$process = Start-Process -FilePath $command -ArgumentList $arguments -NoNewWindow -Wait

if ($process.ExitCode -eq 0) {
    Write-Host "Command executed successfully."
} else {
    Write-Host "Command execution failed with exit code: $($process.ExitCode)."
}
