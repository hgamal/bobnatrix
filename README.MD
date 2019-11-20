# BobNatrix
## Read
pk2cmd -Ppic16f872 -GFx.hex
## Program
pk2cmd -Ppic16f872 -M -F counter.X/dist/default/production/counter.X.production.hex
## Turn on power
pk2cmd -Ppic16f872 -T
## Turn off power
pk2cmd -Ppic16f872 -W
## Program
pk2cmd -Ppic16f872 -M -F counter.X/dist/default/production/counter.X.production.hex
## Program, turn on power and reset
pk2cmd -Ppic16f872 -M -F counter.X/dist/default/production/counter.X.production.hex -R -T
