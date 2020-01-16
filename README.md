# VStancer

A script that allows for changing wheel node details, like camber, track width, height. Might be expanded in the future.

## Downloads

[GTA5-Mods.com](https://www.gta5-mods.com/scripts/vstancer)

## Building

### Requirements
* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)
* [GTAVMenuBase](https://github.com/E66666666/GTAVMenuBase)
* [GTAVManualTransmission](https://github.com/E66666666/GTAVManualTransmission)
* Visual Studio 2019 (and its build tools)

Instructions:

* Clone both this repo and Manual Transmission to one directory
  * `git clone --recurse-submodules https://github.com/E66666666/GTAVStancer.git`
  * `git clone --recurse-submodules https://github.com/E66666666/GTAVManualTransmission.git`
* Build!

## Todo until v1.0.0 release

* ~~Patching wheel set/reset instructions.~~ __Fixed in v0.1.0__
* Automatic optional blacklist for unsupported models (lowrider suspension drop handling)

### Less attainable goals

* Independent (visual) wheel size, tie visual/physical sizes together
* Fix glitching with extreme camber or height changes

## Related

[carmineos' VStancer](https://github.com/carmineos/fivem-vstancer) for [FiveM](https://fivem.net/)
