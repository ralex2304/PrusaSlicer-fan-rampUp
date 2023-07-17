# Prusaslicer fan ramp up postproccessor

Postproccessor for PS that turns on the model fan in advance.
Primitive time calculation without taking into account accelerations.

## Usage
Type in PrusaSlicer postprocessing scripts section:
`python <path>\prusaslicer-fan-rampup\script.py <time>;`

E. g.

`C:\Users\A\AppData\Local\Programs\Python\Python310\python.exe D:\Documents\prusaslicer-fan-rampup\script.py 0.8;`

## Plans

I would like to improve the time calculation using https://github.com/Annex-Engineering/klipper_estimator