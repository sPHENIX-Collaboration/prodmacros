- [Getting started](#getting-started)
- [Macro directories](#macro-directories)
  - [Sidebar: Comparing to main](#sidebar-comparing-to-main)
- [Rules](#rules)
- [Autopilot](#autopilot)
  - [Adapt or create rules](#adapt-or-create-rules)
- [Add autopilot to the crontab(s)](#add-autopilot-to-the-crontabs)
- [Appendix: Complete yaml files](#appendix-complete-yaml-files)
  - [Contents of `rules/run3oo_calo_physics_pro001_pcdb001_v001.yaml`](#contents-of-rulesrun3oo_calo_physics_pro001_pcdb001_v001yaml)
  - [Contents of `pilots/autopilot_run3oo_calo_physics_pro001_pcdb001_v001.yaml`](#contents-of-pilotsautopilot_run3oo_calo_physics_pro001_pcdb001_v001yaml)

## Getting started

Clone the ProdFlow repository in the right location and into an appropriately named folder:
```bash
cd Production2026
git clone git@github.com:sPHENIX-Collaboration/prodmacros.git run3oo_calo_pro001_pcdb001_v001
cd run3oo_calo_pro001_pcdb001_v001
```

Start by checking out a new branch, with a slightly different name to not confuse git.
Then make a copy of the appropriate directory, again slightly differently named.
```bash
cp -r run3oo dir_run3oo_calo_pro001_pcdb001_v001
git add dir_run3oo_calo_pro001_pcdb001_v001
```
Optional: No other directories are needed at this point, so we can delete them
```bash
git rm -r run*
```

A production needs:
* One or more directories with root macros and steering scripts
* A `yaml` file with production rules
* An autopilot `yaml` file that instantiates these rules, to be used by cron jobs. 
These three live in respective subdirectories.

After renaming the template contents some more, we have:
```bash
cd dir_run3oo_calo_pro001_pcdb001_v001
ls -1
calo_code
pilots
rules
streaming_code
tracking_code
triggered_code
```

Optional: We can delete unneeded directories, in this case tracking and streaming code:
```bash
git rm -rf tracking_code streaming_code
```

## Macro directories
The `main` branch which we used as a starting point should always be up to date. If you need to make general changes to macros, please do so in the `main` branch first, then come back here and merge them. For production specific "one-off" changes, please adjust code in
```bash
triggered_code
calo_code
```
These directories contain one or more Fun4All macros accompanied by a shell wrapper:
```bash
find triggered_code calo_code
> find triggered_code calo_code
triggered_code
triggered_code/Fun4All_Prdf_Combiner.C
triggered_code/run_eventcombine.sh
calo_code
calo_code/Fun4All_Year2_Fitting.C
calo_code/run_fitting.sh
```

### Sidebar: Comparing to main
To compare a current file, use `git diff branch-name -- filename`. For example,
```diff
git diff main:run3oo/triggered_code/Fun4All_Prdf_Combiner.C ./triggered_code/Fun4All_Prdf_Combiner.C
diff --git a/run3oo/triggered_code/Fun4All_Prdf_Combiner.C b/dir_run3oo_calo_pro001_pcdb001_v001/triggered_code/Fun4All_Prdf_Combiner.C
index ec93f7e..1a0f63e 100644
--- a/run3oo/triggered_code/Fun4All_Prdf_Combiner.C
+++ b/dir_run3oo_calo_pro001_pcdb001_v001/triggered_code/Fun4All_Prdf_Combiner.C
@@ -26,6 +26,7 @@ void Fun4All_Prdf_Combiner(int nEvents = 0,
                            const std::string &outbase = "delme",
                            const std::string &outdir = "/sphenix/data/data02/sphnxpro/scratch/kolja/test")
 {
+  std::cout << "hello world" << std::endl;
   Fun4AllServer *se = Fun4AllServer::instance();
   se->Verbosity(1);
   se->VerbosityDownscale(100000);
```

<!-- (To compare committed changes, you need to use `git diff branch1:path/to/file1 branch2:path/to/file2` instead) -->

## Rules
Start from the appropriate template:
```bash
git mv run3oo_calo_physics_PROD_TAG_VERSION.yaml run3oo_calo_physics_pro001_pcdb001_v001.yaml
```
You can pretty much guess what changes are needed in this file from that `mv` command.

In this example, we need rules for event combining, i.e. `DST_TRIGGERED_EVENT`, and wave form fitting, `DST_CALOFITTING`.
The individual names can be freely chosen; by convention, adorn the `dsttype` value with a prefix and postfix. We will use two rules ("top nodes"), `pro001_TRIGGERED_EVENT_run3oo` and `pro001_CALOFITTING_run3oo`.

Critical fields to adapt are `build`, `dbtag`, and version, which then need to properly trickle down into the next step's `intriplet`:
```yaml
#__________________________________________________________________________________
pro001_TRIGGERED_EVENT_run3oo:
  params:
    dsttype:      DST_TRIGGERED_EVENT
    build:        pro001
    dbtag:        pcdb001
    version:      1
    period:       run3oo
    dataset:      run3oo
    physicsmode:  physics


[...]
#__________________________________________________________________________________
pro001_CALOFITTING_run3oo:
  params:
    dsttype:      DST_CALOFITTING
    build:        pro001
    dbtag:        pcdb001
    version:      1
    period:       run3oo
    dataset:      run3oo
    physicsmode:  physics

  input:
    intriplet:   pro001_pcdb001_v001   # This is the output triplet of the previous rule!

[...]
#__________________________________________________________________________________
```
Note that `build`, `tag`, and `version` of the two steps don't need to be the same, the connection is made via the `intriplet`. However, if they do differ, please name the base directories, branch, git tag, etc. wisely.


<!-- # filesystem: # redirect log while data03 is full
#   outdir:      "/sphenix/lustre01/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}"
#   finaldir:    "/sphenix/lustre01/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}"
#   logdir:     "/sphenix/sim/sim02/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/log"
#   histdir:  "/sphenix/data/data02/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/hist"
#   condor:       "/tmp/sphenixprod/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/log" -->


It is a good idea to look over the other fields as well. The full contents are in the [Appendix](#appendix-complete-yaml-files). Of particular interest are `dataset` and `physicsmode` for things like cosmics, and `request_memory`, which allows you to specify which RAM image sizes to try before condor gives up. Example:
```yaml
    request_memory:         2 GB, 3 GB, 5 GB
```

You can now submit jobs manually using
```bash
create_submission.py --rule pro001_TRIGGERED_EVENT_run3oo --config rules/run3oo_calo_physics_pro001_pcdb001_v001.yaml --runs 82503 --andgo
```
You could also periodically run `dstspider.py` and `histspider.py` with the same arguments. However, especially for spiders, we want to put this job on autopilot.

## Autopilot
Start from the appropriate template.
```
cd pilots
git mv autopilot_run3oo_calo_physics_PROD_TAG_VERSION.yaml autopilot_run3oo_calo_physics_pro001_pcdb001_v001.yaml
```

### Adapt or create rules
The file needs a top node for any submission host you'd want to run this production on. It starts with paths
```yaml
sphnxprod01:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/Production2026/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/rules
    submitdir:  /sphenix/data/data03/sphnxpro/production/run3oo/submission/{rule}
```
Most important here is to change `configbase`. Note that the production submission installation at `prodbase` can also be individualized. `submitdir` is merely a location for helper caches.

Now add an entry for each of the rules we want to run, ex.:
```yaml
  # Event combining
  pro001_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/runlist_run3oo_calo_pro001
    submit: on
[...]

  # Waveform fitting
  pro001_CALOFITTING_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/runlist_run3oo_calo_pro001
    submit: on
[...]
```
The full file in the [Appendix](#appendix-complete-yaml-files) shows additional parameters to control the spider(s), monitoring, priority, etc. Also shown is a repetition that allows to run submission and/or spidering of the same job type from multiple hosts.

## Add autopilot to the crontab(s)
To run, a setup script has to be sourced and a python executable invoked with the location of the steering file. I.~e., the abstract command for cron is
```bash
source /path/to/this_sphenixprod.sh
production_control.py --steer /path/to/autopilot.yaml
```
Console output in cron jobs spams emails to whoever is at the receiving end (Chris), so in practice we need to redirect the output. It gets logged automatically anyway (the `-vv` flag increases verbosity to `DEBUG` level). The full line to be added to the crontabs (on at least those hosts that should run the production) is
```bash
# pro001 run3oo calo 
15,55 * * * * source /sphenix/u/sphnxpro/mainkolja/sphenixprod/this_sphenixprod.sh >& /dev/null && production_control.py --steer 
/sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001/run3oo_calo_pro001/autopilot_run3oo_calo_physics_pro001_2025p009.yaml -vv  >& /dev/null
```
For the 
To generate more complex `cron` time expressions, see [crontab.guru](https://crontab.guru/).

The proper way to edit crontabs is to edit the version controlled (and soft-linked) named file in `sphnxpro`'s home directory, then call the `crontab` command with the file as an argument **on the proper host**.
```bash
sphnxpro@sphnxprod02 ~> emacs crontab.sphnxprod02
sphnxpro@sphnxprod02 ~> crontab crontab.sphnxprod02
```






## Appendix: Complete yaml files

### Contents of `rules/run3oo_calo_physics_pro001_pcdb001_v001.yaml`

```yaml
#______________________________________________________________________________
pro001_TRIGGERED_EVENT_run3oo:
  params:
    dsttype:      DST_TRIGGERED_EVENT
    build:        pro.001
    dbtag:        pcdb001
    version:      1
    period:       run3oo
    dataset:      run3oo
    physicsmode:  physics

  input:
    db:             rawr
    table:          datasets
    min_run_time:   300
    min_run_events: 100000

  job:
    script:                 run_eventcombine.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             10000
    payload:                [../triggered_code/*]
    request_memory:         2 GB, 3 GB, 5 GB
    request_xferslots:     '3'    
    batch_name:            '{rule_name}_{dataset}_{outtriplet}'
    priority:              '90'

#______________________________________________________________________________
pro001_CALOFITTING_run3oo:
  params:
    dsttype:      DST_CALOFITTING
    build:        pro.001
    dbtag:        pcdb001
    version:      1
    period:       run3oo
    dataset:      run3oo
    physicsmode:  physics

  input:
    db:           fcr
    table:        datasets
    intriplet:    pro001_pcdb001_v001  # This is the output triplet of the previous rule

  job:
    script:                 run_fitting.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             100000
    payload:                [../calo_code/*]
    request_memory:         2500MB, 4GB, 6GB
    request_cpus:          '1'
    batch_name:            '{rule_name}_{dataset}_{outtriplet}'
    priority:              '60'

###############################################################################

```

### Contents of `pilots/autopilot_run3oo_calo_physics_pro001_pcdb001_v001.yaml`

```yaml
################################# Prod03 #######################################
### Standard full production 
sphnxprod02:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/Production2026/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/rules
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{rule}

  # Event combining
  pro001_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/runlist_run3oo_calo_pro001
    # runs: [82374 82703]
    #jobprio: 90
    jobprio: 110
    submit: on
    dstspider: on
    finishmon: on

  # Waveform fitting
  pro001_CALOFITTING_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/dir_run3oo_calo_pro001_pcdb001_v001/runlist_run3oo_calo_pro001
    jobprio: 110
    submit: on
    dstspider: on
    finishmon: on

###############################################################################
```




