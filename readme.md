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
-lut   <Y/N>       Use LUT (default: Y - yes)
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



---

# TODO

## mvp podstawowe
1. Circle of Confusion (COC) z modelu thin lens (Sec.5.1, Eq.7) - razem z #2
2. COC zależny od: focus distance, aperture, sensor size - razem z #1
3. Rozróżnienie blur vs bokeh na podstawie COC (Sec.4.1) - wymaga #1,#2
4. Kształt przysłony (aperture blades) (Sec.5.4.1) - niezależne

## mvp ważne
5. Visibility sampling dla bokeh (Sec.4.2) - razem z #6
6. Test depth wewnątrz krążka bokeh (Sec.4.2) - razem z #5
7. Energy conservation przy blendowaniu (Sec.4.1) - razem z #3
8. Bokeh LUT (precomputed texture) (Sec.5.2) - niezależne

## na koniec
9. Downsampling / clustering highlightów (Sec.4.2.2) - razem z #3
10. Cat-eye effect (off-axis bokeh) (Sec.5.4.3) - razem z #11
11. Chromatic aberration (Sec.5.3.3) - razem z #10
12. Temporal stability - niezależne
13. Optymalizacja wydajności - na sam koniec
