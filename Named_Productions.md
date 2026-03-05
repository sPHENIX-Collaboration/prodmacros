- [Getting started](#getting-started)
- [Make relevant changes](#make-relevant-changes)
  - [Adapt or create rules in `run3oo_calo_physics_prod1_2025p009.yaml`](#adapt-or-create-rules-in-run3oo_calo_physics_prod1_2025p009yaml)
  - [Adapt or create rules in `autopilot_run3oo_calo_physics_prod1_2025p009`](#adapt-or-create-rules-in-autopilot_run3oo_calo_physics_prod1_2025p009)
- [Add autopilot to the crontab(s)](#add-autopilot-to-the-crontabs)
- [Appendix: Complete yaml files](#appendix-complete-yaml-files)
  - [`run3oo_calo_physics_prod1_2025p009.yaml`](#run3oo_calo_physics_prod1_2025p009yaml)
  - [`autopilot_run3oo_calo_physics_prod1_2025p009`](#autopilot_run3oo_calo_physics_prod1_2025p009)

## Getting started

Clone the ProdFlow repository in the right location and into an appropriately named folder:
```bash
cd Production2026
git clone git@github.com:sPHENIX-Collaboration/ProdFlow.git 
git clone git@github.com:sPHENIX-Collaboration/ProdFlow.git Run3OO_Calo_prod1
cd Run3OO_Calo_prod1
```

There's already a massively simplified and cleaned up template, let's start from that by checking it out and then making a new branch. Tag it while we're at it, too. In the future, just start from a recent branch. General, make sure any relevant macro updates from `master` make it into this branch. 
```bash
git checkout run3pp_calo_ana524_TEMPLATE
git checkout -b run3oo_calo_prod1
git tag wip_run3oo_calo_prod1
git push --follow-tags
```
Note the `wip`. The tag refers to a specific commit, so a final tag will come later when we've made all changes.

We'll keep everything in a subdirectory. The repetitiveness is a bit annoying, but bear in mind that updtream this repo is just called `ProdFlow`.
```
mv run3pp_calo_ana524 run3oo_calo_prod1
git add run3oo_calo_prod1
git commit -m "Renamed base path"
## I think `git mv ...` would have been better
cd run3oo_calo_prod1
```

A production needs
* One or more directories with root macros and steering scripts
* A `yaml` file with production rules
* An autopilot `yaml` file that instantiates these rules, to be used by cron jobs. 
After renaming the template contents some more, we have:
```bash
> ls -1
autopilot_run3oo_calo_physics_prod1_2025p009
calo_code
run3oo_calo_physics_prod1_2025p009.yaml
```
Let's check against `master`. Here's an example where I made (and commited) a change to demonstrate what to look out for:
```diff
 > git diff  master:short/run3oo/calo_code/Fun4All_Prdf_Combiner.C run3oo_calo_prod1:run3oo_calo_prod1/calo_code/Fun4All_Prdf_Combiner.C
diff --git a/short/run3oo/calo_code/Fun4All_Prdf_Combiner.C b/run3oo_calo_prod1/calo_code/Fun4All_Prdf_Combiner.C
index 0367b23..70c703f 100644
--- a/short/run3oo/calo_code/Fun4All_Prdf_Combiner.C
+++ b/run3oo_calo_prod1/calo_code/Fun4All_Prdf_Combiner.C
@@ -1,3 +1,4 @@
+hello world
 #include <fun4all/Fun4AllServer.h>
 #include <fun4all/Fun4AllInputManager.h>
 #include <fun4all/Fun4AllDstOutputManager.h>
 ```
Get the `master` version of that file (this is one way to do it):
```bash
git show master:short/run3oo/calo_code/Fun4All_Prdf_Combiner.C > calo_code/Fun4All_Prdf_Combiner.C
git commit -m "testing" calo_code/Fun4All_Prdf_Combiner.C
```
Now running the diff over the full directory does not detect a change, good.
```
git diff master:short/run3oo/calo_code/ run3oo_calo_prod1:run3oo_calo_prod1/calo_code/
<nothing>
```

<!-- 
# Better: 
```
git diff main:run3oo/calo_code/ refs/heads/run3oo_calo_prod1:run3oo_calo_prod1/calo_code/
``` -->


## Make relevant changes

### Adapt or create rules in `run3oo_calo_physics_prod1_2025p009.yaml`
In this example, we need rules for event combining, i.e. `DST_TRIGGERED_EVENT`, and wave form fitting, `DST_CALOFITTING`.
Note that these two terms are values defined and expected by the submission framework, they are to be used in the `params->dsttype` parameter. The individual node names can be freely chosen; by convention, adorn the `dsttype` value with a prefix and postfix.
We will use two head nodes, `prod1_TRIGGERED_EVENT_run3oo` and `prod1_CALOFITTING_run3oo`.

Critical fields to adapt are `period` and `dataset`, as well as `build` and `dbtag`, which then need to properly trickle down into the next step's `intriplet`:
```yaml
#__________________________________________________________________________________
pro1_TRIGGERED_EVENT_run3oo:
  params:
    dsttype:    DST_TRIGGERED_EVENT
    period:     run3oo
    physicsmode: physics
    dataset:    run3oo
    build:      pro.1 
    dbtag:      nocdbtag

[...]
#__________________________________________________________________________________
pro1_CALOFITTING_run3oo:
  params:
    dsttype:     DST_CALOFITTING
    period:      run3oo
    physicsmode: physics
    dataset:     run3oo
    build:       pro.1
    dbtag:       2025p009

  input:
    intriplet:   pro1_nocdbtag_v001

[...]
#__________________________________________________________________________________
```

It is a good idea to look over the other fields as well. The full contents are in the [Appendix](#appendix-complete-yaml-files)

### Adapt or create rules in `autopilot_run3oo_calo_physics_prod1_2025p009`
The file needs a top node for any submission host you'd want to run this production on. It starts with paths
```yaml
sphnxprod01:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/mainkolja/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{rule}
```
Most important here is to change `configbase`. Note that the production submission installation at `prodbase` can also be individualized. `submitdir` is merely a location for helper caches.

Now add an entry for each of the rules we want to run, ex.:
```yaml
  # Event combining
  pro1_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_prod1_2025p009.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/runlist_run3oo_calo_prod1
    submit: on
[...]
  # Waveform fitting
  pro1_CALOFITTING_run3oo:
    config: run2ppcalo_physics_new_2024p024_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/runlist_run3oo_calo_prod1
    submit: on
[...]
```
The full file in the [Appendix](#appendix-complete-yaml-files) shows additional parameters to control the spider(s), monitoring, priority, etc. Also shown is a repetition that allows to run submission and/or spidering of the same job type from multiple hosts.


## Add autopilot to the crontab(s)
To run, a setup script has to be sourced and a python executable invoked with the location of the steering file. I.e., the command for cron is
```bash
source /path/to/this_sphenixprod.sh
production_control.py --steer /path/to/autopilot_run3oo_calo_physics_prod1_2025p009.yaml
```
Console output in cron jobs spams emails to whoever is at the receiving end (Chris), so in practice we need to redirect the output. It gets logged automatically anyway (the `-vv` flag increases verbosity to `DEBUG` level). The full line to be added to the crontabs (of at least those hosts that should run the production) is
```bash
# prod1 run3oo calo 
15,55 * * * * source /sphenix/u/sphnxpro/mainkolja/sphenixprod/this_sphenixprod.sh >& /dev/null && production_control.py --steer 
/sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/autopilot_run3oo_calo_physics_prod1_2025p009.yaml -vv  >& /dev/null
```
For the 
To generate more complex `cron` time expressions, see [crontab.guru](https://crontab.guru/).




























## Appendix: Complete yaml files

### `run3oo_calo_physics_prod1_2025p009.yaml`

```yaml
#__________________________________________________________________________________
pro1_TRIGGERED_EVENT_run3oo:
  params:
    dsttype:    DST_TRIGGERED_EVENT
    period:     run3oo
    physicsmode: physics
    dataset:    run3oo
    build:      pro.1 
    dbtag:      nocdbtag
    version:    1

  input:
    db:          rawr
    table:       datasets
    min_run_time:   300
    min_run_events: 100000

  job:
    script:                 run_eventcombine.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             10000
    payload:                [calo_code/*]
    request_memory:         2024MB
    request_xferslots:     '3'    
    batch_name:            '{dsttype}_{dataset}_{outtriplet}'
    priority:              '90'

#______________________________________________________________________________
  pro1_CALOFITTING_run3oo:
  params:
    dsttype:     DST_CALOFITTING
    period:      run3oo
    physicsmode: physics
    dataset:     run3oo
    build:       pro.1
    dbtag:       2025p009
    version:     1

  input:
    db:          fcr
    table:       datasets
    intriplet:   pro1_nocdbtag_v001

  job:
    script:                 run_fitting.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             100000
    payload:                [calo_code/*]
    request_memory:         2500MB
    request_cpus:          '1'
    batch_name:            '{dsttype}_{dataset}_{outtriplet}'
    priority:              '60'

###############################################################################################
```


### `autopilot_run3oo_calo_physics_prod1_2025p009`

```yaml

################################# Prod01 #######################################
### Standard full production 
sphnxprod01:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/mainkolja/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{rule}

  # Event combining
  pro1_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_prod1_2025p009.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/runlist_run3oo_calo_prod1
    # runs: [82374 82703]
    jobprio: 90
    submit: on
    dstspider: on
    finishmon: on

  # Waveform fitting
  pro1_CALOFITTING_run3oo:
    config: run3oo_calo_physics_prod1_2025p009.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/runlist_run3oo_calo_prod1
    jobprio: 60
    submit: on
    dstspider: on
    finishmon: on

################################# Prod02 #######################################
### Additional host for FITTING
sphnxprod02:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/mainkolja/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{rule}


  # Waveform fitting
  pro1_CALOFITTING_run3oo:
    config: run3oo_calo_physics_prod1_2025p009.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_prod1/run3oo_calo_prod1/runlist_run3oo_calo_prod1
    jobprio: 60
    submit: on
    dstspider: on
    finishmon: off
    
```




