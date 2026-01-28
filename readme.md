## Budowanie
cd “glowny folder tam gdzie jest cmakelists.txt

cmake -B build -S .

cmake --build build --config Debug


## Uruchomienie 
.\build\Debug\BokehRenderer.exe

## Pomocne:
BokehRenderer.exe --help

## Opcje CLI:
```
--scene <path>   Sciezka do GLTF/GLB
--focus <0-1>    Focus depth (default: 0.925)
--aperture <f>   F-number (default: 1.4)
--radius <px>    Bokeh radius (default: 4.0)
--scale <f>      Model scale (default: 0.01, 1.0=oryginalny)
--lut   <0/1>       Use LUT (default: 1 - true)
```

**Przykład - oryginalny rozmiar modelu:**
```bash
.\build\Debug\BokehRenderer.exe --scene assets/DamagedHelmet.glb --scale 1.0
```

## Sterowanie

- WASD: kamera
- Mysz: obrot kamery
- Strzalki gora/dol: focus depth
- Strzalki lewo/prawo: bokeh radius  
- Q/E: aperture
