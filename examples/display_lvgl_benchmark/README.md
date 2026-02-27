# display_lvgl_benchmark

LVGL benchmark example for the BigTreeTech Panda Touch BSP.

Runs the built-in LVGL benchmark suite (`lv_demo_benchmark`) and prints a
performance summary (FPS, CPU usage, render time, flush time) to the serial
console for each built-in scene.

## Build

```bash
cd examples/display_lvgl_benchmark
idf.py set-target esp32s3
idf.py build flash monitor
```

## Expected output

After startup the display will cycle through all LVGL benchmark scenes. Once
all scenes have run, the serial console prints a summary table:

```text
Benchmark Summary (9.5.0 )
Name, Avg. CPU, Avg. FPS, Avg. time, render time, flush time
Empty screen, ...
...
All scenes avg., ...
```
