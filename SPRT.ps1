$command = "cutechess-cli.exe"
$arguments = '-engine conf="Leandor dev 2.0 LMR" -engine conf="Leandor 2.0 game_phase" -each tc=inf/10+0.1 -openings file="C:\Users\olive\Desktop\Schach\Books\Balsa_v110221.pgn" format=pgn order=random -games 2 -rounds 5000 -repeat 2 -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -concurrency 6 -ratinginterval 10 -pgnout "sprt.pgn"'


$process = Start-Process -FilePath $command -ArgumentList $arguments -NoNewWindow -Wait

if ($process.ExitCode -eq 0) {
    Write-Host "Command executed successfully."
} else {
    Write-Host "Command execution failed with exit code: $($process.ExitCode)."
}
