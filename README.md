# PrusaSlicer fan ramp up postprocessor

Postprocessor for PrusaSlicer that turns on the model fan in advance.

Uses [fork](https://github.com/ralex2304/klipper_estimator) of [klipper_estimator](https://github.com/Annex-Engineering/klipper_estimator) by [Annex-Engineering](https://github.com/Annex-Engineering) for time calculations

## Installation
Download directory `prusaslicer-fan-rampup` with 2 files: `klipper_estimator.exe` and `script.py`

## Usage
Type in PrusaSlicer postprocessing scripts section **2 lines**:

```
<path>\prusaslicer-fan-rampup\klipper_estimator.exe dump-moves;
python <path>\prusaslicer-fan-rampup\script.py <ramp-up-time>;
```

E.g.

```
D:\Downloads\klipper_estimator.exe dump-moves;
C:\Users\A\AppData\Local\Programs\Python\Python310\python.exe D:\Documents\PrusaSlicer-postproccesors\PrusaSlicer-fan-rampUp\script.py 1.5;
```

### Warning
Don't use klipper_estimator `--config_file` option. It will not work correctly

## Thanks

[Annex-Engineering](https://github.com/Annex-Engineering) - [klipper_estimator plugin](https://github.com/Annex-Engineering/klipper_estimator)