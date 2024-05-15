# PrusaSlicer fan ramp up postprocessor

Postprocessor for PrusaSlicer that turns on the model fan in advance.

Uses [fork](https://github.com/ralex2304/klipper_estimator) of [klipper_estimator](https://github.com/Annex-Engineering/klipper_estimator) by [Annex-Engineering](https://github.com/Annex-Engineering) for time calculations

## Installation
Download archive `prusaslicer-fan-rampup` from releases. It contains 2 executable files: `klipper_estimator.exe` and `script.exe` (or `klipper_estimator` and `script` for Linux)

## Usage
Type in PrusaSlicer postprocessing scripts section **2 lines**:

### Windows

```
<path>\prusaslicer-fan-rampup\klipper_estimator.exe dump-moves;
<path>\prusaslicer-fan-rampup\script.exe <ramp-up-time>;
```

### Linux

```
<path>\prusaslicer-fan-rampup\klipper_estimator dump-moves;
<path>\prusaslicer-fan-rampup\script <ramp-up-time>;
```

### E.g.

```
D:\Documents\PrusaSlicer-postprocesors\PrusaSlicer-fan-rampUp\klipper_estimator.exe dump-moves;
D:\Documents\PrusaSlicer-postprocesors\PrusaSlicer-fan-rampUp\script.exe 1.5;
```

## Warning
Don't use klipper_estimator `--config_file` option. It will not work correctly

## Thanks

[Annex-Engineering](https://github.com/Annex-Engineering) - [klipper_estimator plugin](https://github.com/Annex-Engineering/klipper_estimator)
