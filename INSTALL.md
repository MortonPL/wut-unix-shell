## Instalowanie pakietów ogólnodostępnych
```
sudo apt-get install build-essential
sudo apt-get install cmake
```

## Budowanie projektu
```
# Instrukcja użycia skryptu: ./build.sh -h
# Flaga -c wymusza (re)generację cache CMake
# Flaga -t uruchamia testy (jeśli są zbudowane)
./build.sh release -c -t
```
