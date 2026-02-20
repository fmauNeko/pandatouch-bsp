# display_noglib

Raw panel (no LVGL) example for the BigTreeTech Panda Touch BSP.

Demonstrates direct framebuffer access without LVGL. Uses the `pandatouch_noglib` component variant.

## Prerequisites

The `pandatouch_noglib` component is not stored in the repository. Generate it first:

```bash
pip install idf-component-manager==2.* py-markdown-table
python .github/ci/bsp_noglib.py pandatouch
```

## Build

```bash
cd examples/display_noglib
idf.py set-target esp32s3
idf.py build flash monitor
```
