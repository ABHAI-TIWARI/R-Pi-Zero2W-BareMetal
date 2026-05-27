1
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("PATH","User")

2
cd "C:\Users\Abhai Tiwari\Desktop\R-Pi-Zero2W-BareMetal\blink"

3
make