$command = "cutechess-cli.exe"
$arguments = '-engine conf="Leandor dev 3.0 TT-fix" -engine conf="Leandor dev 3.0 PVS" -each tc=inf/10+0.1 -openings file="C:\Users\olive\Desktop\Schach\Books\Balsa_v110221.pgn" format=pgn order=random -games 2 -rounds 5000 -repeat 2 -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -concurrency 8 -ratinginterval 10 -pgnout "sprt.pgn"'


$process = Start-Process -FilePath $command -ArgumentList $arguments -NoNewWindow -Wait

if ($process.ExitCode -eq 0) {
    Write-Host "Command executed successfully."
} else {
    Write-Host "Command execution failed with exit code: $($process.ExitCode)."
}
