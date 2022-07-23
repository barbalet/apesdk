## Who is the intended user of this?

Software developers

Agent modellers

Third-party project creators

## Aims

### From urban ###

//#define LARGE_SIM (32 * 1024) 
// Runs in GUI won't in CLI

//#define LARGE_SIM (16 * 1024) 
// Runs in GUI and CLI with BRAIN_ON

Problems:

* being_initial_location
* genetics_unique

When solved:

#define LARGE_SIM (64*1024)

Population: 57344
Adults: 57344   Juveniles: 0
Females: 28800 (50%)   Males: 28544 (49%)

### Memory ###

Remove the size_t memory limitation

Better memory management

### JSON ###

***Object/JSON*** has a smaller memory footprint

***Object/JSON*** parses faster

Further work on ***prototype*** - giving JSON to fast access arrays C

### Platform Specific ###

Full Linux version of UI

Full Win32 version of UI

Full Swift version of UI (future-proof from WWDC requirements)

Address the ***metal*** regression (faster graphics on Mac)

### Use Goals ###

Use by a non-simulation project (independent adoption)

Feedback mods from [urban](https://gitlab.com/barbalet/urban)

Feedback mods from [simulatedape](https://gitlab.com/barbalet/simulatedape)

Feedback mods from [simulatedearth](https://gitlab.com/barbalet/simulatedearth)

Feedback mods from [hundred](https://gitlab.com/barbalet/hundred)

Feedback mods from [levelape](https://gitlab.com/barbalet/levelape)








