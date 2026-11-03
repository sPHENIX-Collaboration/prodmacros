- [Getting started](#getting-started)
- [Macro (Code) directories](#macro-code-directories)
- [Rules](#rules)
- [Autopilot](#autopilot)
  - [Adapt or create rules](#adapt-or-create-rules)
- [Add to the automated productions](#add-to-the-automated-productions)
- [Update the branch and tag its tip](#update-the-branch-and-tag-its-tip)
- [Aborting and Cleaning Up a Production](#aborting-and-cleaning-up-a-production)
- [Appendix: Comparing to main](#appendix-comparing-to-main)
- [Appendix: Job Exit Codes](#appendix-job-exit-codes)
- [Appendix: Complete yaml files](#appendix-complete-yaml-files)
  - [Calo rules yaml](#calo-rules-yaml)
  - [Calo autopilot yaml](#calo-autopilot-yaml)
  - [Tracking rules yaml](#tracking-rules-yaml)
  - [Tracking autopilot yaml](#tracking-autopilot-yaml)

## Getting started

The standard location for productions is `/sphenix/u/sphnxpro/Production2026`. 
The `prodmacros` subdirectory should always be the up to date `main` branch of [sPHENIX-Collaboration/prodmacros](https://github.com/sPHENIX-Collaboration/prodmacros).
The `sphenixprod` subdirectory should always be the up to date `main` branch of [BNLNPPS/sphenixprod](https://github.com/BNLNPPS/sphenixprod).
Please perform any development work elsewhere, then bring it here via pull request, and `git pull`.

First, define your production type and tag parameters in your terminal.
**(Adjust "calo" or "tracking", the dataset, and the tags as needed):**

```bash
export PROD_DATASET="run3oo"
export PROD_TYPE="calo"          # or "tracking"
export PROD_PHYSICSMODE="physics" # or "cosmics", etc.
export PROD_BUILD="pro.001"       # build string used in YAML
export PROD_BUILD_TEXT="${PROD_BUILD//./}"  # build string with periods removed for names
export PROD_DBTAG="pcdb001"
export PROD_VERSION="v001"
export SET_TYPE_MODE="${PROD_DATASET}_${PROD_TYPE}_${PROD_PHYSICSMODE}"
export TRIPLET="${PROD_BUILD_TEXT}_${PROD_DBTAG}_${PROD_VERSION}"
## Double-check:
echo "${SET_TYPE_MODE}  ${TRIPLET}"
run3oo_calo_physics  pro001_pcdb001_v001
```

Now, clone the repository and set up your branches. **You can copy/paste this exactly as is:**

```bash
cd ~/Production2026
git clone git@github.com:sPHENIX-Collaboration/prodmacros.git ${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}
cd ${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}
```

Start by checking out a new branch, with a slightly different name to not confuse git.
Then make a copy of the appropriate directory, again slightly differently named.
```bash
git checkout -b branch_${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}
git branch --show-current
branch_run3oo_calo_pro001_pcdb001_v001
cp -r ${PROD_DATASET} thisProd
git add thisProd
```
Keep a production instance small: after copying the dataset template to `thisProd/`, remove the generic repository documentation, the copied active-production list, backup directories, and every top-level dataset template. The tracked files left at the top level should be `.gitignore`, a production-specific README, and `thisProd/`. Inspect the file list first and adapt the explicit paths below if the template repository has changed. Do not edit or remove the live master `active_productions.txt` in `prodmacros`.

```bash
git ls-files
git rm -r --ignore-unmatch -- AGENTS.md Named_Productions.md README.md active_productions.txt .clang-format .clang-tidy bak cosmics run2auau run2pp run3auau run3oo run3pp testing
# Add a README_${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}.md for instance-specific notes.
ls -1
cd thisProd
```


A production needs:
* One or more directories with root macros and steering scripts
* A `yaml` file with production rules
* An autopilot `yaml` file that instantiates these rules, to be used by cron jobs. 
These three live in respective subdirectories.

```bash
cd thisProd
ls -1
pilots
rules
streaming_code
tracking_code
triggered_code
calo_code
```

## Macro (Code) directories
The `main` branch which we used as a starting point should always be up to date. If you need to make general changes to macros, please do so in the `main` branch first, then come back here and merge them. For production specific "one-off" changes, adjust code in the relevant directories:

- **Calo:** `triggered_code`, `calo_code`
- **Tracking:** `streaming_code`, `tracking_code`

These directories contain one or more Fun4All macros accompanied by a shell wrapper. For calo productions you can see them with:
```bash
> find triggered_code calo_code
triggered_code
triggered_code/Fun4All_Prdf_Combiner.C
triggered_code/run_eventcombine.sh
calo_code
calo_code/Fun4All_Year2_Fitting.C
calo_code/run_fitting.sh
calo_code/Fun4All_JetSkimmedProductionYear2.C
calo_code/run_jetskimmer.sh
```

And for tracking productions:
```bash
> find streaming_code/ tracking_code/
streaming_code/
streaming_code/Fun4All_SingleStream_Combiner.C
streaming_code/run_parallel_streams.sh
tracking_code/
tracking_code/Fun4All_JobA.C
tracking_code/Fun4All_JobC.C
tracking_code/Fun4All_RolloverJob0.C
tracking_code/run_jobA.sh
tracking_code/run_jobC.sh
tracking_code/run_rolloverjob0.sh
```

Remove macro directories not needed for this production type, then remove unused rule and pilot templates so `thisProd/` contains only files this instance uses. For example:
```bash
# For calo:
git rm -r -- streaming_code tracking_code
# For tracking instead:
# git rm -r -- triggered_code calo_code
```


## Rules
Start from the appropriate template:
```bash
cd rules
git mv ${SET_TYPE_MODE}_PROD_TAG_VERSION.yaml ${SET_TYPE_MODE}_${TRIPLET}.yaml
```
As usual, you can delete (`git rm -f`) the unused rule.

It is quite easy to guess what changes are needed file, namely PROD, TAG and VERSION, plus maybe a few naming and control details.
Each rule needs a name; by convention adorn the `dsttype` with a build prefix and dataset postfix, e.g. `pro001_TRIGGERED_EVENT_run3oo`.

For a set of commands to make these changes, see below the example. The critical fields to adapt are named `build`, `dbtag`, and `version`, which then need to properly trickle down into the next step's `intriplet`. Note that **`build` requires a period** (e.g. `ana.548`, `pro.001`) — the exception is `new`, which has no period. Calo example:
```yaml
#__________________________________________________________________________________
pro001_TRIGGERED_EVENT_run3oo:
  params:
    dsttype:      DST_TRIGGERED_EVENT
    build:        pro.001
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
    build:        pro.001
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

Quick way to automatically apply substitutions for PROD, TAG, VERSION, and intriplet:
```bash
sed -i "s/build:.*PROD/build:        ${PROD_BUILD}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/dbtag:.*TAG/dbtag:        ${PROD_DBTAG}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/version:.*VERSION/version:      ${PROD_VERSION#v}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/intriplet:.*PROD_TAG_VERSION/intriplet:   ${TRIPLET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_STREAMING_EVENT_${PROD_DATASET}/${PROD_BUILD_TEXT}_STREAMING_EVENT_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_CLUSTER_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_CLUSTER_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_SEED_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_SEED_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_TRACKS_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_TRACKS_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRIGGERED_EVENT_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRIGGERED_EVENT_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_CALOFITTING_${PROD_DATASET}/${PROD_BUILD_TEXT}_CALOFITTING_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_JETSKIMMER_${PROD_DATASET}/${PROD_BUILD_TEXT}_JETSKIMMER_${PROD_DATASET}/g" ${SET_TYPE_MODE}_${TRIPLET}.yaml
```


It is a good idea to look over the other fields as well. The full contents for both calo and tracking are in the [Appendix](#appendix-complete-yaml-files). Of particular interest are `dataset` and `physicsmode` for things like cosmics, and `request_memory`, which allows you to specify which RAM image sizes to try successively before condor gives up. Example:
```yaml
    request_memory:         2 GB, 3 GB, 5 GB
```

You can now submit jobs manually using (adjust the rule name and config path):
```bash
# Calo example:
create_submission.py --rule pro001_TRIGGERED_EVENT_run3oo --config rules/run3oo_calo_physics_pro001_pcdb001_v001.yaml --runs 82503 --andgo
# Tracking example:
create_submission.py --rule pro001_STREAMING_EVENT_run3oo --config rules/run3oo_tracking_physics_pro001_pcdb001_v001.yaml --runs 82503 --andgo
```
And you could also periodically run `dstspider.py` and `histspider.py` with the same arguments. However, especially for spiders, we want to put this job on autopilot.

## Autopilot
Start from the appropriate template.
```bash
cd pilots
git mv autopilot_${SET_TYPE_MODE}_PROD_TAG_VERSION.yaml autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
```
Automatically apply substitutions for rule names, paths, and config file names:
```bash
sed -i "s|configbase:.*BRANCH|configbase: /sphenix/u/sphnxpro/Production2026/${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}/thisProd/rules|g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_STREAMING_EVENT_${PROD_DATASET}/${PROD_BUILD_TEXT}_STREAMING_EVENT_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_CLUSTER_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_CLUSTER_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_SEED_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_SEED_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRKR_TRACKS_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRKR_TRACKS_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_TRIGGERED_EVENT_${PROD_DATASET}/${PROD_BUILD_TEXT}_TRIGGERED_EVENT_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_CALOFITTING_${PROD_DATASET}/${PROD_BUILD_TEXT}_CALOFITTING_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s/PROD_JETSKIMMER_${PROD_DATASET}/${PROD_BUILD_TEXT}_JETSKIMMER_${PROD_DATASET}/g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s|config:.*PROD_TAG_VERSION.yaml|config: ${SET_TYPE_MODE}_${TRIPLET}.yaml|g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
sed -i "s|runlist:.*BRANCH/thisProd/runlist|runlist: /sphenix/u/sphnxpro/Production2026/${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}/thisProd/runlist|g" autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
git add autopilot_${SET_TYPE_MODE}_${TRIPLET}.yaml
```
As usual, you can delete (`git rm`) the unused pilot yaml.

### Adapt or create rules
The file needs a top node for any submission host you'd want to run this production on. It starts with paths:
```yaml
sphnxprod01:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/Production2026/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/rules
    submitdir:  /sphenix/data/data03/sphnxpro/production/run3oo/submission/{batch_name}
```
Most important here is to change `configbase`. Note that the production submission installation at `prodbase` can also be individualized. `submitdir` is a location for helper caches, so make sure it's not in danger of being full.

Now add an entry for each of the rules we want to run. **Calo** example:
```yaml
  # Event combining
  pro001_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]

  # Waveform fitting
  pro001_CALOFITTING_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]
```

**Tracking** example:
```yaml
  # Event combining
  pro001_STREAMING_EVENT_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]

  # Clustering
  pro001_TRKR_CLUSTER_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]

  # Seeding
  pro001_TRKR_SEED_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]

  # Track Fitting
  pro001_TRKR_TRACKS_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runs: [82300 82400]
    submit: on
[...]
```

For production runs, replace `runs` with a `runlist` pointing to a plain-text file of run numbers (one per line):
```yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/runlist_run3oo_calo_pro001
```

The full files in the [Appendix](#appendix-complete-yaml-files) show additional parameters to control the spider(s), monitoring, priority, etc. Also shown is how to run submission and/or spidering of the same job type from multiple submit hosts.

## Add to the automated productions
The autopilot is run with `production_control.py --steer /path/to/autopilot.yaml`. Instead of adding one such line to the `crontab` of all relevant submission hosts, a master script checks on a text file for all productions that should be run. You can double check that it is active on a given node, and which conterol file it is reading, with
```bash
crontab -l
05,35 * * * * /sphenix/u/sphnxpro/Production2026/sphenixprod/master_production_control.sh /sphenix/u/sphnxpro/Production2026/active_productions.txt 180 >& /dev/null 
```
The `180` is the time interval in seconds between invocations of each line in the list.
(To generate more complex `cron` time expressions, see [crontab.guru](https://crontab.guru/)).

To add this production to the list, edit `/sphenix/u/sphnxpro/Production2026/active_productions.txt` which is symlinked to the `prodmacros` repo in the same directory. Important: We should have deleted the local copy of this file early on, but either way, make sure you edit the correct global one. It should look something like:
```bash
cat /sphenix/u/sphnxpro/Production2026/active_productions.txt
# This file lists the active production steering files for the master cron job.
# One full path per line. Lines starting with # are ignored.
/sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/pilots/autopilot_run3oo_calo_physics_pro001_pcdb001_v001.yaml
...
```


## Update the branch and tag its tip
To preserve what we're doing, now commit all changes and create a tag. 
Double check we're not changing main:
```bash
git branch --show-current
branch_run3oo_calo_pro001_pcdb001_v001
```
Commit everything with a reasonable message — stage the message in a variable first to sanity-check it:
```bash
cmessage="Setup for ${PROD_TYPE} production: ${PROD_DATASET}_${PROD_TYPE}_${TRIPLET}"
echo $cmessage
git add .
git commit -a -m "$cmessage"
```
And create an annotated tag, reusing the name we've given this production. Then push everything to github.
```bash
git tag -a tag_${PROD_DATASET}_${PROD_TYPE}_${TRIPLET} -m "$cmessage"
git push -u origin HEAD --follow-tags
```

If you need to make corrections later, get the latest tag and append `_fixN`:
```bash
git tag | tail -1
tag_run3oo_calo_pro001_pcdb001_v001
git tag -a tag_run3oo_calo_pro001_pcdb001_v001_fix1 -m "Added ZDC fix"
git push -u origin HEAD --follow-tags
```
<!-- 
## Problem Solving While Running
To extract subsystem and run number from held jobs (adjust the `condor_q` command with more constraints to isolate a HoldReason or JobBatchName), use
```bash
for line in `condor_q -long -held |grep UserLog | sed 's/UserLog = //g' `; do
   file=$(basename "$line" .condor)
   run=$(      echo "$file" | awk -F'[_-]' '{print $(NF-1)+0}')
   subsystem=$(echo "$file" | awk -F'[_-]' '{print $4}')
   echo $subsystem $run
done
``` -->

## Aborting and Cleaning Up a Production

This section walks through stopping and completely removing a production. We use the three-step tracking production `ana561_2025p013_v001` as a concrete example. All three steps here share a single triplet — this is specific to this production; in general, different steps may have different triplets and each needs its own cleanup pass.

Physical deletion of output files is necessary to avoid collisions if the production is restarted and to prevent stale or incorrect data from persisting. The file catalog and physical files must always be in agreement — deleting one without the other leaves the system in an inconsistent state. The cleanup order matters: the file catalog is queried first to determine whether a file needs to be reproduced, followed by the production database.

### Step 0 — Stop the autopilot and kill running processes

First, prevent the cron job from relaunching anything. Edit `active_productions.txt` and comment out the relevant line:
```
# /sphenix/u/sphnxpro/Production2026/run3oo_tracking_ana561_2025p013_v001/thisProd/pilots/autopilot_run3oo_tracking_physics_ana561_2025p013_v001.yaml
```

Then, on each relevant submission host, find running production processes:
```bash
ps axo pid,args | grep -E 'dispatch|production_control|create_submission|dstspider|histspider'
```

Kill everything referencing this production: `dispatch_productions.py` (it idles and respawns from cron in minutes, but if our job is queued it may dispatch it first), `production_control.py` for our autopilot yaml, and any `create_submission.py`, `dstspider.py`, or `histspider.py` referencing our three rules:
- `ana561_STREAMING_EVENT_run3oo`
- `ana561_TRKR_CLUSTER_run3oo`
- `ana561_TRKR_SEED_run3oo`

Repeat on all submission hosts listed in the autopilot yaml.

Also delete the submission helper directories (the `submitdir` from the autopilot yaml, one per rule):
```bash
rm -r /sphenix/data/data02/sphnxpro/production/run3oo/submission/ana561_STREAMING_EVENT_run3oo/
rm -r /sphenix/data/data02/sphnxpro/production/run3oo/submission/ana561_TRKR_CLUSTER_run3oo/
rm -r /sphenix/data/data02/sphnxpro/production/run3oo/submission/ana561_TRKR_SEED_run3oo/
```

And any leftover DST lists and lock files:
```bash
rm /sphenix/data/data02/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/ana561_STREAMING_EVENT_run3oo_dstlist*
rm /sphenix/data/data02/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/ana561_TRKR_CLUSTER_run3oo_dstlist*
rm /sphenix/data/data02/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/ana561_TRKR_SEED_run3oo_dstlist*
```

### Step 1 — Remove jobs from condor

Check what is queued:
```bash
condor_q -batch
```

Then remove each batch by its `JobBatchName` (format: `{branch}.{rule}_{outtriplet}`):
```bash
condor_rm -const 'JobBatchName=="main.ana561_STREAMING_EVENT_run3oo_ana561_2025p013_v001"'
condor_rm -const 'JobBatchName=="main.ana561_TRKR_CLUSTER_run3oo_ana561_2025p013_v001"'
condor_rm -const 'JobBatchName=="main.ana561_TRKR_SEED_run3oo_ana561_2025p013_v001"'
```

### Step 2 — Delete output files on Lustre

Lustre requires a two-step deletion: first unlink all files, then remove the directory tree. Standard `rm -rf` is much slower on Lustre.

```bash
lfs find /sphenix/lustre01/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/ -type f -print0 | xargs -0 munlink
rm -r /sphenix/lustre01/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/
```

A few notes:
- `lfs find` does not support tab completion — use `find` or `ls` to verify the path before running.
- To be surgical, target a subdirectory rather than the whole triplet, e.g. `.../ana561_2025p013_v001/DST_TRKR_CLUSTER/`.
- This can take a very long time for large productions.

Corresponding logs and histograms live under the same path structure on GPFS (`data02` or `data03` instead of `lustre01`). Standard `rm` works there:
```bash
rm -r /sphenix/data/data02/sphnxpro/production/run3oo/physics/ana561_2025p013_v001/
```

### Step 3 — File catalog cleanup

Connect to the file catalog database:
```bash
psql -d FileCatalog -h sphnxdbmaster.sdcc.bnl.gov
```

For each DST type, delete from `files` first, then `datasets`. Always constrain by both `tag` and `dataset`; the example below uses `tag='ana561_2025p013_v001'` and `dataset='run3oo'`. Use a leading and trailing `%` in the `dsttype` pattern, without an extra `_` before the final `%`. If unsure about the scope, probe with a `SELECT` before deleting. Repeat for all relevant types:
```sql
delete from files using datasets where lfn=filename and datasets.dataset='run3oo' and datasets.tag='ana561_2025p013_v001' and datasets.dsttype like '%DST_STREAMING_EVENT%';
delete from datasets where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_STREAMING_EVENT%';

delete from files using datasets where lfn=filename and datasets.dataset='run3oo' and datasets.tag='ana561_2025p013_v001' and datasets.dsttype like '%DST_TRKR_CLUSTER%';
delete from datasets where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_TRKR_CLUSTER%';

delete from files using datasets where lfn=filename and datasets.dataset='run3oo' and datasets.tag='ana561_2025p013_v001' and datasets.dsttype like '%DST_TRKR_SEED%';
delete from datasets where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_TRKR_SEED%';
```

Some production steps write additional file catalog types. Include these in the file catalog cleanup for the corresponding primary type:
- `DST_JETCALO`: also delete `DST_Jet` and `HIST_JETQA`
- `DST_CALOFITTING`: also delete `DST_ZDC_RAW`, `DST_SEPD_RAW`, and `HIST_CALOFITTINGQA`

Also clean up the production database:
```bash
psql -d Production -h sphnxproddbmaster.sdcc.bnl.gov -U argouser
```
```sql
delete from production_jobs where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_STREAMING_EVENT%';
delete from production_jobs where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_TRKR_CLUSTER%';
delete from production_jobs where dataset='run3oo' and tag='ana561_2025p013_v001' and dsttype like '%DST_TRKR_SEED%';
```

With physical files and both databases cleared, the system is in a clean state. To restart the production, simply uncomment the line in `active_productions.txt` — the cron job will pick it up within minutes.

<!-- ############################################################################### -->
<!-- ############################################################################### -->
<!-- ############################################################################### -->


## Appendix: Comparing to main
To compare a current file, use `git diff branch-name -- filename`. For example,
```diff
git diff main:`${PROD_DATASET}/`triggered_code/Fun4All_Prdf_Combiner.C ./triggered_code/Fun4All_Prdf_Combiner.C
diff --git a/${PROD_DATASET}/triggered_code/Fun4All_Prdf_Combiner.C b/thisProd/triggered_code/Fun4All_Prdf_Combiner.C
index ec93f7e..1a0f63e 100644
--- a/${PROD_DATASET}/triggered_code/Fun4All_Prdf_Combiner.C
+++ b/thisProd/triggered_code/Fun4All_Prdf_Combiner.C
@@ -26,6 +26,7 @@ void Fun4All_Prdf_Combiner(int nEvents = 0,
                            const std::string &outbase = "delme",
                            const std::string &outdir = "/sphenix/data/data02/sphnxpro/scratch/kolja/test")
 {
+  std::cout << "hello world" << std::endl;
   Fun4AllServer *se = Fun4AllServer::instance();
   se->Verbosity(1);
   se->VerbosityDownscale(100000);
```

To compare committed changes, you need to use the following syntax instead:
```bash
git diff branch1:path/to/file1 branch2:path/to/file2
````


## Appendix: Job Exit Codes

All job scripts report a final exit code via `common_runscript_finish.sh`, which records it in the production database. Codes are designed to identify the failure stage at a glance:

| Code | Stage | Meaning |
|------|-------|---------|
| 0 | — | Success |
| 2 | Setup | Bad arguments or configuration error |
| 3 | Setup | Unsupported OS / environment setup failed |
| 10 | Input | No input files found (DB query returned empty) |
| 11 | Input | Remote file health check failed (missing or wrong size) |
| 20 | Stage-in | Input file copy failed (dd retries exhausted or source missing) |
| 21 | Stage-in | Input file md5 mismatch after copy |
| 30 | Stage-out | Output file not found (macro produced no output) |
| 31 | Stage-out | Output file copy failed (dd retries exhausted) |
| 111 | Input | Streaming: wrong number of GL1 or detector list files |
| other | Macro | Propagated directly from `root.exe` exit code |

## Appendix: Complete yaml files

### Calo rules yaml

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
    batch_name:            '{rule_name}_{outtriplet}'
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
    batch_name:            '{rule_name}_{outtriplet}'
    priority:              '60'

###############################################################################

```

### Calo autopilot yaml

```yaml
################################# Prod03 #######################################
### Standard full production 
sphnxprod02:
  defaultlocations:
    prodbase:   /sphenix/u/sphnxpro/Production2026/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/rules
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{batch_name}

  # Event combining
  pro001_TRIGGERED_EVENT_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/runlist_run3oo_calo_pro001
    # runs: [82374 82703]
    #jobprio: 90
    jobprio: 110
    submit: on
    dstspider: on

  # Waveform fitting
  pro001_CALOFITTING_run3oo:
    config: run3oo_calo_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_calo_pro001_pcdb001_v001/thisProd/runlist_run3oo_calo_pro001
    jobprio: 110
    submit: on
    dstspider: on

###############################################################################
```

### Tracking rules yaml

```yaml
#______________________________________________________________________________________________________________________
pro001_STREAMING_EVENT_run3oo:
  params:
    dsttype:    DST_STREAMING_EVENT
    period:     run3oo
    physicsmode: physics
    dataset:    run3oo
    build:      pro001
    dbtag:      pcdb001
    version:    1

  input:
    db:          rawr
    table:       datasets
    min_run_time:   300
    min_run_events: 100000
    combine_seg0_only: false

  job:
    script:                 run_parallel_streams.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             10000
    payload:                [../streaming_code/*]
    request_memory:         3072 MB, 5072 MB, 7072 MB
    request_xferslots:     '3'
    batch_name:            '{rule_name}_{outtriplet}'
    priority:              '90'
    filesystem:
      logdir:   "/sphenix/data/data02/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/log"
      histdir:  "/sphenix/data/data02/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/hist"
      condor:            "/tmp/data02/sphnxpro/{prodmode}/{period}/{physicsmode}/{outtriplet}/{leafdir}/{rungroup}/log"

#______________________________________________________________________________________________________________________
pro001_TRKR_CLUSTER_run3oo:
  params:
    dsttype:     DST_TRKR_CLUSTER
    period:      run3oo
    physicsmode: physics
    dataset:     run3oo
    build:       pro001
    dbtag:       pcdb001
    version:     1

  input:
    db:          fcr
    table:       datasets
    intriplet:   pro001_pcdb001_v001
    # cut_segment: 10
  job:
    script:                 run_rolloverjob0.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             1000
    payload:                [../tracking_code/*]
    request_memory:         8192 MB, 12092 MB, 16092 MB
    request_cpus:          '1'
    batch_name:            '{rule_name}_{outtriplet}'
    priority:              '60'

#______________________________________________________________________________________________________________________
pro001_TRKR_SEED_run3oo:
  params:
    dsttype:     DST_TRKR_SEED
    period:      run3oo
    physicsmode: physics
    dataset:     run3oo
    build:       pro001
    dbtag:       pcdb001
    version:     1
  input:
    db:          fcr
    table:       datasets
    intriplet:   pro001_pcdb001_v001
    # cut_segment: 10

  job:
    script:                 run_jobA.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             1000
    payload:                [../tracking_code/*]
    request_memory:         4096 MB, 6096 MB, 8096 MB
    request_cpus:          '2'
    batch_name:            '{rule_name}_{outtriplet}'
    priority:              '30'

#______________________________________________________________________________________________________________________
pro001_TRKR_TRACKS_run3oo:
  params:
    dsttype:     DST_TRKR_TRACKS
    period:      run3oo
    physicsmode: physics
    dataset:     run3oo
    build:       pro001
    dbtag:       pcdb001
    version:     1
  input:
    db:          fcr
    table:       datasets
    intriplet:   pro001_pcdb001_v001
    # cut_segment: 10

  job:
    script:                 run_jobC.sh
    log:                   '{condor}/{logbase}.condor'
    neventsper:             1000
    payload:                [../tracking_code/*]
    request_memory:         4096 MB, 6096 MB, 8096 MB
    request_cpus:          '2'
    batch_name:            '{rule_name}_{outtriplet}'
    priority:              '30'

###############################################################################
```

### Tracking autopilot yaml

```yaml
################################# Prod01 #######################################
### Standard full production
sphnxprod01:
  defaultlocations:
    submitdir:  /sphenix/data/data02/sphnxpro/production/run3oo/submission/{batch_name}
    prodbase:   /sphenix/u/sphnxpro/Production2026/sphenixprod
    configbase: /sphenix/u/sphnxpro/Production2026/run3oo_tracking_pro001_pcdb001_v001/thisProd/rules

  # STREAMING physics
  pro001_STREAMING_EVENT_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_tracking_pro001_pcdb001_v001/thisProd/runlist
    #runs: [82372 82703]
    jobprio: 90
    submit: on
    dstspider: on

  # TRKR_CLUSTER physics
  pro001_TRKR_CLUSTER_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_tracking_pro001_pcdb001_v001/thisProd/runlist
    #runs: [82372 82703]
    jobprio: 60
    submit: on
    dstspider: on

  # TRKR_SEED physics
  pro001_TRKR_SEED_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_tracking_pro001_pcdb001_v001/thisProd/runlist
    #runs: [82372 82703]
    jobprio: 30
    submit: on
    dstspider: on

  # TRKR_TRACKS physics
  pro001_TRKR_TRACKS_run3oo:
    config: run3oo_tracking_physics_pro001_pcdb001_v001.yaml
    runlist: /sphenix/u/sphnxpro/Production2026/run3oo_tracking_pro001_pcdb001_v001/thisProd/runlist
    #runs: [82372 82703]
    jobprio: 10
    submit: on
    dstspider: on

###############################################################################
```
